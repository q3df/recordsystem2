#ifndef CLIENT_PLUGINPROXY_H_
#define CLIENT_PLUGINPROXY_H_

class PluginProxyBase {
public:
	PluginProxyBase();
	virtual PluginBase *Create() const = 0;
};

inline PluginProxyBase::PluginProxyBase() {
	gPluginStore.Register(this);
}


template <class T>
class PluginProxy : public PluginProxyBase {
public:
	virtual PluginBase *Create() const {
		return new T;
	}
};

#define RegisterPlugin(className) static PluginProxy<className> g##className

#endif // CLIENT_PLUGINPROXY_H_
