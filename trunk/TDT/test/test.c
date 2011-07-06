#include "tdt.h"

int main()
{
	init_translations();
	char *bin = translate("urn:epc:tag:sgtin-96:1.0020003.999717.000405000006", NULL, tdt__LevelTypeList__BINARY);
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
