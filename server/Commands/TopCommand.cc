#include "../RecordsystemDaemon.h"
#include "../Commands.h"
#include <time.h>

#define TOP_QUERY "SELECT \
		a.mstime, \
		b.visname,\
		b.strip_visname,\
		((SELECT count(id) FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND mstime<a.mstime AND status=0 ORDER by mstime)+1) as rank,\
		(SELECT mstime FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND status=0 ORDER by mstime ASC LIMIT 1) as rec_one,\
		defrag_version,\
		a.timestamp,\
		b.country\
	FROM q3_defrag_records a\
	LEFT JOIN q3_users b ON a.user_id=b.id\
	WHERE\
		a.map=?\
		AND a.physic=?\
		AND a.mode=?\
		AND a.status=0\
	ORDER BY a.mstime ASC, a.timestamp ASC\
	LIMIT ?,10"

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


	virtual bool IsAlsoProxymodCommand() {
		return true;
	}

	virtual bool Execute(const vector<string>* args, Conn *contextCon, string *output) {
		int startAt = 0;
		char *diffDftime;
		char *dftime;

		sql::SQLString map;
		sql::SQLString mode;
		sql::SQLString physic;
		po::variables_map options;
		vector<string> optionalOptions;
		vector<string>::iterator optionalOptionsIt;


		this->ParseOptions(args, &options, &optionalOptions);
		startAt = options["startat"].as<int>();

		if(!contextCon) {
			map = sql::SQLString("cpmjump");
			mode = sql::SQLString("-1");
			physic = sql::SQLString("1");
		}else {
			map = sql::SQLString(RS->Clients()->GetClient(contextCon)->GetServerInfo(string("mapname")).c_str());
			mode = sql::SQLString(RS->Clients()->GetClient(contextCon)->GetServerInfo(string("defrag_mode")).c_str());
			physic = sql::SQLString(RS->Clients()->GetClient(contextCon)->GetServerInfo(string("df_promode")).c_str());
		}

		for(optionalOptionsIt = optionalOptions.begin(); optionalOptionsIt != optionalOptions.end(); ++optionalOptionsIt) {
			auto item = (*optionalOptionsIt);
			if(item == "cpm") {
				physic = sql::SQLString("1");
				mode = sql::SQLString("-1");
			} else if(item == "vq3") {
				physic = sql::SQLString("0");
				mode = sql::SQLString("-1");
			} else if(item == "vq3ctf1" || item == "vq3ctf2" || item == "vq3ctf3" || item == "vq3ctf4" || item == "vq3ctf5" || item == "vq3ctf6" || item == "vq3ctf7") {
				physic = sql::SQLString("0");
				mode = item.substr(6, 1);
			} else if (item == "cpmctf1" || item == "cpmctf2" || item == "cpmctf3" || item == "cpmctf4" || item == "cpmctf5" || item == "cpmctf6" || item == "cpmctf7") {
				physic = sql::SQLString("1");
				mode = item.substr(6, 1);
			} else {
				map = sql::SQLString(item);
			}
		}		

		output->append(va(
			"^3  Rankings on %s (^1%s%s^3)^7\n",
			map.c_str(),
			(physic == "1" ? "cpm" : "vq3"),
			(mode == "-1" ? "" : ("ctf-" + mode).c_str())
		));
		output->append("^5-----------------------------------------------------------------------------^7\n");

		if(options.count("help")) {
			ostringstream tmpDesc;
			//this->description_.print(tmpDesc);
			output->append(tmpDesc.str());
			return true;
		}

		sql::Connection *con = RS->SqlPool()->Get();
		try {
			int i = startAt;
			int col_nick = 22;

			auto_ptr< sql::PreparedStatement > stmt(con->prepareStatement(TOP_QUERY));
			stmt->setString(1, map);
			stmt->setString(2, physic);
			stmt->setString(3, mode);
			stmt->setInt(4, startAt);

			auto_ptr< sql::ResultSet > res(stmt->executeQuery());
			while (res->next()) {
				col_nick = 22;
				diffDftime = this->MstimeToDftime(res->getInt("mstime")-res->getInt("rec_one"), 2);
				dftime = this->MstimeToDftime(res->getInt("mstime"), 0);

				if (res->getString("strip_visname").length() < res->getString("visname").length())
					col_nick += (res->getString("visname").length() - res->getString("strip_visname").length());

				struct tm *ptr;
#if defined(WIN32) || defined(__APPLE__)
				time_t epoch;
#else
				int epoch;
#endif
				char timestr[100];
				memset(static_cast<void *>(&timestr), '\0', sizeof(timestr));
				epoch = (time_t)res->getInt64("timestamp");
#if defined(WIN32) || defined(__APPLE__)
				ptr = gmtime((time_t *)(&epoch));
#else
				ptr = gmtime((void *)(&epoch));
#endif
				strftime(timestr, 100, "%Y-%m-%d %H:%M", ptr);

				string rec_color;
				if(res->getInt("defrag_version") < 19112)
					rec_color.append("^7");
				else
					rec_color.append("^3");

				if(res->getInt("mstime") == res->getInt("rec_one")) {
					output->append(va(
						"^3 %3i. ^7%s%-*s^3%s %9s ^3%32s ^7\n",
						res->getInt("rank"),
						res->getString("country").c_str(),
						col_nick,
						res->getString("visname").c_str(),
						rec_color.c_str(),
						dftime,
						timestr
					));
				}else {
					output->append(va(
						"^3 %3i. ^7%s%-*s^3%s %9s^7  ^1%10s ^3%20s^7\n",
						res->getInt("rank"),
						res->getString("country").c_str(),
						col_nick,
						res->getString("visname").c_str(),
						rec_color.c_str(),
						dftime,
						diffDftime,
						timestr
					));
				}

				//free(diffDftime);
				//free(dftime);
				i++;
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
