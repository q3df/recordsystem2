#include "VmCvarItem.h"
#include <string>

VmCvarItem::VmCvarItem(vmCvar_t	*cvarPtr,
		const char name[MAX_CVAR_VALUE_STRING],
		const char defaultValue[MAX_CVAR_VALUE_STRING],
		int flags,
		qboolean track) {

	this->vmCvar = cvarPtr;
	memcpy(&this->cvarName, name, MAX_CVAR_VALUE_STRING);
	memcpy(&this->defaultString, defaultValue, MAX_CVAR_VALUE_STRING);
	this->trackChange = track;
	this->cvarFlags = flags;
}

VmCvarItem::~VmCvarItem() {
}
