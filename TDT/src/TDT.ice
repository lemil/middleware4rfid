module tdt
{
	enum TDTExceptionReason 
	{
		TDTFileNotFound,
		TDTFieldBelowMinimum,
		TDTFieldAboveMaximum,
		TDTFieldOutsideCharacterSet,
		TDTUndefinedField,
		TDTSchemeNotFound,
		TDTLevelNotFound,
		TDTOptionNotFound,
		TDTLookupFailed,
		TDTNumericOverflow
	};

	exception TDTTranslationException
	{
		TDTExceptionReason reason;
	};

	interface TdtService 
	{
		string translate(string epcIdentifier, 
				string parameterList, 
				string outputFormat) throws TDTTranslationException;

		 void refreshTranslations();
	};
};

		
		

