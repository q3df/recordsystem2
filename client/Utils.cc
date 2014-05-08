#include <cstdio>
#include <string>

#include "Utils.h"
#include "Quake3.h"
#include "Recordsystem.h"

#if _MSC_VER
#define snprintf _snprintf
#endif

const char *Utils::InfoValueForKey(const char *infoString, const char *key) {
	char	pkey[BIG_INFO_KEY];
	static	char value[2][BIG_INFO_VALUE];	// use two buffers so compares
											// work without stomping on each other
	static int valueindex = 0;
	char *o;
	const char *s = infoString;

	if ( !s || !key ) {
		return "";
	}

	if ( strlen(s) >= BIG_INFO_STRING ) {
		RS_PrintWarning(va("Utils::InfoValueForKey: strlen(userInfo_) >= %i | key = %s", BIG_INFO_STRING, key));
		return "";
	}

	valueindex ^= 1;
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
			return value[valueindex];

		if (!*s)
			break;
		s++;
	}

	return "";
}

void Utils::InfoSetValueForKey(char *infoString, const char *key, const char *value) {
	char  newi[BIG_INFO_STRING];
	const char* blacklist = "\\;\"";

	if ( strlen( infoString ) >= BIG_INFO_STRING ) {
		RS_PrintWarning("Utils::InfoSetValueForKey: oversize infostring\n");
	}

	for(; *blacklist; ++blacklist) {
		if (strchr (key, *blacklist) || strchr (value, *blacklist)) {
			RS_PrintWarning(va("Utils::InfoSetValueForKey: Can't use keys or values with a '%c': %s = %s\n", *blacklist, key, value));
			return;
		}
	}

	InfoRemoveKey(infoString, key);
	if (!value || !strlen(value))
		return;

	snprintf(newi, sizeof(newi), "\\%s\\%s", key, value);

	if (strlen(newi) + strlen(infoString) >= BIG_INFO_STRING) {
		RS_PrintWarning("Utils::InfoSetValueForKey: Info string length exceeded\n");
		return;
	}

	strcat (newi, infoString);
	strcpy (infoString, newi);
}

void Utils::InfoRemoveKey(char *infoString, const char *key) {
	char  *start;
	char  pkey[BIG_INFO_KEY];
	char  value[BIG_INFO_VALUE];
	char  *o;

	if ( strlen( infoString ) >= BIG_INFO_STRING ) {
		RS_PrintWarning("Utils::InfoRemoveKey: oversize infostring\n");
	}

	if (strchr (key, '\\')) {
		return;
	}

	while (1) {
		start = infoString;
		if (*infoString == '\\')
			infoString++;

		o = pkey;
		while (*infoString != '\\') {
			if (!*infoString)
				return;

			*o++ = *infoString++;
		}
		*o = 0;
		infoString++;

		o = value;
		while (*infoString != '\\' && *infoString) {
			if (!*infoString)
				return;

			*o++ = *infoString++;
		}
		*o = 0;

		if (!strcmp (key, pkey) ) {
			memmove(start, infoString, strlen(infoString) + 1); // remove this part
			return;
		}

		if (!*infoString)
			return;
	}
}

bool Utils::InfoStringValidate( const char *s ) {
	if ( strchr( s, '\"' ) ) {
		return false;
	}
	if ( strchr( s, ';' ) ) {
		return false;
	}
	return true;
}