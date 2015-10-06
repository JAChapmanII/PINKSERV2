#include "varstore.hpp"
using std::string;
using std::to_string;
using std::vector;
using zidcu::Database;

#include <algorithm>
using std::transform;

#include "err.hpp"

#include <iostream>
using std::cerr;
using std::endl;

VarStore::VarStore(Database &db, string varTableName, string permTableName)
		: _db{db}, _varTable(varTableName), _permTable(permTableName) { }

Variable VarStore::get(string name) {
	createTables();
	auto result = _db.executeScalar<string>(
		"SELECT body FROM " + _varTable + " WHERE name = ?1",
		name);

	if(result)
		return Variable(*result);
	else
		return Variable();
}
Variable VarStore::set(string name, Variable var) {
	createTables();
	auto tran = _db.transaction();
	_db.executeVoid("INSERT OR IGNORE INTO " + _varTable + " VALUES(?1, ?2, 0)",
			name, var.toString());
	_db.executeVoid("UPDATE " + _varTable + " SET body = ?1 WHERE name = ?2",
			var.toString(), name);
	return var;
}
bool VarStore::defined(string name) {
	createTables();
	auto result = _db.executeScalar<int>(
			"SELECT COUNT(1) FROM " + _varTable + " WHERE name = ?1",
			name);
	if(!result) throw make_except("expected count value");
	return *result;
}
void VarStore::erase(string name) {
	createTables();
	_db.executeVoid("DELETE FROM " + _varTable + " WHERE name = ?1",
			name);
}

void VarStore::createTables() {
	if(_tablesCreated) return;

	auto tran = _db.transaction();
	_db.executeVoid("CREATE TABLE IF NOT EXISTS " + _varTable
		+ " (name text primary key, body text, execute bit)");
	_db.executeVoid("CREATE INDEX IF NOT EXISTS " + _varTable + "_x_idx"
		+ " ON " + _varTable + " (execute)");
	_tablesCreated = true;
}

Variable makeVariable(string str);
Variable makeVariable(string str) { return Variable::parse(str); }

vector<Variable> VarStore::getList(string variable) {
	string lists = get(variable).toString();
	auto list = makeList(lists);
	vector<Variable> vars(list.size());
	transform(list.begin(), list.end(), vars.begin(), makeVariable);
	return vars;
}

void VarStore::markExecutable(string name, bool x) {
	createTables();
	auto tran = _db.transaction();
	_db.executeVoid("UPDATE " + _varTable + " SET execute = ?1 WHERE name = ?2",
			x ? 1 : 0, name);
}
vector<string> VarStore::getExecutable() {
	createTables();
	vector<string> executable;
	auto results = _db.execute("SELECT * FROM " + _varTable + " WHERE execute = 1");
	while(results.status() == SQLITE_ROW) {
		executable.push_back(results.getString(0));
		results.step();
	}
	if(results.status() == SQLITE_DONE)
		return executable;

	throw make_except("sqlite error: " + to_string(results.status()));
}

