#include "VmCvarItem.h"
#include <memory.h>

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

int VmCvarItem::GetValueAsInteger() {
	return this->vmCvar->integer;
}

const char *VmCvarItem::GetValueAsString() {
	return this->vmCvar->string;
}

float VmCvarItem::GetValueAsFloat() {
	return this->vmCvar->value;
}