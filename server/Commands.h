#ifndef Server_CommandsProxyBase_H_
#define Server_CommandsProxyBase_H_

#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <sstream>
#include <google/protobuf/rpc/rpc_conn.h>

using namespace std;
namespace po = boost::program_options;

class CommandBase {
protected:
	po::options_description description_;

public:
	CommandBase();
	~CommandBase();

	virtual void Init() = 0;
	virtual void Destroy() = 0;
	virtual const string &Name() = 0;
	virtual bool Execute(const vector<string> *args, google::protobuf::rpc::Conn *contextCon, string *output) = 0;

	const po::options_description &GetDescription();
};


class CommandProxyBase {
public:
	CommandProxyBase();
	~CommandProxyBase();

	virtual CommandBase *Create() const = 0;
};

typedef vector<CommandProxyBase const*> CommandVector;


template<class T>
class CommandProxy : public CommandProxyBase {
public:
	CommandBase *Create() const {
		return new T;
	}
};


class CommandStore {
public:
	CommandStore();
	~CommandStore();
	void Register(const CommandProxyBase* pBase);
	int GetCount();
	const CommandProxyBase *GetAt(int i);

private:
	void Init();
	CommandVector* pConverters_;
};

extern CommandStore gCommandStore;

#define RegisterCommand(className) static CommandProxy<className> g##className

#endif // Server_CommandsProxyBase_H_
