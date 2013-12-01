#ifndef CLIENT_PLUGINBASE_H_
#define CLIENT_PLUGINBASE_H_

class PluginBase {
public:
	PluginBase() {}
	virtual void Init() const = 0;
	virtual void Destroy() const = 0;
};

#endif // CLIENT_PLUGINBASE_H_