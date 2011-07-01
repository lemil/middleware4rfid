#include <Ice/Ice.h>
#include <iostream>
#include <iterator>
#include "TDT.h"

using namespace std;
using namespace tdt;

int main(int argc, char* argv[])
{
    int status = 0;
    Ice::CommunicatorPtr ic;
    try {
        // Create a communicator
        //
        ic = Ice::initialize(argc, argv);

        // Create a proxy for the root directory
        //
        Ice::ObjectPrx base = ic->stringToProxy("TdtService:default -p 10000");
        if (!base)
            throw "Could not create proxy";

        // Down-cast the proxy to a TdtService proxy
        //
        TdtServicePrx tdt = TdtServicePrx::checkedCast(base);
        if (!tdt)
            throw "Invalid proxy";

        // Recursively list the contents of the root directory
        //
		tdt->refreshTranslations();
    } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
        status = 1;
    } catch (const char* msg) {
        cerr << msg << endl;
        status = 1;
    }

    // Clean up
    //
    if (ic)
        ic->destroy();

    return status;
}
