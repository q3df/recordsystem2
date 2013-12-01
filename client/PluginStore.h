#ifndef CLIENT_PLUGINSTORE_H_
#define CLIENT_PLUGINSTORE_H_

class PluginBase;
class PluginProxyBase;

typedef std::vector<const PluginProxyBase*> PluginVector;

class PluginStore {
public:
	PluginStore();
	~PluginStore();

	void Register(const PluginProxyBase* proxy);
	int GetCount();
	const PluginProxyBase *GetAt(int i);

private:
	void Init();
	PluginVector* pConverters_;
};

extern PluginStore gPluginStore;

#endif // CLIENT_PLUGINPROXY_H_