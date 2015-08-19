#include "kvpair.hpp"
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

KVPairDB::KVPairDB(string idb, string itable)
		: _dbFilename(idb), _table(itable), _db{nullptr} {
	int rc = sqlite3_open(_dbFilename.c_str(), &_db);
	if(rc) {
		cerr << "KVPairDB::KVPairDB: unable to open db \"" << _dbFilename << "\""
			<< ": " << sqlite3_errmsg(_db) << endl;
		throw -1;
	}
}
KVPairDB::~KVPairDB() {
	int rc = sqlite3_close(_db);
}

bool KVPairDB::exists(string key) {
	string sql = "SELECT COUNT(1) FROM " + _table + " WHERE key = ?";
	sqlite3_stmt *statement{nullptr};
	const char *leftover{nullptr};
	int rc = sqlite3_prepare_v2(_db, sql.c_str(), sql.size(),
			&statement, &leftover);
	if(rc) {
		cerr << "KVPairDB::exists: error: " << rc << endl;
		throw -1;
	}
	sqlite3_bind_text(statement, 1, key.c_str(), key.size(), SQLITE_STATIC);
	rc = sqlite3_step(statement);
	if(rc != SQLITE_ROW) { throw -4; }

	int result = sqlite3_column_int(statement, 0);

	rc = sqlite3_finalize(statement);
	if(rc != 0) { throw -3; }

	return result > 0;
}
string *KVPairDB::getValue(string key) {
	string sql = "SELECT value FROM " + _table + " WHERE key = ?;";
	sqlite3_stmt *statement{nullptr};
	const char *leftover{nullptr};
	int rc = sqlite3_prepare_v2(_db, sql.c_str(), sql.size(),
			&statement, &leftover);
	if(rc) {
		cerr << "KVPairDB::getValue: error: " << rc << endl;
		throw -1;
	}
	sqlite3_bind_text(statement, 1, key.c_str(), key.size(), SQLITE_STATIC);
	rc = sqlite3_step(statement);
	if(rc == SQLITE_DONE) {
		rc = sqlite3_finalize(statement);
		if(rc != 0) { throw -3; }
		return nullptr;
	}
	if(rc != SQLITE_ROW) {
		throw -2;
	}

	const unsigned char *result = sqlite3_column_text(statement, 0);
	if(result == nullptr) {
		rc = sqlite3_finalize(statement);
		if(rc != 0) { throw -3; }
		return nullptr;
	}
	string value{(char *)result};

	rc = sqlite3_finalize(statement);
	if(rc != 0) { throw -3; }

	return new string(value);
}
void KVPairDB::insertPair(KVPair pair) {
	string sql = "INSERT INTO " + _table + " VALUES(?, ?)";
	sqlite3_stmt *statement{nullptr};
	const char *leftover{nullptr};
	int rc = sqlite3_prepare_v2(_db, sql.c_str(), sql.size(),
			&statement, &leftover);
	if(rc) {
		cerr << "KVPairDB::insertPair: error: " << rc << endl;
		throw -1;
	}

	sqlite3_bind_text(statement, 1, pair.key.c_str(), pair.key.size(), SQLITE_STATIC);
	sqlite3_bind_text(statement, 2, pair.value.c_str(), pair.value.size(), SQLITE_STATIC);

	rc = sqlite3_step(statement);
	if(rc != SQLITE_DONE) { throw -2; }

	rc = sqlite3_finalize(statement);
	if(rc != 0) { throw -3; }
}
void KVPairDB::setPair(KVPair pair) {
	string sql = "UPDATE " + _table + " SET value = ? WHERE key = ?";
	sqlite3_stmt *statement{nullptr};
	const char *leftover{nullptr};
	int rc = sqlite3_prepare_v2(_db, sql.c_str(), sql.size(),
			&statement, &leftover);
	if(rc) {
		cerr << "KVPairDB::setPair: error: " << rc << endl;
		throw -1;
	}

	sqlite3_bind_text(statement, 1, pair.value.c_str(), pair.value.size(), SQLITE_STATIC);
	sqlite3_bind_text(statement, 2, pair.key.c_str(), pair.key.size(), SQLITE_STATIC);

	rc = sqlite3_step(statement);
	if(rc != SQLITE_DONE) { throw -2; }

	rc = sqlite3_finalize(statement);
	if(rc != 0) { throw -3; }
}

