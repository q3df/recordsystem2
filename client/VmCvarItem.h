#ifndef CLIENT_VMCVARITEM_H_
#define CLIENT_VMCVARITEM_H_

#include "Quake3.h"

class VmCvarItem {
friend class Recordsystem;
public:
	VmCvarItem(vmCvar_t	*cvarPtr,
		const char name[MAX_CVAR_VALUE_STRING],
		const char defaultValue[MAX_CVAR_VALUE_STRING],
		int flags,
		qboolean track
	);
	~VmCvarItem();

private:
	vmCvar_t	*vmCvar;
	char		cvarName[MAX_CVAR_VALUE_STRING];
	char		defaultString[MAX_CVAR_VALUE_STRING];
	int			cvarFlags;
	int			modificationCount;  // for tracking changes
	qboolean	trackChange;	    // track this variable, and announce if changed
};

#endif // CLIENT_VMCVARITEM_H_