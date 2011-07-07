#include "tdt.h"

int main()
{
	init_translations();
	//char *bin = translate("urn:epc:tag:sgtin-96:1.0020003.999717.000405000006", NULL, tdt__LevelTypeList__BINARY);
	char *bin = translate("gtin=96902018997174;serial=110224000001", "taglength=96;filter=3;companyprefixlength=7", tdt__LevelTypeList__TAG_USCOREENCODING);
	if (bin)
	{
		fprintf(stdout, "bin=%s\n", bin);
		free(bin);
	}
	else
	{
		fprintf(stderr, "bin=NULL\n");
	}
	free_translations();
}
