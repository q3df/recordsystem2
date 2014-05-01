#include "SqliteDatabase.h"
#include "Recordsystem.h"

#define CALL_SQLITE(f)                                          \
    {                                                           \
        int i;                                                  \
        i = sqlite3_ ## f;                                      \
        if (i != SQLITE_OK) {                                   \
            gRecordsystem->GetSyscalls()->Print(                \
				va("%s failed with status %d: %s\n",            \
                     #f, i, sqlite3_errmsg (this->db_)));       \
            return NULL;                                        \
        }                                                       \
    }                                                           \

#define CALL_SQLITE_EXPECT(f,x)                                 \
    {                                                           \
        int i;                                                  \
        i = sqlite3_ ## f;                                      \
        if (i != SQLITE_ ## x) {                                \
            gRecordsystem->GetSyscalls()->Print(va(	            \
				"%s failed with status %d: %s\n",               \
                     #f, i, sqlite3_errmsg (this->db_)));       \
            return NULL;                                        \
        }                                                       \
    }                                                           \

SqliteDatabase::SqliteDatabase(const char *dbfile) : initialized_(false) {
	char *zErrMsg = 0;
	int  rc;
	
	rc = sqlite3_open(dbfile, &this->db_);
	if(rc) {
		gRecordsystem->GetSyscalls()->Print(va("Can't open sqlite database: %s\n", sqlite3_errmsg(this->db_)));
	} else {
		gRecordsystem->GetSyscalls()->Print(va("Open sqlite database: %s\n", dbfile));
		initialized_ = true;
	}
}

SqliteDatabase::~SqliteDatabase() {
	sqlite3_close(this->db_);
}

sqlite3_stmt *SqliteDatabase::QueryStmt(const char *sql) {
	sqlite3_stmt *stmt;

	CALL_SQLITE(prepare_v2(this->db_, sql, strlen (sql) + 1, &stmt, NULL));
	return stmt;
}

void *SqliteDatabase::BindInteger(sqlite3_stmt *stmt, int value) {
	CALL_SQLITE (bind_int (stmt, 1, value));
	return NULL;
}

void *SqliteDatabase::BindString(sqlite3_stmt *stmt, const char *value, int size) {
	CALL_SQLITE (bind_text (stmt, 1, value, size+1, SQLITE_STATIC));
	return NULL;
}

void *SqliteDatabase::BindBlob(sqlite3_stmt *stmt, void *data, int size) {
	CALL_SQLITE (bind_blob (stmt, 1, data, size+1, SQLITE_STATIC));
	return NULL;
}

int SqliteDatabase::StmtStep(sqlite3_stmt *stmt) {
	CALL_SQLITE_EXPECT(step (stmt), DONE);
	return SQLITE_DONE;
}

int SqliteDatabase::LastInsertId() {
	return (int)sqlite3_last_insert_rowid(this->db_);
}

