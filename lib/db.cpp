#include "db.hpp"
using std::string;
using std::to_string;

#include "err.hpp"

#include <iostream>
using std::cerr;
using std::endl;

zidcu::Database::Database() { }
zidcu::Database::Database(string fileName) : _fileName(fileName) { open(_fileName); }
zidcu::Database::~Database() {
	if(_cache) { delete _cache; }
	if(_startTransaction) { delete _startTransaction; }
	if(_commitTransaction) { delete _commitTransaction; }
	if(_opened) {
		if(_db) {
			int rc = sqlite3_close(_db);
			if(rc != SQLITE_OK) {
				throw make_except(string{"Database::~Database: error closing db "}
					+ "\"" + _fileName + "\": error: "
					+ string{sqlite3_errmsg(_db)} + ": " + to_string(rc));
			}
		} else {
			cerr << "Database::~Database: db open but null" << endl;
		}
	}
}

sqlite3 *zidcu::Database::getDB() { return _db; }
zidcu::Transaction zidcu::Database::transaction() {
	if(!_startTransaction) {
		_startTransaction = new Statement{*this, "BEGIN TRANSACTION"};
	}
	if(!_commitTransaction) {
		_commitTransaction = new Statement{*this, "COMMIT TRANSACTION"};
	}
	return Transaction{*_startTransaction, *_commitTransaction};
}

void zidcu::Database::open(string fileName) {
	if(_opened) {
		if(fileName == _fileName) return;
		throw make_except("zidcu::Database::open: tried to open new db");
	}
	_fileName = fileName;

	int rc = sqlite3_open(_fileName.c_str(), &_db);
	if(rc != SQLITE_OK) {
		throw make_except(string{"zidcu::Database::Database: error opening db "}
			+ "\"" + _fileName + "\": error: "
			+ sqlite3_errmsg(_db) + ": " + to_string(rc));
	}
}

zidcu::Statement &zidcu::Database::operator[](string sql) {
	if(!_cache)
		_cache = new StatementCache(*this);
	return (*_cache)[sql];
}

zidcu::Statement::Statement(Database &db, string sql) : _db(db), _sql(sql) {
	const char *leftover{nullptr};
	int rc = sqlite3_prepare_v2(_db.getDB(), _sql.c_str(), _sql.size(),
			&_statement, &leftover);
	if(rc != SQLITE_OK) {
		throw make_except(string{"zidcu::Statement::Statement: "}
				+ "error preparing statement: \"" + _sql + "\": " + to_string(rc));
	}
	if(*leftover != '\0')
		throw make_except("zidcu::Statement::Statement: leftover: " + string{leftover});
}
zidcu::Statement::~Statement() {
	if(_statement) {
		int rc = sqlite3_finalize(_statement);
		if(rc != SQLITE_OK) {
			throw make_except("zidcu::Statement::~Statement: error finalizing statement: "
					+ to_string(rc));
		}
	}
}

void zidcu::Statement::bind(int idx, int val) {
	sqlite3_bind_int(_statement, idx, val);
}
void zidcu::Statement::bind(int idx, sqlite_int64 val) {
	sqlite3_bind_int64(_statement, idx, val);
}
void zidcu::Statement::bind(int idx, double val) {
	sqlite3_bind_double(_statement, idx, val);
}
void zidcu::Statement::bind(int idx, const char *val) {
	sqlite3_bind_text(_statement, idx, val, -1, SQLITE_TRANSIENT);
}
void zidcu::Statement::bind(int idx, string val) {
	sqlite3_bind_text(_statement, idx, val.c_str(), val.size(), SQLITE_STATIC);
}

zidcu::Result zidcu::Statement::execute() { return Result(*this); }
void zidcu::Statement::executeVoid() {
	auto result = this->execute();
	if(result.status() == SQLITE_DONE)
		return;

	throw make_except("expected done not: " + to_string(result.status()));
}

string zidcu::Statement::sql() const { return _sql; }
sqlite3_stmt *zidcu::Statement::getStatement() { return _statement; }


zidcu::Result::Result(Statement &statement) : _statement(statement) { step(); }
zidcu::Result::~Result() { sqlite3_reset(_statement.getStatement()); }

int zidcu::Result::status() { return _rc; }
int zidcu::Result::step() { return _rc = sqlite3_step(_statement.getStatement()); }

int zidcu::Result::getInteger(int idx) {
	return sqlite3_column_int(_statement.getStatement(), idx);
}
sqlite_int64 zidcu::Result::getLong(int idx) {
	return sqlite3_column_int64(_statement.getStatement(), idx);
}
string zidcu::Result::getString(int idx) {
	const unsigned char *result = sqlite3_column_text(_statement.getStatement(), idx);
	return string{(char *)result};
}
namespace zidcu {
	template<> int Result::get<int>(int idx) {
		return this->getInteger(idx);
	}
	template<> sqlite_int64 Result::get<sqlite_int64>(int idx) {
		return this->getLong(idx);
	}
	template<> string Result::get<string>(int idx) {
		return this->getString(idx);
	}
}

zidcu::Transaction::Transaction(Statement &start, Statement &end)
		: _start(start), _end(end) {
	_start.executeVoid();
}
zidcu::Transaction::~Transaction() {
	_end.executeVoid();
}

zidcu::StatementCache::StatementCache(Database &db) : _db{db}, _cache{} { }
zidcu::StatementCache::~StatementCache() { for(auto &e : _cache) delete e.second; }

zidcu::Statement &zidcu::StatementCache::operator[](string sql) {
	if(_cache.find(sql) != _cache.end())
		return *_cache[sql];
	return *(_cache[sql] = new Statement{_db, sql});
}


