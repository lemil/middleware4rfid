#include <map>

#include "TdtServantI.h"

using namespace std;

std::string TdtServantI::translate(const std::string& input, const std::string& parms, const std::string& outformat, const Ice::Current& current)
{
	list<string> formats;
	formats.push_back("BINARY");
	formats.push_back("LEGACY");
	formats.push_back("LEGACY_AI");
	formats.push_back("TAG_ENCODING");
	formats.push_back("PURE_IDENTITY");
	formats.push_back("ONS_HOSTNAME");

	if (find(formats.begin(), formats.end(), outformat) == formats.end())
	{
		return "";
	}

	_tableMutex.lock();
	if (trans_table.size() != 0)	
		refreshTranslations(current);	
	_tableMutex.unlock();

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

	for (int i = 0; i<trans_table.size(); i++)
	{
		ns1__EpcTagDataTranslation *item = &trans_table[i];
		item->
	}

	return "";
}
void TdtServantI::refreshTranslations(const Ice::Current&)
{
	_tableMutex.lock();
	trans_table.clear();
	int i=0;
	while(xmlfiles[i++].name != 0)
	{
		ns1__EpcTagDataTranslation item;
		struct soap soap;
		soap_init(&soap);
		std::stringstream s(std::ios_base::in | 
				std::ios_base::out | std::ios_base::binary);
		s.write(xmlfiles[i].start, xmlfiles[i].end - xmlfiles[i].start);
		s.seekg(0, std::ios::beg);
		soap.is = &s;
		if (!soap_read_ns1__EpcTagDataTranslation(&soap, &item))
		{
			trans_table.push_back(item);	
		}
		soap_free_temp(&soap);
	}
	_tableMutex.unlock();	
}

