#include "ngram.hpp"
using std::vector;
using std::string;
using std::to_string;
using std::map;
using zidcu::Database;
using zidcu::Statement;

#include <random>
using std::uniform_int_distribution;

#include "util.hpp"
#include "global.hpp"

#include <iostream>
using std::cerr;
using std::endl;

ngram_t::ngram_t(prefix_t iprefix, word_t iatom)
		: prefix(iprefix), atom(iatom) { }

int ngram_t::order() const { return prefix.size(); }


chain_t::chain_t(ngram_t ingram, count_t icount)
		: ngram(ingram), count(icount) { }


ngramStore::ngramStore(Database &db, std::string baseTableName)
		: _db(db), _baseTableName(baseTableName), _builder(baseTableName) { }

chain_t ngramStore::fetch(ngram_t ngram) {
	createTable(ngram.order());
	auto &statement = _db[_builder.ngramFetch(ngram.order())];
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
	createTable(ngram.order());
	{
		auto &statement = _db[_builder.ngramInsert(ngram.order())];
		bind(statement, ngram);
		auto result = statement.execute();
		if(result.status() != SQLITE_DONE) {
			cerr << "ngramStore::increment: error: " << result.status() << endl;
			throw result.status();
		}
	}
	{
		auto &statement = _db[_builder.ngramIncrement(ngram.order())];
		bind(statement, ngram);
		auto result = statement.execute();
		if(result.status() != SQLITE_DONE) {
			cerr << "ngramStore::increment: error: " << result.status() << endl;
			throw result.status();
		}
	}
}
bool ngramStore::exists(ngram_t ngram) {
	createTable(ngram.order());
	auto &statement = _db[_builder.ngramExists(ngram.order())];
	bind(statement, ngram);

	auto result = statement.execute();
	int rc = result.status();
	if(rc != SQLITE_ROW) {
		cerr << "ngramStore::exists: error: " << rc << endl;
		throw rc;
	}

	return (result.getInteger(0) > 0);
}
word_t ngramStore::random(prefix_t prefix) {
	cerr << "ngramStore::random: ";
	for(word_t &w : prefix) cerr << global::dictionary[w] << " ";
	cerr << endl;

	createTable(prefix.size());
	int total = 0;
	{
		auto &statement = _db[_builder.prefixCount(prefix.size())];
		bind(statement, prefix);
		auto result = statement.execute();
		if(result.status() != SQLITE_ROW) { throw result.status(); }
		total = result.getInteger(0);
	}

	uniform_int_distribution<> uid(0, total);
	total = uid(global::rengine);

	long rowCount = 0;
	{
		auto &statement = _db[_builder.prefixFetch(prefix.size())];
		bind(statement, prefix);
		auto result = statement.execute();
		if(result.status() == SQLITE_DONE) return -1;
		if(result.status() != SQLITE_ROW) {
			cerr << "ngramStore::random: prefixFetch failed: " << result.status() << endl;
			throw result.status();
		}
		while(result.status() == SQLITE_ROW) {
			rowCount++;
			total -= result.getInteger(prefix.size() + 1);
			if(total <= 0) {
				cerr << "ngramStore::random: rowCount: " << rowCount << endl;
				return result.getInteger(prefix.size());
			}
			result.step();
		}
	}
	cerr << "ngramStore::random: " << prefix.size() << " order ran off edge" << endl
		<< "    rows: " << rowCount << ", total: " << total << endl;
	throw -1;
}

