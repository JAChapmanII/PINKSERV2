#ifndef NGRAM_HPP
#define NGRAM_HPP

#include <vector>
#include <string>
#include <map>
#include "db.hpp"

using word_t = unsigned;
using count_t = unsigned;
using prefix_t = std::vector<word_t>;

struct ngram_t {
	ngram_t(prefix_t prefix, word_t atom);

	prefix_t prefix;
	word_t atom;

	int order() const;
};

struct chain_t {
	chain_t(ngram_t ngram, count_t count = 0);

	ngram_t ngram;
	count_t count;
};

struct StatementCache {
	StatementCache(zidcu::Database &db);
	~StatementCache();

	zidcu::Statement &operator[](std::string sql);

	private:
		zidcu::Database &_db;
		std::map<std::string, zidcu::Statement *> _cache{};
};

struct ngramStoreStatementBuilder {
	ngramStoreStatementBuilder(std::string baseTableName);

	std::string table(int order) const;
	std::string column(int p) const;
	std::string where(int order) const;
	std::string qmarks(int order) const;

	std::string createTable(int order) const;
	std::string createIndex(int order) const;
	std::string ngramExists(int order) const;
	std::string ngramFetch(int order) const;
	std::string ngramInsert(int order) const;
	std::string ngramIncrement(int order) const;
	std::string random(int order) const;

	private:
		std::string _baseTableName{};
};

struct ngramStatementCache {
	ngramStatementCache(zidcu::Database &db, ngramStoreStatementBuilder builder);

	zidcu::Statement &createTable(int order);
	zidcu::Statement &createIndex(int order);
	zidcu::Statement &ngramExists(int order);
	zidcu::Statement &ngramFetch(int order);
	zidcu::Statement &ngramInsert(int order);
	zidcu::Statement &ngramIncrement(int order);
	zidcu::Statement &random(int order);

	private:
		ngramStoreStatementBuilder _builder;
		std::map<int, std::string> _tableCache{};
		std::map<int, std::string> _indexCache{};
		std::map<int, std::string> _existsCache{};
		std::map<int, std::string> _fetchCache{};
		std::map<int, std::string> _insertCache{};
		std::map<int, std::string> _incrementCache{};
		std::map<int, std::string> _randomCache{};
		StatementCache _cache;
};

struct ngramTableCache {
	ngramTableCache(ngramStatementCache &cache);

	bool exists(int order);

	private:
		ngramStatementCache &_cache;
		std::map<int, bool> _exists{};
};

struct ngramStore {
	ngramStore(zidcu::Database &db, std::string baseTableName);

	chain_t fetch(ngram_t ngram);
	void increment(ngram_t ngram);
	bool exists(ngram_t ngram);

	word_t random(prefix_t prefix);

	private:
		void bind(zidcu::Statement &statement, ngram_t &ngram);
		void bind(zidcu::Statement &statement, prefix_t &prefix);

	private:
		zidcu::Database &_db;
		std::string _baseTableName;
		ngramStoreStatementBuilder _builder;
		ngramStatementCache _cache;
		ngramTableCache _tableCache;
};

#endif // NGRAM_HPP
