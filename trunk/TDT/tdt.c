#include <assert.h>

#include "tdt.h"
#include "table.h"


#define MAX_TABLE_SIZE 21
static struct tdt__EpcTagDataTranslation *table[MAX_TABLE_SIZE] = {0};

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

		FILE *file = fmemopen(xmlfiles[i].start, 
				xmlfiles[i].end - xmlfiles[i].start, "r");
		assert(file);

		struct soap soap;
		soap_init(&soap);
		soap_begin(&soap);
		soap.recvfd = fileno(file);
		if (soap_read_tdt__EpcTagDataTranslation(&soap, trans))
		{
			table[i] = trans;
		}
		fclose(file);
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

#define MAX_PARAMETERS 10
static char *keys[MAX_PARAMETERS];
static char *values[MAX_PARAMETERS];

void release_parameters()
{
	int i;
	for (i=0; i<MAX_PARAMETERS; i++)
	{
		if (keys[i])
		{
			free(keys[i]);
			keys[i] = NULL;
		}
		if (values[i])
		{
			free(values[i]);
			values[i] = NULL;
		}
	}
}

int parse_parameters(const char *parameters)
{
	release_parameters();
	if (!parameters)
		return 1;

	const char *pattern = "^([^=;]+)=([^=;]+)(;([^=;]+)=([^=;]+))*$";
	regex_t reg;
	assert(regcomp(&reg, pattern, REG_EXTENDED) == 0);
	const size_t nmatch = 100;
	regmatch_t match[nmatch];
	if (regexec(&reg, parameters, nmatch, match, 0) == REG_NOMATCH)
	{
		regfree(&reg);
		return 2;
	}
	int t=0;
	while(match[t].rm_so != -1)
	{
		fprintf(stdout, "match[%d]=%s\n", t, strndup(parameters + match[t].rm_so, match[t].rm_eo -match[t].rm_so));
		t++;
	}
	int idx = 0;
	int forkey = 1;
	int i = 1;
	int first = 1;
	while (match[i].rm_so != -1)
	{
		if (forkey)
		{
			keys[idx] = strndup(parameters + match[i].rm_so, match[i].rm_eo - match[i].rm_so);
fprintf(stdout, "keys[%d] = %s\n", idx, keys[idx]);
		}
		else
		{
			values[idx] = strndup(parameters + match[i].rm_so, match[i].rm_eo - match[i].rm_so);
fprintf(stdout, "values[%d] = %s\n", idx, values[idx]);
			idx ++;
	/*		if (first)
			{
				first = 0;
				i++;
			}
*/
		}
		i++;
		forkey = !forkey;
	}
	if (!forkey)
	{
		return 3;
	}
	regfree(&reg);
}

char *translate(const char *input, 
		const char *parameters, 
		enum tdt__LevelTypeList output_format)
{

	assert(parse_parameters(parameters) == 0);

	release_parameters();	
	return NULL;
}

void refresh_translations()
{
}

