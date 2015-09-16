#include "../RecordsystemDaemon.h"
#include "../Commands.h"
#include <time.h>
#include "client/Q3User.h"


class AdminCommand : public CommandBase {
private:
	string name_;

public:
	AdminCommand() {
		this->description_.add_options()
			("help", "produce help message")
			("command",  po::value<vector<string>>(), "command")
		;

		name_ = string("!admin");
	}


	~AdminCommand() {
		
	}


	virtual void Init() {
		
	}


	virtual void Destroy() {
		
	}


	virtual const string &Name() {
		return this->name_;
	}


	virtual bool IsAlsoProxymodCommand() {
		return true;
	}

	virtual bool Execute(const vector<string>* args, Conn *contextCon, string *output) {
		po::variables_map options;
		vector<string> optionalOptions;
		vector<string>::iterator optionalOptionsIt;
		
		po::positional_options_description p;
		p.add("command", -1);

		this->ParseOptions(args, &options, &optionalOptions, &p);

		if(options.count("help")) {
			ostringstream tmpDesc;
			//this->description_.print(tmpDesc);
			output->append(tmpDesc.str());
			return true;
		}

		if(!options.count("command")) {
			ostringstream tmpDesc;
			//this->description_.print(tmpDesc);
			output->append(tmpDesc.str());
			return true;
		}

		string cmd = options["command"].as<vector<string>>().at(0);
		if(cmd == "test1") {
			output->append("test1 command...\n");
		}else if(cmd == "test2") {
			output->append("test2 command...\n");
		}else{
			ostringstream tmpDesc;
			//this->description_.print(tmpDesc);
			output->append(tmpDesc.str());
		}

		return true;
	}
	


	//void cmd_say() {};
	//void cmd_maprestart() {};
	//void cmd_map() {};
	//void cmd_clientkick() {};
	//void cmd_promode() {};
	//void cmd_mode() {};
	//void cmd_ban() {};
	//void cmd_serverpw() {};
	//void cmd_mode_fastcaps() {};
	//void cmd_pl2spect() {};
	//void cmd_ban() {};
	//void cmd_rename() {};
	//void cmd_help(int bits) {};
};

RegisterCommand(AdminCommand);
