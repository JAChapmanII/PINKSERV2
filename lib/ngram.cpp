#include "ngram.hpp"
using std::vector;
using std::string;
using std::to_string;
using std::map;
using zidcu::Database;
using zidcu::Statement;

#include <random>
using std::uniform_int_distribution;
using std::mt19937_64;

#include "util.hpp"
#include "err.hpp"

#include <iostream>
using std::cerr;
using std::endl;


void bind(Statement &statement, ngram_t &ngram) {
	for(int i = 0; i < ngram.order(); ++i)
		statement.bind(i + 1, ngram.prefix[i]);
	statement.bind(ngram.order() + 1, ngram.atom);
}
void bind(Statement &statement, prefix_t &prefix) {
	for(int i = 0; i < (int)prefix.size(); ++i)
		statement.bind(i + 1, prefix[i]);
}

namespace zidcu {
	template<> void Statement::bind<ngram_t>(int, ngram_t val)
		{ ::bind(*this, val); }
	template<> void Statement::bind<prefix_t>(int, prefix_t val)
		{ ::bind(*this, val); }
}

ngram_t::ngram_t(prefix_t iprefix, word_t iatom)
		: prefix(iprefix), atom(iatom) { }

int ngram_t::order() const { return prefix.size(); }


chain_t::chain_t(ngram_t ingram, count_t icount)
		: ngram(ingram), count(icount) { }


ngramStore::ngramStore(Database &db, std::string baseTableName)
		: _db(db), _baseTableName(baseTableName), _builder(baseTableName) { }

chain_t ngramStore::fetch(ngram_t ngram) {
	createTable(ngram.order());
	auto count = _db.executeScalar<sqlite_int64>(
			_builder.ngramFetch(ngram.order()),
			ngram);
	return chain_t{ngram, count ? *count : 0};
}
void ngramStore::increment(ngram_t ngram, int amount) {
	createTable(ngram.order());
	auto &statement = _db[_builder.ngramIncrement(ngram.order())];
	statement.bind(0, ngram);
	statement.bind(ngram.order() + 2, amount);
	statement.executeVoid();
	if(sqlite3_changes(_db.getDB()) == 0 && amount > 0)
		_db.executeVoid(_builder.ngramInsert(ngram.order()), ngram);
}
bool ngramStore::exists(ngram_t ngram) {
	createTable(ngram.order());
	auto count = _db.executeScalar<int>(
			_builder.ngramExists(ngram.order()),
			ngram);
	return (count ? *count > 0 : false);
}
sqlite_int64 ngramStore::chainsWithPrefix(prefix_t prefix) {
	createTable(prefix.size());
	return _db.executeScalar<sqlite_int64>(
			_builder.chainsWithPrefix(prefix.size()), prefix)
		.value_or(0);
}
sqlite_int64 ngramStore::count() {
	static const int maxOrder = 6;
	sqlite_int64 count = 0;
	for(int order = 0; order < maxOrder; ++order) {
		createTable(order);
		count += _db.executeScalar<sqlite_int64>(_builder.count(order))
			.value_or(0);
	}
	return count;
}
template<typename Generator>
word_t ngramStore::random(prefix_t prefix, Generator &g) {
	createTable(prefix.size());
	auto total = _db.executeScalar<sqlite_int64>(
			_builder.prefixCount(prefix.size()), prefix).value_or(0);

	uniform_int_distribution<> uid(0, total);
	total = uid(g);

	long rowCount = 0;
	{
		auto result = _db.execute(_builder.prefixFetch(prefix.size()), prefix);
		if(result.status() == SQLITE_DONE) return -1;
		if(result.status() != SQLITE_ROW) {
			throw make_except("ngramStore::random: prefixFetch failed: " + to_string(result.status()));
		}
		while(result.status() == SQLITE_ROW) {
			rowCount++;
			total -= result.getLong(prefix.size() + 1);
			if(total <= 0)
				return result.getLong(prefix.size());
			result.step();
		}
	}
	throw make_except(string{"ngramStore::random: "} + to_string(prefix.size())
			+ " order ran off edge\n    rows: " + to_string(rowCount)
			+ ", total: " + to_string(total));
}

template word_t ngramStore::random<mt19937_64>(prefix_t prefix, mt19937_64 &g);

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
string ngramStoreStatementBuilder::ngramExists(int order) const {
	return "SELECT COUNT(1) FROM " + table(order) + " WHERE " + where(order);
}
string ngramStoreStatementBuilder::ngramFetch(int order) const {
	return "SELECT count FROM " + table(order) + " WHERE " + where(order);
}
string ngramStoreStatementBuilder::ngramInsert(int order) const {
	return "INSERT OR IGNORE INTO " + table(order)
		+ " VALUES(" + qmarks(order) + ", 1);";
}
string ngramStoreStatementBuilder::ngramIncrement(int order) const {
	return "UPDATE " + table(order) + " SET count = "
			+ " max(0, count + ?" + to_string(order + 2) + ")"
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
string ngramStoreStatementBuilder::chainsWithPrefix(int order) const {
	if(order == 0)
		return count(0);

	vector<string> clauses;
	for(int i = 0; i < order; ++i)
		clauses.push_back("(" + column(i) + " = ?" + to_string(i + 1) + ")");

	return "SELECT COUNT(1) FROM " + table(order)
		+ " WHERE " + util::join(clauses, " AND ");
}
string ngramStoreStatementBuilder::count(int order) const {
	return "SELECT COUNT(1) FROM " + table(order);
}


void ngramStore::createTable(int order) {
	while((int)_tableCache.size() <= order)
		_tableCache.push_back(false);
	if(_tableCache[order])
		return;

	_db.executeVoid(_builder.createTable(order));

	_tableCache[order] = true;
}

