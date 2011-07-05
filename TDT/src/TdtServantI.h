#ifndef __TDTSERVANTI_H__
#define __TDTSERVANTI_H__

#include <list>
#include <map>
#include <vector>

#include <IceUtil/Mutex.h>
#include "TDT.h"

#include "table.h"
#include "soapH.h"

class TdtServantI : public tdt::TdtService 
{
public:
	TdtServantI();
	virtual ~TdtServantI();

	virtual std::string translate(const std::string&, const std::string&, const std::string&, const Ice::Current&);
	virtual void refreshTranslations(const Ice::Current&);

private:
	void reloadTable();
	void translate1(const std::string &input, 
			std::map<std::string, std::string> &parameters, 
			ns1__LevelTypeList &outLevel, 
			ns1__EpcTagDataTranslation &trans, 
			ns1__Scheme &scheme, 
			ns1__Level &level);
	void translate2(const std::string &input, 
			std::map<std::string, std::string> &parameters, 
			ns1__LevelTypeList &outLevel, 
			ns1__EpcTagDataTranslation &trans, 
      ns1__Scheme &scheme, 
			ns1__Level &level, 
			ns1__Option &option, 
			std::vector<std::string> &segments);


	std::vector<ns1__EpcTagDataTranslation *> _table;
	IceUtil::Mutex _tableMutex;
	std::map<std::string, ns1__LevelTypeList> _formats;
};

#endif //__TDTSERVANTI_H__
