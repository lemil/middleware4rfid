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
  try 
	{ 
		log4cpp::PropertyConfigurator::configure("log4cpp.conf");
	} 
	catch(log4cpp::ConfigureFailure& f) 
	{
		std::cerr << "Configure Problem " << f.what() << std::endl;
		exit(-1);
  }
  _log = &log4cpp::Category::getInstance("TDTEngine");
	assert(_log);

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
	_log->info("begin translate");
	string ret;
	map<string, string> params = parse_parameters(parameters);
	string taglength;
	if (params.find("taglength") != params.end())
		taglength = params["taglength"];
	_log->info("taglength=%s", taglength.c_str());

	pthread_rwlock_rdlock(&_table_lock);
	for(vector<tdt__EpcTagDataTranslation *>::iterator it1 = _table.begin();
			it1 != _table.end(); it1++)
	{
		_log->info("tdt__EpcTagDataTranslation");
		tdt__EpcTagDataTranslation *trans = *it1;
		for (vector<tdt__Scheme *>::iterator it2 = trans->scheme.begin();
				it2 != trans->scheme.end(); it2++)
		{
			_log->info("tdt__Scheme");
			tdt__Scheme *scheme = *it2;
			_log->info("schemd.name=%s", scheme->name.c_str());
			string tagLength = scheme->tagLength;
			_log->info("taglength=%s,tagLength=%s", taglength.c_str(), tagLength.c_str());
			// tagLength missmatch
			if (taglength.empty() || tagLength != taglength)
				continue;
			
			for (vector<tdt__Level *>::iterator it3 = scheme->level.begin();
					it3 != scheme->level.end(); it3++)
			{
				_log->info("tdt__Level");
				tdt__Level *level = *it3;
				// prefixMatch missmatch
				if ((*it3)->prefixMatch)
				{
					if (input.substr(0, level->prefixMatch->size()) 
								!= *(level->prefixMatch))
					{
						continue;
					}
				}
	
				for (vector<tdt__Option *>::iterator it4 = level->option.begin();
						it4 != level->option.begin(); it4++)
				{
					tdt__Option *option = *it4;
					if (option->pattern)
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
							ret = translate2(input, params, output_format, trans, scheme, 
												level, option, segments);
							if (!ret.empty())
							{
								pthread_rwlock_unlock(&_table_lock);
								return ret;
							}
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
	pthread_rwlock_unlock(&_table_lock);
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
		tdt__EpcTagDataTranslation *trans = new tdt__EpcTagDataTranslation;
		struct soap soap;
		soap_init(&soap);
		string buf(xmlfiles[i].start, xmlfiles[i].end - xmlfiles[i].start);
		stringstream stream(buf);
		soap.is = &stream;
		if (soap_read_tdt__EpcTagDataTranslation(&soap, trans) == SOAP_OK)
		{
			_table.push_back(trans);
		}
		soap_free_temp(&soap);
		i++;
	}
	_log->info("_table.size=%d", _table.size());
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
	_log->info("parameters=%s", parameters.c_str());
	map<string, string> ret;
	size_t start = 0;
	size_t end;
	do
	{
		end = parameters.find(';', start);
		string str;
		if (end == string::npos)
			str = parameters.substr(start);
		else
			str = parameters.substr(start, end - start);
		size_t pos = str.find('=');
		if (pos != string::npos)
		{
			ret[str.substr(0, pos)] = str.substr(pos + 1);
			_log->info("reg[%s]=%s", str.substr(0, pos).c_str(), 
					str.substr(pos + 1).c_str());
		}
		if (end == string::npos)
			break;
		start = end + 1;
	} while(true);
	_log->info("ret.size=%d", ret.size());
	return ret;	
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
	_log->info("begin translate2");
	assert(segments.size() == option->field.size());

	for (int i=0; i != option->field.size(); ++i)
	{
		tdt__Field *field = option->field[i];
		_log->info("cheching field");
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
		_log->info("params[%s]=%s", field->name.c_str(), segments[i].c_str());
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

	for (vector<tdt__Level *>::iterator it = scheme->level.begin();
			it != scheme->level.end(); it++)
	{
		if ((*it)->type == output_format)
		{
			for (vector<tdt__Rule *>::iterator it2 = (*it)->rule.begin();
					it2 != (*it)->rule.end(); it2++)
			{
				apply_format_rule(params, output_format, trans, scheme, level, *it, option, *it2);
			}
			break;
		}
	}
	return "";
}

string TDTEngine::apply_rule_function(
			std::map<std::string, std::string> &params, 
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
		assert(false);
	}
	else if (rule->function.find("LENGTH(") == 0)
	{
		//apply_length();
		assert(false);
	}
	else if (rule->function.find("GS1CHECKSUM(") == 0)
	{
		//apply_gs1checksum();
		assert(false);
	}
	else if (rule->function.find("TABLELOOKUP(") == 0)
	{
		//apply_tablelookup();
		assert(false);
	}
	return ret;
}

void TDTEngine::apply_extract_rule(std::map<std::string, std::string> &params, 
      enum tdt__LevelTypeList output_format, 
      tdt__EpcTagDataTranslation *trans, 
      tdt__Scheme *scheme,
      tdt__Level *level,
      tdt__Option *option,
      tdt__Rule *rule)
{
	_log->info("begin apply_extract_rule");
	string ret;
	ret = apply_rule_function(params, output_format, trans, scheme, level, 
						option, rule);
	params[rule->newFieldName] = ret;
	_log->info("params[%s]=%s", rule->newFieldName.c_str(), ret.c_str());
}

void TDTEngine::apply_format_rule(std::map<std::string, std::string> &params, 
      enum tdt__LevelTypeList output_format, 
      tdt__EpcTagDataTranslation *trans, 
      tdt__Scheme *scheme,
      tdt__Level *in_level,
      tdt__Level *out_level,
      tdt__Option *option,
      tdt__Rule *rule)
{
	_log->info("begin apply_format_rule");
	string ret;
	ret = apply_rule_function(params, output_format, trans, scheme, out_level, 
						option, rule);
	params[rule->newFieldName] = ret;
	_log->info("params[%s]=%s", rule->newFieldName.c_str(), ret.c_str());
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
