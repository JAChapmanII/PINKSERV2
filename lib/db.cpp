#include "db.hpp"
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

db::Database::Database(string fileName) : _fileName(fileName) {
	int rc = sqlite3_open(_fileName.c_str(), &_db);
	if(rc != SQLITE_OK) {
		cerr << "Database::Database: error opening db "
			<< "\"" << _fileName << "\": "
			<< "error: " << sqlite3_errmsg(_db) << endl;
		throw rc;
	}
}
db::Database::~Database() {
	if(_db) {
		int rc = sqlite3_close(_db);
		if(rc != SQLITE_OK) {
			cerr << "Database::~Database: error closing db "
				<< "\"" << _fileName << "\": "
				<< "error: " << sqlite3_errmsg(_db) << endl;
			throw rc;
		}
	}
}

sqlite3 *db::Database::getDB() { return _db; }

db::Statement::Statement(Database &db, string sql) : _db(db), _sql(sql) {
	cerr << "Statement::Statement: sql: \"" << _sql << "\"" << endl;
	const char *leftover{nullptr};
	int rc = sqlite3_prepare_v2(_db.getDB(), _sql.c_str(), _sql.size(),
			&_statement, &leftover);
	if(rc != SQLITE_OK) {
		cerr << "Statement::Statement: error preparing statement: "
			<< "\"" << _sql << "\"" << endl
			<< "    " << rc << endl;
		throw rc;
	}
}
db::Statement::~Statement() {
	if(_statement) {
		int rc = sqlite3_finalize(_statement);
		if(rc != SQLITE_OK) {
			cerr << "Statement::~Statement: error finalizing statement: "
				<< rc << endl;
			throw rc;
		}
	}
}

void db::Statement::bind(int idx, int val) {
	sqlite3_bind_int(_statement, idx, val);
}
void db::Statement::bind(int idx, string val) {
	sqlite3_bind_text(_statement, idx, val.c_str(), val.size(), SQLITE_STATIC);
}

int db::Statement::step() {
	return sqlite3_step(_statement);
}

int db::Statement::getInteger(int idx) {
	return sqlite3_column_int(_statement, idx);
}
string db::Statement::getString(int idx) {
	const unsigned char *result = sqlite3_column_text(_statement, idx);
	return string{(char *)result};
}

