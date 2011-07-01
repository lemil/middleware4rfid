#ifndef __TDTSERVANTI_H__
#define __TDTSERVANTI_H__

#include <vector>

#include <IceUtil/Mutex.h>
#include "TDT.h"

#include "table.h"
#include "soapH.h"

class TdtServantI : public tdt::TdtService 
{
public:
	virtual std::string translate(const std::string&, const std::string&, const std::string&, const Ice::Current&);
	virtual void refreshTranslations(const Ice::Current&);

private:
	std::vector<ns1__EpcTagDataTranslation> trans_table;
	IceUtil::Mutex _tableMutex;
};

#endif //__TDTSERVANTI_H__
