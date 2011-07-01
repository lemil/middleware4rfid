#include "TdtServantI.h"

std::string TdtServantI::translate(const std::string&, const std::string&, const std::string&, const Ice::Current&)
{
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

