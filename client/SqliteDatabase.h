#ifndef SQLITE_DATABASE__
#define SQLITE_DATABASE__
#include <sqlite3.h>

class SqliteDatabase {
private:
	int stmt_counter_;
	bool initialized_;
	sqlite3 *db_;

public:
	SqliteDatabase(const char *dbfile);
	~SqliteDatabase();
	sqlite3_stmt *QueryStmt(const char *sql);
	void *BindInteger(sqlite3_stmt *stmt, int value);
	void *BindString(sqlite3_stmt *stmt, const char *value, int size);
	void *BindBlob(sqlite3_stmt *stmt, void *data, int size);
	int StmtStep(sqlite3_stmt *stmt);
	int LastInsertId();
	
};


#endif // SQLITE_DATABASE__