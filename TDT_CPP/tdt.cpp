#include <sys/types.h>
#include <regex.h>
#include <assert.h>
#include <gmpxx.h>

#include <sstream>
#include <map>
#include <vector>

#include "tdt.h"
#include "table.h"

using namespace std;

TDTEngine::TDTEngine()
{
	pthread_rwlock_init(&_table_lock, NULL);
	load_trans_table();
}

TDTEngine::~TDTEngine()
{
	unload_trans_table();
	pthread_rwlock_destroy(&_table_lock);
}

string TDTEngine::translate(const string &input, const string &parameters,
      enum tdt__LevelTypeList output_format)
{
	string ret;
	map<string, string> params = parse_parameters(parameters);
	map<string, string>::iterator len_it = params.find("taglength");
	string taglength;
	if (len_it != params.end())
		taglength = len_it->second;

	for(vector<tdt__EpcTagDataTranslation*>::iterator it1 = _table.begin();
			it1 != _table.end(); it1++)
	{
		for (vector<tdt__Scheme *>::iterator it2 = (*it1)->scheme.begin();
				it2 != (*it1)->scheme.end(); it2++)
		{
			// tagLength missmatch
			if (taglength.empty() || taglength != (*it2)->tagLength)
				continue;
			
			for (vector<tdt__Level *>::iterator it3 = (*it2)->level.begin();
					it3 != (*it2)->level.end(); it3++)
			{
				// prefixMatch missmatch
				if ((*it3)->prefixMatch)
				{
					if (input.substr(0, (*it3)->prefixMatch->size()) != *(*it3)->prefixMatch)
					{
						continue;
					}
				}
	
				for (vector<tdt__Option *>::iterator it4 = (*it3)->option.begin();
						it4 != (*it3)->option.begin(); it4++)
				{
					if ((*it4)->pattern)
					{
						string pat = "^" + input + "$";
						regex_t reg;
						assert(regcomp(&reg, pat.c_str(), REG_EXTENDED) != 0);
						const size_t nmatch = 20;
						regmatch_t match[nmatch];
						int match_ret = regexec(&reg, input.c_str(), nmatch, match, 0);
						regfree(&reg);
						if (match_ret != REG_NOMATCH)
						{
							vector<string> segments;
							int i = 1;
							while (match[i].rm_so != -1)
							{
								segments.push_back(input.substr(match[i].rm_so, 
										match[i].rm_eo - match[i].rm_so));
								i++;
							}
							ret = translate2(input, params, output_format, *it1, *it2, *it3, *it4, 
												segments);
							if (!ret.empty())
								return ret;
						}
					}
					else
					{
						// nothing	
					}
				}
			}
		}
	}	

	return ret;
}

void TDTEngine::refresh_translations()
{
	unload_trans_table();
	load_trans_table();
}


void TDTEngine::load_trans_table()
{
	pthread_rwlock_wrlock(&_table_lock);	
	int i = 0;
	while (xmlfiles[i].name != 0)
	{
		stringstream ss(stringstream::in);
		ss.read(xmlfiles[i].start, xmlfiles[i].end - xmlfiles[i].start);
		struct soap soap;
		soap_init(&soap);
		soap_begin(&soap);
		soap.is = &ss;
		tdt__EpcTagDataTranslation *trans = new tdt__EpcTagDataTranslation;
		if (soap_read_tdt__EpcTagDataTranslation(&soap, trans))
		{
			_table.push_back(trans);
		}
		i++;
	}
	pthread_rwlock_unlock(&_table_lock);
}

void TDTEngine::unload_trans_table()
{
	pthread_rwlock_wrlock(&_table_lock);
	for (vector<tdt__EpcTagDataTranslation *>::iterator itor = _table.begin();
			itor != _table.end(); itor++)
		delete *itor;
	_table.clear();
	pthread_rwlock_unlock(&_table_lock);
}

map<string, string> TDTEngine::parse_parameters(const string &parameters)
{
	map<string, string> ret;
	size_t start = 0;
	size_t end =	parameters.find(';', start);
	do
	{
		string str = parameters.substr(0, end);	
		size_t pos;
		if (end == string::npos)
			pos = str.find('=', start);
		else
			pos = str.find("=", start, end - start);
		if (pos == string::npos)
		{
			// missing '='
			ret.clear();
			break;
		}
		else
		{
			ret[str.substr(start, pos - start)] = str.substr(pos + 1);
		}
	} while(true);
	return ret;	
}

string TDTEngine::translate1(const string &input, 
      map<string, string> &params, 
      enum tdt__LevelTypeList output_format, 
      tdt__EpcTagDataTranslation *trans,
      tdt__Scheme *scheme,
      tdt__Level *level)
{
	return "";
}

