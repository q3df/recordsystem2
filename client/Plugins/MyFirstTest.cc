#include <stdio.h>
#include "Plugin.h"
#include "../Recordsystem.h"
#include "../Logger.h"

class MyFirstTest : public PluginBase {
public:
	virtual void Init() const {
		printf("Init MyFirstPlugin\n");
		LOG(LOG_INFO, "%s", "STARTING MY FIRST PLUGIN");
	}

	virtual void Destroy() const {
		printf("Destroy MyFirstPlugin\n");
	}
};

// Register plugin!
static PluginProxy<MyFirstTest> gMyFirstPluginProxy;

