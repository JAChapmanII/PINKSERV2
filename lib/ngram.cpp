#include "ngram.hpp"
using std::vector;
using std::string;
using std::to_string;
using db::Database;
using db::Statement;

#include "util.hpp"

ngram_t::ngram_t(prefix_t iprefix, word_t iatom)
		: prefix(iprefix), atom(iatom) { }

int ngram_t::order() const { return prefix.size(); }


chain_t::chain_t(ngram_t ingram, count_t icount)
		: ngram(ingram), count(icount) { }


ngramStore::ngramStore(Database &db, std::string baseTableName)
		: _db(db), _baseTableName(baseTableName) { }

chain_t ngramStore::fetch(ngram_t ngram) {
	createTable(ngram.order());
	string sql = "SELECT count FROM " + table(ngram.order())
		+ " WHERE " + where(ngram.order());
	Statement statement{_db, sql};
	bind(statement, ngram);

	chain_t chain{ngram};
	int rc = statement.step();

	if(rc == SQLITE_DONE) { return ngram; }
	if(rc != SQLITE_ROW) { throw rc; }

	chain.count = statement.getInteger(0);
	return chain;
}
void ngramStore::increment(ngram_t ngram) {
	createTable(ngram.order());
	string sql = "INSERT INTO " + table(ngram.order())
		+ " VALUES(" + qmarks(ngram.order()) + ", 1)";
	if(exists(ngram)) {
		sql = "UPDATE " + table(ngram.order()) + " SET count = count + 1 "
			+ " WHERE " + where(ngram.order());
	}
	Statement statement{_db, sql};
	bind(statement, ngram);

	int rc = statement.step();
	if(rc != SQLITE_DONE) { throw rc; }
}
bool ngramStore::exists(ngram_t ngram) {
	createTable(ngram.order());
	string sql = "SELECT COUNT(1) FROM " + table(ngram.order())
		+ " WHERE " + where(ngram.order());
	Statement statement{_db, sql};
	bind(statement, ngram);

	int rc = statement.step();
	if(rc != SQLITE_ROW) { throw rc; }

	return (statement.getInteger(0) > 0);
}

string ngramStore::table(int order) {
	return _baseTableName + "_" + to_string(order);
}
string ngramStore::column(int p) {
	return "s_" + to_string(p);
}
string ngramStore::where(int order) {
	vector<string> clauses;
	for(int i = 0; i < order; ++i)
		clauses.push_back(" (" + column(i) + " = ? )");
	clauses.push_back(" (atom = ?)");
	return util::join(clauses, " AND ");
}
void ngramStore::bind(Statement &statement, ngram_t &ngram) {
	for(int i = 0; i < ngram.order(); ++i)
		statement.bind(i + 1, ngram.prefix[i]);
	statement.bind(ngram.order() + 1, ngram.atom);
}
string ngramStore::qmarks(int order) {
	vector<string> qms{order + 1, "?"}; // include atom
	return util::join(qms, ", ");
}
void ngramStore::createTable(int order) {
	string sql = "CREATE TABLE IF NOT EXISTS " + table(order);
	vector<string> columns;
	for(int i = 0; i < order; ++i)
		columns.push_back(column(i) + " int");
	columns.push_back("atom int");
	columns.push_back("count int");
	sql += "( " + util::join(columns, ", ") + " )";

	Statement statement{_db, sql};
	int rc = statement.step();
	if(rc != SQLITE_DONE) { throw rc; }
}

