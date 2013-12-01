#include <vector>
#include "PluginStore.h"
#include "PluginBase.h"
#include "PluginProxy.h"

PluginStore::PluginStore() {
}

PluginStore::~PluginStore() {
	delete pConverters_;
}

void PluginStore::Init() {
	if(!pConverters_)
		pConverters_ = new PluginVector();
}


void PluginStore::Register(const PluginProxyBase* pBase) {
	Init();
	pConverters_->push_back(pBase);
}

int PluginStore::GetCount() {
	return pConverters_->size();
}

const PluginProxyBase *PluginStore::GetAt(int i) {
	return pConverters_->at(i);
}