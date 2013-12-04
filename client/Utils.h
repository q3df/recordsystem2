class Utils {
public:
	static const char *InfoValueForKey(const char *infoString, const char *key);
	static void InfoSetValueForKey(char *infoString, const char *key, const char *value);
	static void InfoRemoveKey(char *infoString, const char *key);
	static bool InfoStringValidate( const char *s );
};