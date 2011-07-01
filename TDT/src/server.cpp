#include <Ice/Service.h>
#include "TdtServantI.h"

class MyTdtService : public Ice::Service 
{
protected:
    virtual bool start(int, char*[], int &);
private:
    Ice::ObjectAdapterPtr _adapter;
};

bool
MyTdtService::start(int argc, char* argv[], int &code)
{
	_adapter = communicator()->createObjectAdapter("TdtAdapter");
	_adapter->add(new TdtServantI, 
			communicator()->stringToIdentity("TdtService"));
	_adapter->activate();
    return true;
}

int main(int argc, char* argv[])
{
	MyTdtService svc;
	return svc.main(argc, argv);
}
