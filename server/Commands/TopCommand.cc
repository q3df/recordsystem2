#include "../RecordsystemDaemon.h"
#include "../Commands.h"

class TopCommand : public CommandBase {
private:
	string name_;

public:
	TopCommand() {
		this->description_.add_options()
			("help", "produce help message")
			("startat", po::value<int>()->default_value(0), "starting at record XX")
		;

		name_ = string("!top");
	}


	~TopCommand() {
		
	}


	virtual void Init() {
		
	}


	virtual void Destroy() {
		
	}


	virtual const string &Name() {
		return this->name_;
	}


	virtual bool Execute(const vector<string>* args, Conn *contextCon, string *output) {
		int startAt = 0;

		po::variables_map vm;
		po::store(po::command_line_parser(*args).options(this->description_).allow_unregistered().run(), vm);
		po::notify(vm);

		startAt = vm["startat"].as<int>();

		sql::SQLString map;
		sql::SQLString mode;
		sql::SQLString physic;
		
		if(!contextCon) {
			map = sql::SQLString("cpmjump");
			mode = sql::SQLString("-1");
			physic = sql::SQLString("1");
		}else {
			map = sql::SQLString(RS->Clients()->GetClient(contextCon)->GetServerInfo(string("mapname")).c_str());
			mode = sql::SQLString(RS->Clients()->GetClient(contextCon)->GetServerInfo(string("defrag_mode")).c_str());
			physic = sql::SQLString(RS->Clients()->GetClient(contextCon)->GetServerInfo(string("df_promode")).c_str());
		}

		output->append(va("top 10 of map <%s>\n", map.c_str()));
		output->append("^3------------------------------------^7\n");

		if(vm.count("help")) {
			ostringstream tmpDesc;
			this->description_.print(tmpDesc);
			output->append(tmpDesc.str());
			return true;
		}

		sql::Connection *con = RS->SqlPool()->Get();
		try {
			int i = startAt;
			auto_ptr< sql::PreparedStatement > stmt(con->prepareStatement("SELECT * FROM q3_defrag_records WHERE map = ? AND mode = ? AND physic = ? ORDER BY mstime LIMIT ?, 10"));
			stmt->setString(1, map);
			stmt->setString(2, mode);
			stmt->setString(3, physic);
			stmt->setInt(4, startAt);

			auto_ptr< sql::ResultSet > res(stmt->executeQuery());
			while (res->next()) {
				i++;
				output->append(va(" %-3i). %-20s^7 %i\n", i, res->getString("nickname").c_str(), res->getInt("mstime")));
			}
		} catch (sql::SQLException &e) {
			output->clear();
			output->append(va("REMOTE ERROR: Q3dfApiImpl::ClientCommand::SqlError: '%s'\n", e.what()));
			RS->Con()->PrintError("Q3dfApiImpl::ClientCommand::SqlError: '%s'\n", e.what());
		}

		RS->SqlPool()->Return(con);

		return true;
	}
};

RegisterCommand(TopCommand);