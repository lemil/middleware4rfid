#ifndef __TDT_H__
#define __TDT_H__

#include <pthread.h>

#include <map>
#include <exception>

#include "soapH.h"

enum TDTExceptionReason
{
	TDTBitLengthOverflow,
	TDTDifferentLength,

	TDTFieldBelowMinimum,
	TDTFieldAboveMaximum,
	TDTFieldOutsideCharacterSet
};

class TDTTranslationException : public std::exception
{
public:
	TDTTranslationException(enum TDTExceptionReason reason): _reason(reason) {}
private:
	enum TDTExceptionReason _reason;
};

class TDTEngine
{
public:
	TDTEngine();
	virtual ~TDTEngine();
	std::string translate(const std::string &input, const std::string &parameters, 
			enum tdt__LevelTypeList output_format);

	void refresh_translations();

private:
	std::vector<tdt__EpcTagDataTranslation *> _table;
	pthread_rwlock_t _table_lock;
	void load_trans_table();
	void unload_trans_table();
	std::map<std::string, std::string> parse_parameters(const std::string &parameters);
	std::string translate1(const std::string &input, 
			std::map<std::string, std::string> &params, 
			enum tdt__LevelTypeList output_format, 
			tdt__EpcTagDataTranslation *trans, 
			tdt__Scheme *scheme, 
			tdt__Level *level);
	std::string translate2(const std::string &input, 
			std::map<std::string, std::string> &params, 
			enum tdt__LevelTypeList output_format, 
			tdt__EpcTagDataTranslation *trans, 
			tdt__Scheme *scheme, 
			tdt__Level *level, 
			tdt__Option *option,
      std::vector<std::string> &segments);
	void apply_extract_rule(std::map<std::string, std::string> &params, 
			enum tdt__LevelTypeList output_format,
			tdt__EpcTagDataTranslation *trans, 
			tdt__Scheme *scheme, 
			tdt__Level *level, 
			tdt__Option *option,
			tdt__Rule *rule);
  std::string apply_substr(std::map<std::string, std::string> &params, 
			enum tdt__LevelTypeList output_format, 
			tdt__EpcTagDataTranslation *trans, 
			tdt__Scheme *scheme, 
			tdt__Level *level, 
			const std::vector<std::string> &ps);
};

#endif /* __TDT_H__ */

