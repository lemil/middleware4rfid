#include <assert.h>

#include "tdt.h"
#include "table.h"


#define MAX_TABLE_SIZE 21
struct tdt__EpcTagDataTranslation *table[MAX_TABLE_SIZE] = {0};

int init_translations()
{
	free_translations();
	
	int i = 0;
	while(xmlfiles[i].name != 0)
	{
		struct tdt__EpcTagDataTranslation *trans = 
				(struct tdt__EpcTagDataTranslation*)malloc(
						sizeof(struct tdt__EpcTagDataTranslation));
		assert(trans);

		FILE *fd = fmemopen(xmlfiles[i].start, 
				xmlfiles[i].end - xmlfiles[i].start, "r");
		assert(fd);

		struct soap soap;
		soap_init(&soap);
		soap_begin(&soap);
		soap.recvfd = fd;
		if (soap_read_tdt__EpcTagDataTranslation(&soap, trans))
		{
			table[i] = trans;
		}
		fclose(fd);
		i++;
	}
}

int free_translations()
{
	int i;
	for (i = 0; i < MAX_TABLE_SIZE; i++)
	{
		if (table[i])
		{
			free(table[i]);
			table[i] = NULL;
		}
	}
}

char *translate(const char *input, 
		const char *parameters, 
		enum tdt__LevelTypeList output_format)
{
	return NULL;
}

void refresh_translations()
{
}

