#include "modules.h"

extern "C" {
char RS_MYSQL_CONNECTION_ERROR[] = "print \"^1-> ^2can't connect to database.^7\n^1-> ^2Contact an admin, please!^7\n\"";
char RS_CMD_TOP_LINE1[] = "print \"\n^3  Rankings on %s (^1%s^3)^7\n\"";
char RS_CMD_TOP_LINE2[] = "print \"^5-----------------------------------------------------^7\n\"";
char RS_CMD_TOP_LINE2_DETAIL[] = "print \"^5-----------------------------------------------------------------------------^7\n\"";
char RS_CMD_TOP_FIRST_PLACE[] = "print \"^3 %3i. ^7%s%-*s^3%s %9s ^3%32s ^7\n\"";
char RS_CMD_TOP_PLACE[] = "print \"^3 %3i. ^7%s%-*s^3%s %9s^7  ^1%10s ^3%20s^7\n\"";

//char RS_CMD_TOP_COUNTRY_FIRST_PLACE[] = "print \"^3 %3d.  ^7%s ^3%i ^7\n\"";
//char RS_CMD_TOP_COUNTRY_PLACE[] = 		"print \"^3 %3d.  ^7%s ^3%i ^1%i ^7\n\"";
char RS_CMD_TOP_COUNTRY_FIRST_PLACE[] = "print \"^3 %3d.  ^7%s ^3%9d ^7\n\"";
char RS_CMD_TOP_COUNTRY_PLACE[] = 		"print \"^3 %3d.  ^7%s ^3%9d ^1%9s ^7\n\"";

char RS_CMD_NOENTRY[] = "chat \"^7%s^2, there are ^1NO^2 entries in the database.^7\"";

Function *global = NULL;

#define MAX_LIMIT_LENGTH 10
#define MAX_MAP_LENGTH 80
#define MAX_COL_NICK_LENGTH 22

void rs_cmd_top(rs_client * cl)
{
	// default values for command !top
	char *s_map;
	int s_physic;
	int s_mode;
	s_physic = GetCurrentPhysic();
	s_mode = GetCurrentMode();
	s_map = GetCurrentMap();

	// Defrag time Output strings
	char *dftime;
	char *dif_dftime;

	char map[MAX_MAP_LENGTH];
	char escaped_map[MAX_MAP_LENGTH * 2 + 1];
	char physic_string[10];
	char tmp[2];
	tmp[1] = '\0';

	snprintf(map, sizeof(map), "%s", s_map);	// default map

	MYSQL sqlcon;
	MYSQL_ROW fetch;
	MYSQL_RES *results;

	int physic = s_physic;	// default phyisc
	int mode = s_mode;	// default mode

	//int lastmstime;
	int stripNick_len = 0;
	int i = 0;
	//int i_fix = 1;
	int rec_one = 0;
	int col_nick = MAX_COL_NICK_LENGTH;

	int arg_show = 0;
	int arg_more = 0;
	int startat = 0;
	int arg_loggedin = 0;
	int arg_detail = 0;
	int arg_combined = 0;
	
	int arg_country = 0;
	int dfrank = 0;
	int dif_dfrank = 0;
	char dif_dfrank_string[10];

	int begin = 0;

	i++;
	// parse argument list ...
	while (cl->cmd_argv[i] != NULL) {

		if (!strcmp(cl->cmd_argv[i], "vq3")) {
			physic = 0;
			mode = -1;

		} else if (!strcmp(cl->cmd_argv[i], "cpm")) {
			physic = 1;
			mode = -1;

		} else if (!strncmp(cl->cmd_argv[i], "cpm.", 4)) {
			if ((cl->cmd_argv[i][4] >= '1') && (cl->cmd_argv[i][4] <= '7')) {
				tmp[0] = cl->cmd_argv[i][4];
				mode = atoi(tmp);
				physic = 1;
			}
		} else if (!strncmp(cl->cmd_argv[i], "vq3.", 4)) {
			if ((cl->cmd_argv[i][4] >= '1') && (cl->cmd_argv[i][4] <= '7')) {
				tmp[0] = cl->cmd_argv[i][4];
				mode = atoi(tmp);
				physic = 0;
			}
		} else if (!strncmp(cl->cmd_argv[i], "mode", 4)) {
			if ((cl->cmd_argv[i][4] >= '1') && (cl->cmd_argv[i][4] <= '7')) {
				tmp[0] = cl->cmd_argv[i][4];
				mode = atoi(tmp);
			}
		} else if (!strncmp(cl->cmd_argv[i], "mode.", 5)) {
			if ((cl->cmd_argv[i][5] >= '1') && (cl->cmd_argv[i][5] <= '7')) {
				tmp[0] = cl->cmd_argv[i][5];
				mode = atoi(tmp);
			}					
		} else if (!strcmp(cl->cmd_argv[i], "show")) {
			arg_show = 1;

		} else if (!strcmp(cl->cmd_argv[i], "more")) {
			arg_more++;
			startat = arg_more * MAX_LIMIT_LENGTH;
		} else if (!strncmp(cl->cmd_argv[i], "startat", 7)) {
			begin = 7;
			char substr[7];
			memset((void *)&substr, '\0', begin);
			int end = (strlen(cl->cmd_argv[i]) - begin);
			if (end <= 0)
				end = strlen(cl->cmd_argv[i]);
			strncpy(substr, cl->cmd_argv[i] + begin, end);
			if ((atoi(substr) - 1 > 0) && (atoi(substr) < 3000)) {
				startat = atoi(substr) - 1;
			}
		} else if (!strcmp(cl->cmd_argv[i], "date") || !strncmp(cl->cmd_argv[i], "detail", 6)) {
			arg_detail = 1;			
		} else if (!strcmp(cl->cmd_argv[i], "combined")) {
			arg_combined = 1;		
		} else if (!strcmp(cl->cmd_argv[i], "loggedin")) {
			arg_loggedin = 1;
		} else if (!strncmp(cl->cmd_argv[i], "countr", 6) ) {  
			// country or countries
			arg_country = 1;
		} else {
			snprintf(map, sizeof(map), "%s", cl->cmd_argv[i]);
		}
		i++;
	}

	// physic2string...
	if (physic == 1)
		snprintf(physic_string, sizeof(physic_string), "cpm");
	else
		snprintf(physic_string, sizeof(physic_string), "vq3");

	if (mode > 0) {
		if (physic == 1)
			snprintf(physic_string, sizeof(physic_string), "cpm.%i", mode);
		else
			snprintf(physic_string, sizeof(physic_string), "vq3.%i", mode);
	}

	print_user_cmd_to_chat(cl, arg_show);

	// sql connection and get all the output...
	if (sql_connect(&sqlcon)) {
		mysql_real_escape_string(&sqlcon, escaped_map, map, strlen(map));	//before query escape the map... \'
		if (arg_country) {
			// top by country
/* 			results = sql_get_query(&sqlcon, "SELECT country, sum(rank) as TotalScore FROM ( \ */
/* 			SELECT t1.country, t1.rank FROM  \ */
/* 			(SELECT b.strip_visname, country, ((SELECT count(id) FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND mstime<a.mstime AND status=0 ORDER by mstime)+1) as rank \ */
/* 			FROM q3_defrag_records a LEFT JOIN q3_users b ON a.user_id=b.id \ */
/* 			WHERE a.map='%s' AND a.physic='%i' AND a.mode='%i' AND a.status=0 ORDER BY a.mstime ASC, a.timestamp ASC ) t1 \ */
/* 			INNER JOIN \ */
/* 			(SELECT b.strip_visname, country, ((SELECT count(id) FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND mstime<a.mstime AND status=0 ORDER by mstime)+1) as rank \ */
/* 			FROM q3_defrag_records a LEFT JOIN q3_users b ON a.user_id=b.id \ */
/* 			WHERE a.map='%s' AND a.physic='%i' AND a.mode='%i' AND a.status=0 ORDER BY a.mstime ASC, a.timestamp ASC ) t2 \ */
/* 			ON (t1.country=t2.country) \ */
/* 			WHERE (t1.rank >= t2.rank) OR (t1.rank = t2.rank) \ */
/* 			GROUP BY t1.country, t1.rank \ */
/* 			HAVING COUNT(t1.country) <=3 ) t \ */
/* 			GROUP BY country HAVING count(*)=3 ORDER BY TotalScore \ */
/* 			LIMIT %i,%i", escaped_map, physic, mode, escaped_map, physic, mode, startat, MAX_LIMIT_LENGTH);	 */		
			results = sql_get_query(&sqlcon, "SELECT country, sum(rank) as ts FROM ( SELECT t1.country, t1.rank FROM  (SELECT country, ((SELECT count(id) FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND mstime<a.mstime AND status=0 ORDER by mstime)+1) as rank FROM q3_defrag_records a LEFT JOIN q3_users b ON a.user_id=b.id WHERE a.map='%s' AND a.physic='%i' AND a.mode='%i' AND a.status=0 ORDER BY a.mstime ASC, a.timestamp ASC ) t1 \
INNER JOIN (SELECT country, ((SELECT count(id) FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND mstime<a.mstime AND status=0 ORDER by mstime)+1) as rank \
FROM q3_defrag_records a LEFT JOIN q3_users b ON a.user_id=b.id \
WHERE a.map='%s' AND a.physic='%i' AND a.mode='%i' AND a.status=0 ORDER BY a.mstime ASC, a.timestamp ASC ) t2 \
ON (t1.country=t2.country) WHERE (t1.rank >= t2.rank) OR (t1.rank = t2.rank) GROUP BY t1.country, t1.rank HAVING COUNT(t1.country) <=3 ) t GROUP BY country HAVING count(*)=3 ORDER BY ts \
LIMIT %i,%i", escaped_map, physic, mode, escaped_map, physic, mode, startat, MAX_LIMIT_LENGTH);	// over a certain length this sql query will fail..
/* SELECT country, sum(rank) as TotalScore FROM ( */
/* SELECT t1.country, t1.rank */
/* FROM  */
/* (SELECT b.strip_visname, country, ((SELECT count(id) FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND mstime<a.mstime AND status=0 ORDER by mstime)+1) as rank  */
/* FROM q3_defrag_records a LEFT JOIN q3_users b ON a.user_id=b.id  */
/* WHERE a.map='lovet-second' AND a.physic='1' AND a.mode='-1' AND a.status=0 ORDER BY a.mstime ASC, a.timestamp ASC  */
/* ) t1 */
/* INNER JOIN  */
/* (SELECT b.strip_visname, country, ((SELECT count(id) FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND mstime<a.mstime AND status=0 ORDER by mstime)+1) as rank  */
/* FROM q3_defrag_records a LEFT JOIN q3_users b ON a.user_id=b.id  */
/* WHERE a.map='lovet-second' AND a.physic='1' AND a.mode='-1' AND a.status=0 ORDER BY a.mstime ASC, a.timestamp ASC  */
/* ) t2 ON (t1.country=t2.country) */
/* WHERE (t1.rank >= t2.rank) OR (t1.rank = t2.rank) */
/* GROUP BY t1.country, t1.rank */
/* HAVING COUNT(t1.country) <=3 */
/* ) t GROUP BY country HAVING count(*)=3 ORDER BY TotalScore; */
			SV_SendServerCommand((arg_show ? NULL : cl), RS_CMD_TOP_LINE1, map, physic_string);
			SV_SendServerCommand((arg_show ? NULL : cl), RS_CMD_TOP_LINE2);
			if (results) {
				// loop from database
				i = 1;
				while ((fetch = mysql_fetch_row(results))) {
					if (fetch) {
						if (i == 1)
							rec_one = atoi(fetch[1]);	//rec_one

						dfrank = atoi(fetch[1]);
						dif_dfrank = dfrank - rec_one;
						
						snprintf(dif_dfrank_string, sizeof(dif_dfrank_string), "+%i", dif_dfrank);
						
						// put country in uppercase
						const char null = 0;
						int index;
						for (index = 0; fetch[0][index] != null; index++) {
							fetch[0][index] = toupper(fetch[0][index]);
						}

						if (dfrank == rec_one) {
							// first place
							SV_SendServerCommand((arg_show ? NULL : cl), RS_CMD_TOP_COUNTRY_FIRST_PLACE, i + startat, fetch[0], dfrank);
						} else {
							SV_SendServerCommand((arg_show ? NULL : cl), RS_CMD_TOP_COUNTRY_PLACE, i + startat, fetch[0], dfrank, dif_dfrank_string);
						}
						i++;
					}
				}
				if ((int)mysql_num_rows(results) == 0) {
					SV_SendServerCommand((arg_show ? NULL : cl), "print \"^1-> ^7NO Results \n\"");
				}

				SV_SendServerCommand((arg_show ? NULL : cl), "print \"\n\"");
				sql_free_result(results);
			} else 
				SV_SendServerCommand((arg_show ? NULL : cl), "print \"^1-> ^7NO Results \n\n\"");

		} else if (arg_combined) {
			// times of both physics 
			// TODO: sql rank is busted
//			((SELECT count(user_id) FROM q3_defrag_records WHERE map=a.map AND mode=a.mode AND status=a.status group by user_id HAVING count(user_id=2) AND (sum(mstime)<sum(a.mstime)) and count(user_id)=2 ORDER by sum(mstime) LIMIT 1)+1)			
//SELECT count(b.id), round(sum(a.mstime)/count(b.id)), b.visname, map, b.strip_visname, 1 as rank, (SELECT sum(mstime)/count(user_id) FROM q3_defrag_records WHERE map=a.map AND status=0 GROUP BY user_id HAVING count(user_id)>=2 ORDER by sum(mstime)/count(user_id) ASC LIMIT 1) as rec_one, defrag_version, a.timestamp, b.country FROM q3_defrag_records a LEFT JOIN q3_users b ON a.user_id=b.id WHERE a.map='bardok-avp' AND a.status=0 group by b.id HAVING count(b.id)>=2 ORDER BY sum(a.mstime)/count(b.id) ASC, a.timestamp ASC LIMIT 10;
			results = sql_get_query(&sqlcon, "SELECT sum(a.mstime) as total_time, b.visname, map, b.strip_visname, \
			1 as rank, \
			(SELECT sum(mstime) FROM q3_defrag_records WHERE map=a.map AND mode=a.mode AND status=0 GROUP BY user_id HAVING count(user_id)=2 ORDER by sum(mstime) ASC LIMIT 1) as rec_one, \
			defrag_version, a.timestamp, b.country \
			FROM q3_defrag_records a LEFT JOIN q3_users b ON a.user_id=b.id WHERE a.map='%s' AND a.mode='%i' AND a.status=0 \
			GROUP BY a.user_id HAVING count(b.id)=2 \
			ORDER BY sum(a.mstime) ASC, a.timestamp ASC LIMIT %i,%i", escaped_map, mode, startat, MAX_LIMIT_LENGTH);	


			if (mode > 0) 
				snprintf(physic_string, sizeof(physic_string), "vq3+cpm.%i", mode);
			else 
				snprintf(physic_string, sizeof(physic_string), "vq3+cpm");

			SV_SendServerCommand((arg_show ? NULL : cl), RS_CMD_TOP_LINE1, map, physic_string);
			SV_SendServerCommand((arg_show ? NULL : cl), (arg_detail ? RS_CMD_TOP_LINE2_DETAIL : RS_CMD_TOP_LINE2));
			if (results) {
				// loop from database
				i = 1;
				while ((fetch = mysql_fetch_row(results))) {

					if (fetch) {
						if (i == 1)
							rec_one = atoi(fetch[5]);	//rec_one

						dif_dftime = mstime_to_dftime(atoi(fetch[0]) - rec_one, 2);
						dftime = mstime_to_dftime(atoi(fetch[0]), 0);

						// strip " from visname, replace with '
						const char null = 0;
						int index;
						for (index = 0; fetch[1][index] != null; index++) {
							if (fetch[1][index] == '"')
								fetch[1][index] = '\'';
						}

						// fix the output from color nicknames "spaces" printf("%XXs");
						stripNick_len = strlen(fetch[3]);
						if (stripNick_len < strlen(fetch[1]))
							col_nick += (strlen(fetch[1]) - stripNick_len);

						char rec_color[3];
						if (atoi(fetch[6]) < 19112) {
							snprintf(rec_color, sizeof(rec_color), "^7");
						} else {
							snprintf(rec_color, sizeof(rec_color), "^3");
						}


						struct tm *ptr;
						time_t epoch;
						char timestr[100];
						memset((void *)&timestr, '\0', sizeof(timestr));
						char countrystr[10];
						memset((void *)&countrystr, '\0', sizeof(countrystr));
					
						if (arg_detail) {
							if (fetch[7]) {
								memset((void *)&timestr, '\0', sizeof(timestr));
								epoch = atoi(fetch[7]);
								ptr = gmtime(&epoch);
								strftime(timestr, 100, "%Y-%m-%d %H:%M", ptr);
		//						strftime(timestr, 100, "%Y-%m-%d", ptr);						
							}
							if (fetch[8]) {
								snprintf(countrystr, sizeof(countrystr), "%s  ", fetch[8]);
								// put country in uppercase
								const char null = 0;
								int index;
								for (index = 0; countrystr[index] != null; index++) {
									countrystr[index] = toupper(countrystr[index]);
								}								
							}
						}
						if (atoi(fetch[0]) == rec_one) {
							// first place
//							SV_SendServerCommand((arg_show ? NULL : cl), RS_CMD_TOP_FIRST_PLACE, atoi(fetch[4]), countrystr, col_nick, fetch[1], rec_color, dftime, timestr);
							SV_SendServerCommand((arg_show ? NULL : cl), RS_CMD_TOP_FIRST_PLACE, i+startat, countrystr, col_nick, fetch[1], rec_color, dftime, timestr);
						} else {
//							SV_SendServerCommand((arg_show ? NULL : cl), RS_CMD_TOP_PLACE, atoi(fetch[4]), countrystr, col_nick, fetch[1], rec_color, dftime, dif_dftime, timestr);
							SV_SendServerCommand((arg_show ? NULL : cl), RS_CMD_TOP_PLACE, i+startat, countrystr, col_nick, fetch[1], rec_color, dftime, dif_dftime, timestr);
						}

						free(dftime);
						free(dif_dftime);
						i++;

						col_nick = MAX_COL_NICK_LENGTH;
					}
				}
				if ((int)mysql_num_rows(results) == 0) {
					SV_SendServerCommand((arg_show ? NULL : cl), "print \"^1-> ^7NO Results \n\"");
				}

				SV_SendServerCommand((arg_show ? NULL : cl), "print \"\n\"");
				sql_free_result(results);
			} else 
				SV_SendServerCommand((arg_show ? NULL : cl), "print \"^1-> ^7NO Results \n\n\"");

		} else { // top by player
			if (arg_loggedin) {
				int server_id = GetServerId();
				results = sql_get_query(&sqlcon, "SELECT a.mstime, b.visname, map, b.strip_visname, \
		        ((SELECT count(id) FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND mstime<a.mstime AND status=0 ORDER by mstime)+1) as rank, \
				(SELECT mstime FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND status=0 ORDER by mstime ASC LIMIT 1) as rec_one, \
				defrag_version, a.timestamp, b.country \
				FROM q3_servers_status_players c LEFT JOIN q3_defrag_records a ON a.user_id=c.user_id LEFT JOIN q3_users b ON a.user_id=b.id \
				WHERE c.server_id='%i' AND a.map='%s' AND a.physic='%i' AND a.mode='%i' ORDER BY a.mstime ASC, a.timestamp ASC LIMIT %i,%i", server_id, escaped_map, physic, mode, startat, MAX_LIMIT_LENGTH);
			} else {
				results = sql_get_query(&sqlcon, "SELECT a.mstime, b.visname, map, b.strip_visname, \
		        ((SELECT count(id) FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND mstime<a.mstime AND status=0 ORDER by mstime)+1) as rank, \
				(SELECT mstime FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND status=0 ORDER by mstime ASC LIMIT 1) as rec_one, \
				defrag_version, a.timestamp, b.country \
				FROM q3_defrag_records a LEFT JOIN q3_users b ON a.user_id=b.id \
				WHERE a.map='%s' AND a.physic='%i' AND a.mode='%i' AND a.status=0 ORDER BY a.mstime ASC, a.timestamp ASC LIMIT %i,%i", escaped_map, physic, mode, startat, MAX_LIMIT_LENGTH);
			}

			if ((startat > 0) && ((int)mysql_num_rows(results) == 0)) {
				/*we are overpaged, so show first page */
				startat = 0;
				sql_free_result(results);
				if (arg_loggedin) {
					int server_id = GetServerId();
					results = sql_get_query(&sqlcon, "SELECT a.mstime, b.visname, map, b.strip_visname, \
		            ((SELECT count(id) FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND mstime<a.mstime AND status=0 ORDER by mstime)+1) as rank, \
				    (SELECT mstime FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND status=0 ORDER by mstime ASC LIMIT 1) as rec_one, \
				    defrag_version, a.timestamp, b.country \
				    FROM q3_servers_status_players c LEFT JOIN q3_defrag_records a ON a.user_id=c.user_id LEFT JOIN q3_users b ON a.user_id=b.id \
				    WHERE c.server_id='%i' AND a.map='%s' AND a.physic='%i' AND a.mode='%i' ORDER BY a.mstime ASC, a.timestamp ASC LIMIT %i,%i", server_id, escaped_map, physic, mode, startat, MAX_LIMIT_LENGTH);
				} else {
					results = sql_get_query(&sqlcon, "SELECT a.mstime, b.visname, map, b.strip_visname, \
		            ((SELECT count(id) FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND mstime<a.mstime AND status=0 ORDER by mstime)+1) as rank, \
				    (SELECT mstime FROM q3_defrag_records WHERE map=a.map AND physic=a.physic AND mode=a.mode AND status=0 ORDER by mstime ASC LIMIT 1) as rec_one, \
		    		defrag_version, a.timestamp, b.country \
				    FROM q3_defrag_records a LEFT JOIN q3_users b ON a.user_id=b.id \
				    WHERE a.map='%s' AND a.physic='%i' AND a.mode='%i' AND a.status=0 ORDER BY a.mstime ASC, a.timestamp ASC LIMIT %i,%i", escaped_map, physic, mode, startat, MAX_LIMIT_LENGTH);
				}
			}

			SV_SendServerCommand((arg_show ? NULL : cl), RS_CMD_TOP_LINE1, map, physic_string);
	//		SV_SendServerCommand((arg_show ? NULL : cl), RS_CMD_TOP_LINE2);
			SV_SendServerCommand((arg_show ? NULL : cl), (arg_detail ? RS_CMD_TOP_LINE2_DETAIL : RS_CMD_TOP_LINE2));
	
			if (results) {
				// loop from database
				i = 1;
				while ((fetch = mysql_fetch_row(results))) {

					if (fetch) {
						if (i == 1)
							rec_one = atoi(fetch[5]);	//rec_one

						dif_dftime = mstime_to_dftime(atoi(fetch[0]) - rec_one, 2);
						dftime = mstime_to_dftime(atoi(fetch[0]), 0);

						// strip " from visname, replace with '
						const char null = 0;
						int index;
						for (index = 0; fetch[1][index] != null; index++) {
							if (fetch[1][index] == '"')
								fetch[1][index] = '\'';
						}

						// fix the output from color nicknames "spaces" printf("%XXs");
						stripNick_len = strlen(fetch[3]);
						if (stripNick_len < strlen(fetch[1]))
							col_nick += (strlen(fetch[1]) - stripNick_len);

						char rec_color[3];
						if (atoi(fetch[6]) < 19112) {
							snprintf(rec_color, sizeof(rec_color), "^7");
						} else {
							snprintf(rec_color, sizeof(rec_color), "^3");
						}


						struct tm *ptr;
						time_t epoch;
						char timestr[100];
						memset((void *)&timestr, '\0', sizeof(timestr));
						char countrystr[10];
						memset((void *)&countrystr, '\0', sizeof(countrystr));
					
						if (arg_detail) {
							if (fetch[7]) {
								memset((void *)&timestr, '\0', sizeof(timestr));
								epoch = atoi(fetch[7]);
								ptr = gmtime(&epoch);
								strftime(timestr, 100, "%Y-%m-%d %H:%M", ptr);
								//strftime(timestr, 100, "%Y-%m-%d", ptr);						
							}
							if (fetch[8]) {
								snprintf(countrystr, sizeof(countrystr), "%s  ", fetch[8]);
								// put country in uppercase
								const char null = 0;
								int index;
								for (index = 0; countrystr[index] != null; index++) {
									countrystr[index] = toupper(countrystr[index]);
								}								
							}
						}
						if (atoi(fetch[0]) == rec_one) {
							// first place
							SV_SendServerCommand((arg_show ? NULL : cl), RS_CMD_TOP_FIRST_PLACE, atoi(fetch[4]), countrystr, col_nick, fetch[1], rec_color, dftime, timestr);
						} else {
							SV_SendServerCommand((arg_show ? NULL : cl), RS_CMD_TOP_PLACE, atoi(fetch[4]), countrystr, col_nick, fetch[1], rec_color, dftime, dif_dftime, timestr);
						}

						free(dftime);
						//free(dif_dftime);
						i++;

						col_nick = MAX_COL_NICK_LENGTH;
					}
				}
				if ((int)mysql_num_rows(results) == 0) {
					SV_SendServerCommand((arg_show ? NULL : cl), "print \"^1-> ^7NO Results \n\"");
				}

				SV_SendServerCommand((arg_show ? NULL : cl), "print \"\n\"");
				sql_free_result(results);
			} else 
				SV_SendServerCommand((arg_show ? NULL : cl), "print \"^1-> ^7NO Results \n\n\"");
		} // player 
	} else
		SV_SendServerCommand(cl, RS_MYSQL_CONNECTION_ERROR);

	sql_close(&sqlcon);
	FreeParameterArray(cl->cmd_argv);
	cl->cmd_progress = 0;
	return;
}
}