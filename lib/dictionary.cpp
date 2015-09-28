#include "dictionary.hpp"
using std::string;
using zidcu::Database;
using zidcu::Statement;

#include <iostream>
using std::cerr;
using std::endl;

Dictionary::Dictionary(Database &db, string tableName)
		: _db{db}, _tableName{tableName} { }

string Dictionary::get(sqlite_int64 key) {
	createTable();
	if(key == (sqlite_int64)Anchor::Start)
		return "{^}";
	if(key == (sqlite_int64)Anchor::End)
		return "{$}";

	auto str = _db.executeScalar<string>(
			"SELECT str FROM " + _tableName + " WHERE code = ?1",
			key - (sqlite_int64)Anchor::ReservedCount);
	return (str ? *str : "");
}
string Dictionary::operator[](sqlite_int64 key) {
	return this->get(key);
}

void Dictionary::insert(string value) {
	createTable();
	auto tran = _db.transaction();
	_db.executeVoid("INSERT INTO " + _tableName + " (str) VALUES (?1)", value);
}
sqlite_int64 Dictionary::get(string value) {
	if(value.empty() || value.find_first_not_of(" \t\r\n") == string::npos)
		throw make_except("dictionary cannot into space");
	createTable();
	auto code = _db.executeScalar<sqlite_int64>(
			"SELECT code FROM " + _tableName + " WHERE str = ?1",
			value);
	if(!code) {
		this->insert(value);
		return this->get(value);
	}
	return *code + (sqlite_int64)Anchor::ReservedCount;
}
sqlite_int64 Dictionary::operator[](string value) {
	return this->get(value);
}


sqlite_int64 Dictionary::size() {
	createTable();
	auto count = _db.executeScalar<sqlite_int64>(
			"SELECT COUNT(1) FROM " + _tableName);
	return (count ? *count : 0);
}

bool Dictionary::isAnchor(sqlite_int64 key) {
	return key >= 0 && key <= (sqlite_int64)Anchor::ReservedCount;
}
bool Dictionary::isInvalidAnchor(sqlite_int64 key) {
	return key >= (sqlite_int64)Anchor::Count
		&& key <= (sqlite_int64)Anchor::ReservedCount;
}

void Dictionary::createTable() {
	if(_createdTable) return;

	auto tran = _db.transaction();
	_db.executeVoid("CREATE TABLE IF NOT EXISTS " + _tableName
			+ " (str text, code integer primary key);");
	_db.executeVoid("CREATE UNIQUE INDEX IF NOT EXISTS "
			+ _tableName + "_usc_idx "
			+ " ON " + _tableName + " (str, code);");
	_db.executeVoid("CREATE INDEX IF NOT EXISTS "
			+ _tableName + "_str_idx "
			+ " ON " + _tableName + " (str);");
	_db.executeVoid("CREATE INDEX IF NOT EXISTS "
			+ _tableName + "_code_idx "
			+ " ON " + _tableName + " (code);");

	_createdTable = true;
}