string TDTEngine::translate2(const string &input, 
      map<string, string> &params, 
      enum tdt__LevelTypeList output_format, 
      tdt__EpcTagDataTranslation *trans, 
      tdt__Scheme *scheme, 
      tdt__Level *level, 
      tdt__Option *option,
      vector<string> &segments)
{
	assert(segments.size() == option->field.size());

	for (int i=0; i != option->field.size(); ++i)
	{
		tdt__Field *field = option->field[i];
		// bitLength
		if (field->bitLength)
		{
			int bitLength = atoi(field->bitLength->c_str());
			if (bitLength != segments[i].size())
			{
				TDTTranslationException ex(TDTBitLengthOverflow);
				throw ex; 
			}	
		}
		//characterSet
		string cs = "^" + field->characterSet + "$";
		regex_t reg;
		assert(regcomp(&reg, cs.c_str(), REG_EXTENDED | REG_NOSUB) == 0);
		int match_ret = regexec(&reg, segments[i].c_str(), 0, NULL, 0);
		if (match_ret == REG_NOMATCH)
		{
			TDTTranslationException ex(TDTFieldOutsideCharacterSet);
			throw ex;
		}
		//decimalMinimum
		if (field->decimalMinimum)
		{
			mpz_class val;
			if (level->type == tdt__LevelTypeList__BINARY)
			{
				val.set_str(segments[i].c_str(), 2);
			}
			else 
			{
				val.set_str(segments[i].c_str(), 10);
			}
			mpz_class decimalMinimum(field->decimalMinimum->c_str(), 10);
			if (val < decimalMinimum)
			{
				TDTTranslationException ex(TDTFieldBelowMinimum);
      	throw ex;
			}
		}
		//decimalMaximum
		if (field->decimalMaximum)
		{
      mpz_class val;
      if (level->type == tdt__LevelTypeList__BINARY)
      {
        val.set_str(segments[i].c_str(), 2);
      }
      else
      {
        val.set_str(segments[i].c_str(), 10);
      }
      mpz_class decimalMaximum(field->decimalMaximum->c_str(), 10);

      if (val > decimalMaximum)
      {
        TDTTranslationException ex(TDTFieldAboveMaximum);
        throw ex;
      }
		}
		//length
		if (field->length)
		{
			int length = atoi(field->length->c_str());
			if (length != segments[i].size())
			{
        TDTTranslationException ex(TDTDifferentLength);
        throw ex;
			}
		}
		
		params[field->name] = segments[i];
	}	

	for (vector<tdt__Rule *>::iterator it = level->rule.begin();
			it != level->rule.end(); it++)
	{
		tdt__Rule *rule = *it;
		if (rule->type == tdt__ModeList__EXTRACT)
		{
			apply_extract_rule(params, output_format, trans, scheme, level, option, rule);
		}	
	}
	return "";
}

void TDTEngine::apply_extract_rule(std::map<std::string, std::string> &params, 
      enum tdt__LevelTypeList output_format, 
      tdt__EpcTagDataTranslation *trans, 
      tdt__Scheme *scheme,
      tdt__Level *level,
      tdt__Option *option,
      tdt__Rule *rule)
{
	string ret;
	vector<string> ps;
	string func = rule->function;
	size_t start = func.find("(");
	assert(start != string::npos);
	start++;
	size_t end = func.find(",", start);
	do
	{
		if (end == string::npos)
		{
			end = func.find(")", start);
			assert(end != string::npos);
			ps.push_back(func.substr(start, end - start));
			break;	
		}
		ps.push_back(func.substr(start, end - start));
		start = end + 1;	
		end = func.find(",", start);
	} while(true);

	if (rule->function.find("SUBSTR(") == 0)
	{
		ret = apply_substr(params, output_format, trans, scheme, level, ps);
	}
	else if (rule->function.find("CONCAT(") == 0)
	{
		//apply_concat();
	}
	else if (rule->function.find("LENGTH(") == 0)
	{
		//apply_length();
	}
	else if (rule->function.find("GS1CHECKSUM(") == 0)
	{
		//apply_gs1checksum();
	}
	else if (rule->function.find("TABLELOOKUP(") == 0)
	{
		//apply_tablelookup();
	}
}

string TDTEngine::apply_substr(std::map<std::string, std::string> &params, 
      enum tdt__LevelTypeList output_format, 
      tdt__EpcTagDataTranslation *trans, 
      tdt__Scheme *scheme, 
      tdt__Level *level, 
      const std::vector<std::string> &ps)
{
	string literal = ps[0];
	if (literal[0] == '"' && literal[literal.size()-1] == '"')
	{
		literal = literal.substr(1, literal.size() - 2);
	}
	else
	{
		literal = params[literal];
	}
	switch (ps.size())
	{
		case 2:
			return literal.substr(atoi(ps[1].c_str()));
		case 3:
			return literal.substr(atoi(ps[1].c_str()), atoi(ps[2].c_str()));
		default:
			assert(false);
	}
	return "";
}
