#include <stdio.h>
#include "Plugin.h"
#include "../Recordsystem.h"
#include "../Logger.h"

#include <tinyxml.h>



namespace {
	char* my_itoa(int val, int base) {
		static char buf[32] = {0};
		int i = 30;

		memset(&buf, 0, sizeof(buf));

		for(; val && i ; --i, val /= base)
			buf[i] = "0123456789abcdef"[val % base];

		return &buf[i+1];
	}
}

class ServerDemosPlugin : public PluginBase {
public:
	virtual void Init() {
	}

	virtual void Destroy() {
	}
};

// Register plugin!
static PluginProxy<ServerDemosPlugin> gServerDemosPlugin;
