#include "Commands.h"
#include "RecordsystemDaemon.h"

CommandStore gCommandStore;

CommandBase::CommandBase() {
}


CommandBase::~CommandBase() {
}


po::options_description const& CommandBase::GetDescription() {
	return this->description_;
}


CommandStore::CommandStore() {
}


CommandStore::~CommandStore() {
	delete pConverters_;
}


void CommandStore::Init() {
	if (!pConverters_)
		pConverters_ = new CommandVector();
}


void CommandStore::Register(CommandProxyBase const* pBase) {
	Init();
	pConverters_->push_back(pBase);
}


int CommandStore::GetCount() {
	return pConverters_->size();
}


CommandProxyBase::CommandProxyBase() {
	gCommandStore.Register(this);
}


CommandProxyBase::~CommandProxyBase() {
}


CommandProxyBase const *CommandStore::GetAt(int i) {
	return pConverters_->at(i);
}
