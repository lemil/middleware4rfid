#include <sys/types.h>
#include <regex.h>

#include <map>
#include <list>
#include <iostream>

#include <Ice/Service.h>
#include <Ice/LoggerUtil.h>
#include "TdtServantI.h"

using namespace std;

TdtServantI::TdtServantI()
{
	// init supported output format
	if (_formats.empty())
	{
		_formats["BINARY"] = ns1__LevelTypeList__BINARY;
		_formats["LEGACY"] = ns1__LevelTypeList__LEGACY;
		_formats["LEGACY_AI"] = ns1__LevelTypeList__LEGACY_USCOREAI;
		_formats["TAG_ENCODING"] = ns1__LevelTypeList__TAG_USCOREENCODING;
		_formats["PURE_IDENTITY"] = ns1__LevelTypeList__PURE_USCOREIDENTITY;
		_formats["ONS_HOSTNAME"] = ns1__LevelTypeList__ONS_USCOREHOSTNAME;
	}

	// load table
	reloadTable();
}

TdtServantI::~TdtServantI()
{
  _tableMutex.lock();
	for(int i = 0; i < _table.size(); i++)
	{
		delete _table[i];
	};
  _table.clear();
	_tableMutex.unlock();	
}

std::string TdtServantI::translate(const std::string& input, const std::string& parms, const std::string& outformat, const Ice::Current& current)
{

	/*Ice::Trace trace(current.adapter->getCommunicator()->getLogger(), 
			"TdtServantI::translate");
	trace << "TdtServantI::translate";
*/

	ns1__LevelTypeList outLevel;
	map<string, ns1__LevelTypeList>::iterator itor = _formats.find(outformat);
	if (itor == _formats.end())
	{
		// outformat not found
		return "";
	}
	outLevel = itor->second;

	// parse parameters
	map<string, string> parameters;
	size_t start = 0;
	size_t end = parms.find(";", start);
	while (end != string::npos)
	{
		string sub = parms.substr(start, end - start);
		size_t idx = sub.find("=");
		if (idx == string::npos)
			continue;
		string key = sub.substr(0, idx);
		string value = sub.substr(idx + 1);
		parameters[key] = value;
		start = end + 1;	
	}
	
	for (int i = 0; i < _table.size(); i++)
	{
		ns1__EpcTagDataTranslation* trans = _table[i];
		for (int j = 0; j < trans->scheme.size(); j++)
		{
			ns1__Scheme *scheme = trans->scheme[j];
			for (int k = 0; k < scheme->level.size(); k++)
			{
				ns1__Level *level = scheme->level[k];
				if (level->prefixMatch)
				{
					size_t len = level->prefixMatch->size();
					if (input.substr(0, len) == *(level->prefixMatch))
					{
						translate1(input, parameters, outLevel, *trans, 
								*scheme, *level);
					}
				}
				else 
				{
					for (int l = 0; l < level->option.size(); l++)
					{
						ns1__Option *option = level->option[l];
						if (option->pattern)
						{
							regex_t reg;
							string pattern = "^" + (*(option->pattern)) + "$";
							if (regcomp(&reg, pattern.c_str(), 
									REG_EXTENDED) == 0)	
							{
#define MAX_FIELD 10
								regmatch_t match[MAX_FIELD];
								if (regexec(&reg, input.c_str(), MAX_FIELD, match, 
										0) != 0)
								{
									regfree(&reg);
									continue;
								}
								vector<string> segments;
								for (int i = 1; i < MAX_FIELD; i++)
								{
									if (match[i].rm_so == -1)
										break;
									segments.push_back(input.substr(match[i].rm_so,
											match[i].rm_eo - match[i].rm_so));
								}
								regfree(&reg);
								translate2(input, parameters, outLevel, *trans, 
														*scheme, *level, *option, segments);
							}
						}
					}
				}
			}
		}
	}

	return "";
}

void TdtServantI::refreshTranslations(const Ice::Current&)
{
	cout << "TdtServantI::refreshTranslations" << endl;
	reloadTable();
}

void TdtServantI::reloadTable()
{
	_tableMutex.lock();
	_table.clear();
	int i=0;
	while(xmlfiles[i++].name != 0)
	{
		ns1__EpcTagDataTranslation *item = new ns1__EpcTagDataTranslation;
		struct soap soap;
		soap_init(&soap);
		std::stringstream s(std::ios_base::in | 
				std::ios_base::out | std::ios_base::binary);
		s.write(xmlfiles[i].start, xmlfiles[i].end - xmlfiles[i].start);
		s.seekg(0, std::ios::beg);
		soap.is = &s;
		if (!soap_read_ns1__EpcTagDataTranslation(&soap, item))
		{
			_table.push_back(item);	
		}
		else
		{
cout << "error reading " << xmlfiles[i-1].name << endl;	
		}
		//soap_free_temp(&soap);
	}
cout << "table_cout=" << _table.size() << endl;
	_tableMutex.unlock();	
}

void TdtServantI::translate1(const string &input, 
      map<string, string> &parameters, 
      ns1__LevelTypeList &outLevel, 
      ns1__EpcTagDataTranslation &trans, 
      ns1__Scheme &scheme, 
      ns1__Level &level)
{
	cout << "TdtServantI::translate1" << endl;	
}

void TdtServantI::translate2(const std::string &input, 
      std::map<std::string, std::string> &parameters, 
      ns1__LevelTypeList &outLevel, 
      ns1__EpcTagDataTranslation &trans,
      ns1__Scheme &scheme, 
      ns1__Level &level, 
      ns1__Option &option, 
      std::vector<std::string> &segments)
{
	cout << "TdtServantI::translate2" << endl;	
}
