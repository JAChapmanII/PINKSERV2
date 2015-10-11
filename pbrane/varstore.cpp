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

VarStore::VarStore(Database &db, string varTableName)
		: _db{db}, _varTable(varTableName) { }

Variable VarStore::get(string name) {
	createTables();
	auto row = _db.execute(
		"SELECT body, type FROM " + _varTable + " WHERE name = ?1",
		name);
	if(row.status() == SQLITE_ROW) {
		Variable var{};
		var.type = typeFromString(row.getString(1));
		var.value = row.getString(0);
		return var;
	}

	return Variable{};
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
		+ " (name text primary key, body text, type text)");
	_db.executeVoid("CREATE INDEX IF NOT EXISTS " + _varTable + "_type_idx"
		+ " ON " + _varTable + " (type)");
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

vector<string> VarStore::getVariablesOfType(Type type) {
	createTables();
	vector<string> vars;
	auto results = _db.execute("SELECT * FROM " + _varTable + " WHERE type = ?1",
			typeToString(type));
	while(results.status() == SQLITE_ROW) {
		vars.push_back(results.getString(0));
		results.step();
	}
	if(results.status() == SQLITE_DONE)
		return vars;

	throw make_except("sqlite error: " + to_string(results.status()));
}

