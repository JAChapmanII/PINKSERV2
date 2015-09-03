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

	auto &statement = _db["SELECT str FROM " + _tableName + " WHERE code = ?1"];
	statement.bind(1, key - (sqlite_int64)Anchor::ReservedCount);
	auto result = statement.execute();
	if(result.status() == SQLITE_DONE) return "";
	if(result.status() != SQLITE_ROW) {
		cerr << "Dictionary::get(int64): sqlite error: " << result.status() << endl;
		throw result.status();
	}
	return result.getString(0);
}
string Dictionary::operator[](sqlite_int64 key) {
	return this->get(key);
}

void Dictionary::insert(string value) {
	auto tran = _db.transaction();
	auto &statement = _db["INSERT INTO " + _tableName + " (str) VALUES (?1)"];
	statement.bind(1, value);
	auto result = statement.execute();
	if(result.status() != SQLITE_DONE) {
		cerr << "Dictionary::insert: sqlite error: " << result.status() << endl;
		throw result.status();
	}
}
sqlite_int64 Dictionary::get(string value) {
	createTable();
	auto &statement = _db["SELECT code FROM " + _tableName + " WHERE str = ?1"];
	statement.bind(1, value);
	auto result = statement.execute();
	// not found, insert and return new value
	if(result.status() == SQLITE_DONE) {
		insert(value);
		return get(value);
	}
	return result.getInteger(0) + (sqlite_int64)Anchor::ReservedCount;
}
sqlite_int64 Dictionary::operator[](string value) {
	return this->get(value);
}


sqlite_int64 Dictionary::size() {
	createTable();
	auto &statement = _db["SELECT COUNT(1) FROM " + _tableName];
	auto result = statement.execute();
	if(result.status() != SQLITE_ROW) {
		cerr << "Dictionary::size: sqlite error: " << result.status() << endl;
		throw result.status();
	}

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
	{
		auto &statement = _db[
			"CREATE TABLE IF NOT EXISTS " + _tableName + " (str text, code integer primary key);"];
		auto result = statement.execute();
		if(result.status() != SQLITE_DONE) {
			cerr << "Dictionary::createTable: sqlite error: " << result.status() << endl;
			throw result.status();
		}
	}
	{
		auto &statement = _db[
			"CREATE UNIQUE INDEX IF NOT EXISTS " + _tableName + "_usc_idx "
				+ " ON " + _tableName + " (str, code);"];
		auto result = statement.execute();
		if(result.status() != SQLITE_DONE) {
			cerr << "Dictionary::createTable: sqlite error: " << result.status() << endl;
			throw result.status();
		}
	}
	{
		auto &statement = _db[
			"CREATE INDEX IF NOT EXISTS " + _tableName + "_str_idx "
				+ " ON " + _tableName + " (str);"];
		auto result = statement.execute();
		if(result.status() != SQLITE_DONE) {
			cerr << "Dictionary::createTable: sqlite error: " << result.status() << endl;
			throw result.status();
		}
	}
	{
		auto &statement = _db[
			"CREATE INDEX IF NOT EXISTS " + _tableName + "_code_idx "
				+ " ON " + _tableName + " (code);"];
		auto result = statement.execute();
		if(result.status() != SQLITE_DONE) {
			cerr << "Dictionary::createTable: sqlite error: " << result.status() << endl;
			throw result.status();
		}
	}

	_createdTable = true;
}