void ngramStore::bind(Statement &statement, ngram_t &ngram) {
	for(int i = 0; i < ngram.order(); ++i)
		statement.bind(i + 1, ngram.prefix[i]);
	statement.bind(ngram.order() + 1, ngram.atom);
}
void ngramStore::bind(Statement &statement, prefix_t &prefix) {
	for(int i = 0; i < (int)prefix.size(); ++i)
		statement.bind(i + 1, prefix[i]);
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
		clauses.push_back("(" + column(i) + " = ?" + to_string(i + 1) + ")");
	clauses.push_back("(atom = ?" + to_string(order + 1) + ")");
	return util::join(clauses, " AND ");
}
string ngramStoreStatementBuilder::qmarks(int order) const {
	vector<string> qms;
	for(int i = 0; i < order + 1; ++i) // include atom
		qms.push_back("?" + to_string(i + 1));
	return util::join(qms, ", ");
}
string ngramStoreStatementBuilder::createTable(int order) const {
	vector<string> columns;
	for(int i = 0; i < order; ++i)
		columns.push_back(column(i));
	columns.push_back("atom");

	string pkColumns = util::join(columns, ", ");

	columns.push_back("count");
	string allColumns = util::join(columns, " int, ");

	return "CREATE TABLE IF NOT EXISTS " + table(order)
		+ "(" + allColumns + " int, PRIMARY KEY(" + pkColumns + "));";
}
string ngramStoreStatementBuilder::createIndex1(int order) const {
	vector<string> columns;
	for(int i = 0; i < order; ++i)
		columns.push_back(column(i));
	columns.push_back("atom");
	string pkColumns = util::join(columns, ", ");

	return "CREATE UNIQUE INDEX IF NOT EXISTS idx_" + table(order)
		+ " ON " + table(order) + " (" + pkColumns + ");";
}
string ngramStoreStatementBuilder::createIndex2(int order) const {
	return "CREATE INDEX IF NOT EXISTS idx_" + table(order) + "_atom "
		+ " ON " + table(order) + " (atom);";
}
string ngramStoreStatementBuilder::ngramExists(int order) const {
	return "SELECT COUNT(1) FROM " + table(order) + " WHERE " + where(order);
}
string ngramStoreStatementBuilder::ngramFetch(int order) const {
	return "SELECT count FROM " + table(order) + " WHERE " + where(order);
}
string ngramStoreStatementBuilder::ngramInsert(int order) const {
	return "INSERT OR IGNORE INTO " + table(order)
		+ " VALUES(" + qmarks(order) + ", 0);";
}
string ngramStoreStatementBuilder::ngramIncrement(int order) const {
	return "UPDATE " + table(order) + " SET count = count + 1 "
		+ " WHERE " + where(order);
}
string ngramStoreStatementBuilder::prefixCount(int order) const {
	vector<string> clauses;
	for(int i = 0; i < order; ++i)
		clauses.push_back("(" + column(i) + " = ?" + to_string(i + 1) + ")");
	string where = util::join(clauses, " AND ");
	if(!where.empty()) where = " WHERE " + where;

	return "SELECT SUM(count * count) FROM " + table(order) + where;
}
string ngramStoreStatementBuilder::prefixFetch(int order) const {
	vector<string> columns;
	for(int i = 0; i < order; ++i)
		columns.push_back(column(i));

	string pkColumns = util::join(columns, ", ");
	if(!pkColumns.empty()) pkColumns += ",";

	vector<string> clauses;
	for(int i = 0; i < order; ++i)
		clauses.push_back("(" + column(i) + " = ?" + to_string(i + 1) + ")");
	string where = util::join(clauses, " AND ");
	if(!where.empty()) where = " WHERE " + where;

	return "SELECT " + pkColumns + " atom, count * count FROM " + table(order) + where;
}


void ngramStore::createTable(int order) {
	while((int)_tableCache.size() <= order)
		_tableCache.push_back(false);
	if(_tableCache[order])
		return;

	{
		auto &statement = _db[_builder.createTable(order)];
		auto result = statement.execute();
		if(result.status() != SQLITE_DONE) { throw result.status(); }
	}
	{
		auto &statement = _db[_builder.createIndex1(order)];
		auto result = statement.execute();
		if(result.status() != SQLITE_DONE) { throw result.status(); }
	}

	if(order > 0) {
		auto &statement = _db[_builder.createIndex2(order)];
		auto result = statement.execute();
		if(result.status() != SQLITE_DONE) { throw result.status(); }
	}

	_tableCache[order] = true;
}

