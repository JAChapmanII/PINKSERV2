#include "ngram.hpp"
using std::vector;
using std::string;
using std::to_string;
using std::map;
using db::Database;
using db::Statement;

#include "util.hpp"

#include <iostream>
using std::cerr;
using std::endl;

ngram_t::ngram_t(prefix_t iprefix, word_t iatom)
		: prefix(iprefix), atom(iatom) { }

int ngram_t::order() const { return prefix.size(); }


chain_t::chain_t(ngram_t ingram, count_t icount)
		: ngram(ingram), count(icount) { }


ngramStore::ngramStore(Database &db, std::string baseTableName)
		: _db(db), _baseTableName(baseTableName), _builder(baseTableName),
		  _cache(_db, _builder), _tableCache(_cache) { }

chain_t ngramStore::fetch(ngram_t ngram) {
	_tableCache.exists(ngram.order());
	auto &statement = _cache.ngramFetch(ngram.order());
	bind(statement, ngram);

	chain_t chain{ngram};
	auto result = statement.execute();
	auto rc = result.status();

	if(rc == SQLITE_DONE) { return ngram; }
	if(rc != SQLITE_ROW) { throw rc; }

	chain.count = result.getInteger(0);
	return chain;
}
void ngramStore::increment(ngram_t ngram) {
	_tableCache.exists(ngram.order());
	int rc = SQLITE_ERROR;
	if(exists(ngram)) {
		auto &statement = _cache.ngramIncrement(ngram.order());
		bind(statement, ngram);
		auto result = statement.execute();
		rc = result.status();
	} else {
		auto &statement = _cache.ngramInsert(ngram.order());
		bind(statement, ngram);
		auto result = statement.execute();
		rc = result.status();
	}
	if(rc != SQLITE_DONE) {
		cerr << "ngramStore::increment: error: " << rc << endl;
		throw rc;
	}
}
bool ngramStore::exists(ngram_t ngram) {
	_tableCache.exists(ngram.order());
	auto &statement = _cache.ngramExists(ngram.order());
	bind(statement, ngram);

	auto result = statement.execute();
	int rc = result.status();
	if(rc != SQLITE_ROW) {
		cerr << "ngramStore::exists: error: " << rc << endl;
		throw rc;
	}

	return (result.getInteger(0) > 0);
}

void ngramStore::bind(Statement &statement, ngram_t &ngram) {
	for(int i = 0; i < ngram.order(); ++i)
		statement.bind(i + 1, ngram.prefix[i]);
	statement.bind(ngram.order() + 1, ngram.atom);
}



ngramStoreStatementBuilder::ngramStoreStatementBuilder(string baseTableName)
		: _baseTableName(baseTableName) { }
string ngramStoreStatementBuilder::table(int order) const {
	return _baseTableName + "_" + to_string(order);
}
string ngramStoreStatementBuilder::column(int p) const {
	return "s_" + to_string(p);
}
string ngramStoreStatementBuilder::where(int order) const {
	vector<string> clauses;
	for(int i = 0; i < order; ++i)
		clauses.push_back(" (" + column(i) + " = ? )");
	clauses.push_back(" (atom = ?)");
	return util::join(clauses, " AND ");
}
string ngramStoreStatementBuilder::qmarks(int order) const {
	vector<string> qms{order + 1, "?"}; // include atom
	return util::join(qms, ", ");
}
string ngramStoreStatementBuilder::createTable(int order) const {
	vector<string> columns;
	for(int i = 0; i < order; ++i)
		columns.push_back(column(i));
	columns.push_back("atom");
	columns.push_back("count");

	string sql = "CREATE TABLE IF NOT EXISTS " + table(order);
	sql += "( " + util::join(columns, " int, ") + " int);";

	columns.pop_back(); // remove count for index

	sql += "CREATE UNIQUE INDEX IF NOT EXISTS idx_" + table(order)
		+ " ( " + util::join(columns, ", ") + ");";
	return sql;
}
string ngramStoreStatementBuilder::ngramExists(int order) const {
	return "SELECT COUNT(1) FROM " + table(order) + " WHERE " + where(order);
}
string ngramStoreStatementBuilder::ngramFetch(int order) const {
	return "SELECT count FROM " + table(order) + " WHERE " + where(order);
}
string ngramStoreStatementBuilder::ngramInsert(int order) const {
	return "INSERT INTO " + table(order) + " VALUES(" + qmarks(order) + ", 1)";
}
string ngramStoreStatementBuilder::ngramIncrement(int order) const {
	return "UPDATE " + table(order) + " SET count = count + 1 "
		+ " WHERE " + where(order);
}


StatementCache::StatementCache(Database &db) : _db{db}, _cache{} { }
StatementCache::~StatementCache() { for(auto &e : _cache) delete e.second; }

Statement &StatementCache::operator[](string sql) {
	if(_cache.find(sql) != _cache.end())
		return *_cache[sql];
	return *(_cache[sql] = new Statement{_db, sql});
}

ngramStatementCache::ngramStatementCache(
		Database &db, ngramStoreStatementBuilder builder)
			: _builder(builder),  _cache(db) { }

Statement &ngramStatementCache::createTable(int order) {
	if(_tableCache.find(order) == _tableCache.end())
		_tableCache[order] = _builder.createTable(order);
	return _cache[_tableCache[order]];
}
Statement &ngramStatementCache::ngramExists(int order) {
	if(_existsCache.find(order) == _existsCache.end())
		_existsCache[order] = _builder.ngramExists(order);
	return _cache[_existsCache[order]];
}
Statement &ngramStatementCache::ngramFetch(int order) {
	if(_fetchCache.find(order) == _fetchCache.end())
		_fetchCache[order] = _builder.ngramFetch(order);
	return _cache[_fetchCache[order]];
}
Statement &ngramStatementCache::ngramInsert(int order) {
	if(_insertCache.find(order) == _insertCache.end())
		_insertCache[order] = _builder.ngramInsert(order);
	return _cache[_insertCache[order]];
}
Statement &ngramStatementCache::ngramIncrement(int order) {
	if(_incrementCache.find(order) == _incrementCache.end())
		_incrementCache[order] = _builder.ngramIncrement(order);
	return _cache[_incrementCache[order]];
}

ngramTableCache::ngramTableCache(ngramStatementCache &cache) : _cache(cache) { }

bool ngramTableCache::exists(int order) {
	if(_exists.find(order) != _exists.end())
		return true;

	auto &statement = _cache.createTable(order);
	auto result = statement.execute();
	int rc = result.status();
	if(rc != SQLITE_DONE) { throw rc; }


	return _exists[order] = true;
}

