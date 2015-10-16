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

Variable makeVariable(string str);
Variable makeVariable(string str) { return Variable::parse(str); }

template<typename Store> vector<Variable> getList(Store &store, string variable) {
	string lists = store.get(variable).toString();
	auto list = makeList(lists);
	vector<Variable> vars(list.size());
	transform(list.begin(), list.end(), vars.begin(), makeVariable);
	return vars;
}

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
	_db.executeVoid("INSERT OR IGNORE INTO " + _varTable + " VALUES(?1, ?2, ?3)",
			name, var.value, typeToString(var.type));
	_db.executeVoid("UPDATE " + _varTable + " SET body = ?1, type = ?2 WHERE name = ?3",
			var.value, typeToString(var.type), name);
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

vector<Variable> VarStore::getList(string variable) {
	return ::getList(*this, variable);
}

vector<string> VarStore::getVariablesOfType(Type type) {
	createTables();
	vector<string> vars;
	auto results = _db.execute("SELECT name, type FROM " + _varTable + " WHERE type = ?1",
			typeToString(type));
	// TODO: I'm baffled by this... I've been out of my mind for days maybe that's it
	// TODO: investigate and see if a previous query is not being closed?
	// TODO: do get queries only work because they request one element?
	// TODO: is type a kewyord?
	while(results.status() == SQLITE_ROW) {
		if(results.getString(1) != typeToString(type)) {
			cerr << "added '" << results.getString(0) << "' : "
				<< results.getString(1) << " != " << typeToString(type) << endl;
		} else {
			vars.push_back(results.getString(0));
		}
		results.step();
	}
	if(results.status() == SQLITE_DONE)
		return vars;

	throw make_except("sqlite error: " + to_string(results.status()));
}

LocalVarStore::LocalVarStore() { }

Variable LocalVarStore::get(string name) {
	if(this->defined(name))
		return _vars[name];
	return Variable{};
}
Variable LocalVarStore::set(string name, Variable var) {
	return _vars[name] = var;
}

bool LocalVarStore::defined(string name) {
	return _vars.find(name) != _vars.end();
}
void LocalVarStore::erase(string name) {
	if(!this->defined(name))
		return;
	_vars.erase(_vars.find(name));
}

vector<Variable> LocalVarStore::getList(string variable) {
	return ::getList(*this, variable);
}

vector<string> LocalVarStore::getVariablesOfType(Type type) {
	// TODO: currently would be very inefficient...
	throw make_except("not implemented");
	vector<string> vars;
	return vars;
}

