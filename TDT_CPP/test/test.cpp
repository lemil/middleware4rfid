#include "tdt.h"

using namespace std;

int main()
{
	TDTEngine tdt;
	string bin = tdt.translate("gtin=96902018997174;serial=110224000001", "taglength=96;filter=3;companyprefixlength=7", tdt__LevelTypeList__TAG_USCOREENCODING);
	if (!bin.empty())
	{
		cout << "bin=%s" << bin << endl;
	}
	else
	{
		cerr << "bin=NULL" << endl;
	}
}
