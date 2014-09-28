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
	virtual bool IsAlsoProxymodCommand() = 0;
	virtual bool Execute(const vector<string> *args, google::protobuf::rpc::Conn *contextCon, string *output) = 0;

	inline void ParseOptions(const vector<string> *args, po::variables_map *options, vector<string> *optionalOptions) {
		po::parsed_options t = po::command_line_parser(*args).options(this->description_).allow_unregistered().run();
		po::store(t, *options);
		po::notify(*options);

		*optionalOptions = po::collect_unrecognized(t.options, po::include_positional);

		/*		for(vector<string>::iterator it = tmp.begin(); it != tmp.end(); ++it)
			optionalOptions*/
	}

	inline char *MstimeToDftime(int mstime, int opt) {
		int a, b, c, i, len;
		char *mem;

		a = mstime / 60000;
		b = (mstime % 60000) / 1000;
		c = ((mstime % 60000) % 1000);

		len = 0;
		i = a;
		while (i > 0) {
			i /= 10;
			len++;
		}

		if (a != 0)
			len++;		/* create space for a ':' */

		i = b;
		while (i > 0) {
			i /= 10;
			len++;
		}
		if (b == 0) {
			len++;		/* len would be 0, but we need at least 1 for a '0' */
			if (a != 0)
				len++;	/* there should be space for a leading zero if the time is longer than 1 minute */
		}
		len++;			/* create space for a ':' */
		len += 3;		/* the last 3 diges will always be there */

		mem = (char *)malloc(len + 1);
		if (opt == 1) {
			len++;
			if (a == 0)
				if (b == 0)
					if (c == 0)
						sprintf(mem, "-0:000");
					else
						sprintf(mem, "-0:%.3d", c);
				else
					sprintf(mem, "-%d:%.3d", b, c);
			else
				sprintf(mem, "-%d:%.2d:%.3d", a, b, c);	/* there should be a leading zero at the 'seconds'-part wenn the time is longer than 1 minute */
		}
		if (opt == 2) {
			len++;
			if (a == 0)
				if (b == 0)
					if (c == 0)
						sprintf(mem, "+0:000");
					else
						sprintf(mem, "+0:%.3d", c);
				else
					sprintf(mem, "+%d:%.3d", b, c);
			else
				sprintf(mem, "+%d:%.2d:%.3d", a, b, c);	/* there should be a leading zero at the 'seconds'-part wenn the time is longer than 1 minute */
		}
		if (opt == 0) {
			if (a == 0)
				if (b == 0)
					if (c == 0)
						sprintf(mem, "0:000");
					else
						sprintf(mem, "0:%.3d", c);
				else
					sprintf(mem, "%d:%.3d", b, c);
			else
				sprintf(mem, "%d:%.2d:%.3d", a, b, c);	/* there should be a leading zero at the 'seconds'-part wenn the time is longer than 1 minute */
		}
		return mem;
	}


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
