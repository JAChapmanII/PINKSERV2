#include "kvpair.hpp"
using std::string;
using db::Database;
using db::Statement;

#include <iostream>
using std::cerr;
using std::endl;

KVPairDB::KVPairDB(string idb, string itable) : _table(itable), _db{idb} { }
KVPairDB::~KVPairDB() { }

bool KVPairDB::exists(string key) {
	string sql = "SELECT COUNT(1) FROM " + _table + " WHERE key = ?";
	Statement statement{_db, sql};

	statement.bind(1, key);

	int rc = statement.step();
	if(rc != SQLITE_ROW) { throw rc; }

	return statement.getInteger(0) > 0;
}
string *KVPairDB::getValue(string key) {
	string sql = "SELECT value FROM " + _table + " WHERE key = ?;";
	Statement statement{_db, sql};

	statement.bind(1, key);

	int rc = statement.step();
	if(rc == SQLITE_DONE) {
		return nullptr;
	}
	if(rc != SQLITE_ROW) {
		throw rc;
	}

	return new string{statement.getString(0)};
}
void KVPairDB::insertPair(KVPair pair) {
	string sql = "INSERT INTO " + _table + " VALUES(?, ?)";
	Statement statement{_db, sql};

	statement.bind(1, pair.key);
	statement.bind(2, pair.value);

	int rc = statement.step();
	if(rc != SQLITE_DONE) { throw rc; }
}
void KVPairDB::setPair(KVPair pair) {
	string sql = "UPDATE " + _table + " SET value = ? WHERE key = ?";
	Statement statement{_db, sql};

	statement.bind(1, pair.value);
	statement.bind(2, pair.key);

	int rc = statement.step();
	if(rc != SQLITE_DONE) { throw rc; }
}

