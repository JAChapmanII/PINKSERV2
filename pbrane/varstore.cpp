#include "varstore.hpp"
using std::string;
using std::vector;
using zidcu::Database;

#include <iostream>
using std::cerr;
using std::endl;

VarStore::VarStore(Database &db, string varTableName, string permTableName)
		: _db{db}, _varTable(varTableName), _permTable(permTableName) { }

Variable VarStore::get(string name) {
	return Variable(getString(name), Permissions());
}
string VarStore::getString(string name) {
	createTables();
	auto &statement = _db["SELECT body FROM " + _varTable + " WHERE name = ?1"];
	statement.bind(1, name);
	auto result = statement.execute();
	if(result.status() == SQLITE_DONE) { return ""; }
	if(result.status() != SQLITE_ROW) {
		cerr << "VarStore::get: sqlite error: " << result.status() << endl;
		throw result.status();
	}
	return result.getString(0);
}
Variable VarStore::set(string name, Variable var) {
	return set(name, var.toString());
}
Variable VarStore::set(string name, string val) {
	createTables();
	auto tran = _db.transaction();
	{
		auto &statement = _db["INSERT OR IGNORE INTO " + _varTable
			+ " VALUES(?1, ?2, 0)"];
		statement.bind(1, name);
		statement.bind(2, val);
		auto result = statement.execute();
		if(result.status() != SQLITE_DONE) {
			cerr << "VarStore::set: sqlite error: " << result.status() << endl;
			throw result.status();
		}
	}
	{
		auto &statement = _db["UPDATE " + _varTable
			+ " SET body = ?1 WHERE name = ?2"];
		statement.bind(1, val);
		statement.bind(2, name);
		auto result = statement.execute();
		if(result.status() != SQLITE_DONE) {
			cerr << "VarStore::set: sqlite error2: " << result.status() << endl;
			throw result.status();
		}
	}
	return Variable(val, Permissions());
}
bool VarStore::defined(string name) {
	createTables();
	auto &statement = _db["SELECT COUNT(1) FROM " + _varTable + " WHERE name = ?1"];
	statement.bind(1, name);
	auto result = statement.execute();
	if(result.status() != SQLITE_ROW) {
		cerr << "VarStore::defined: sqlite error: " << result.status() << endl;
		throw result.status();
	}
	return result.getInteger(0);
}
void VarStore::erase(string name) {
	createTables();
	auto &statement = _db["DELETE FROM " + _varTable + " WHERE name = ?1"];
	statement.bind(1, name);
	auto result = statement.execute();
	if(result.status() != SQLITE_DONE) {
		cerr << "VarStore::defined: sqlite error: " << result.status() << endl;
		throw result.status();
	}
}

void VarStore::createTables() {
	if(_tablesCreated) return;

	auto tran = _db.transaction();
	{
		auto &statement = _db["CREATE TABLE IF NOT EXISTS " + _varTable
			+ " (name text primary key, body text, execute bit)"];
		auto result = statement.execute();
		if(result.status() != SQLITE_DONE) {
			cerr << "VarStore::createTables: sqlite error: " << result.status() << endl;
			throw result.status();
		}
	}
	{
		auto &statement = _db["CREATE INDEX IF NOT EXISTS " + _varTable + "_x_idx"
			+ " ON " + _varTable + " (execute)"];
		auto result = statement.execute();
		if(result.status() != SQLITE_DONE) {
			cerr << "VarStore::createTables: sqlite error: " << result.status() << endl;
			throw result.status();
		}
	}

	_tablesCreated = true;
}

vector<string> VarStore::getList(string variable) {
	string lists = get(variable).toString();
	return makeList(lists);
}


