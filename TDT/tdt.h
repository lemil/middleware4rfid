#ifndef __TDT_H__
#define __TDT_H__

#include "soapH.h"

int init_translations();
int free_translations();

char *translate(const char *input, 
		const char *parameters, 
		enum tdt__LevelTypeList output_format);

void refresh_translations();

#endif /* __TDT_H__ */

