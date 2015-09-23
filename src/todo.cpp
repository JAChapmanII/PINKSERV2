#include "todo.hpp"
using std::string;
using zidcu::Database;

TODOs::TODOs(Journal &journal, Database &db, string table)
	: _journal{journal}, _db{db}, _table{table} { }

bool TODOs::push(string text) {
	createTables();

	auto res = _journal.fetch(NoopPredicate, 1);
	if(res.size() != 1)
		throw make_except("fetch returned over limit");
	auto last = res[0];

	auto tran = _db.transaction();
	_db.executeVoid("INSERT INTO " + _table + "(line, todo) VALUES(?1, ?2)",
			last.id, text);

	return true;
}

sqlite_int64 TODOs::unresolvedCount() {
	createTables();
	return _db.executeScalar<sqlite_int64>(
			"SELECT COUNT(1) FROM " + _table + " WHERE resolved IS NULL").value_or(0);
}

void TODOs::createTables() {
	if(_tablesCreated) return;

	auto tran = _db.transaction();
	_db.executeVoid("CREATE TABLE IF NOT EXISTS " + _table
			+ " ( id integer primary key, line integer not null, todo text, "
				+ " resolved integer, note text, rnote text )");

	_tablesCreated = true;
}

