#include <stdio.h>
#include "Plugin.h"
#include "../Recordsystem.h"
#include "../Logger.h"

class ServerRecordsPlugin : public PluginBase {
public:
	virtual void Init() {
	}

	virtual void Destroy() {
	}
};

// Register plugin!
static PluginProxy<ServerRecordsPlugin> gServerRecordsPlugin;
