#ifndef NGRAM_HPP
#define NGRAM_HPP

#include <vector>
#include <string>
#include <map>
#include <sqlite3.h>
#include "db.hpp"

using word_t = sqlite3_int64;
using count_t = sqlite3_int64;
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

struct ngramStoreStatementBuilder {
	ngramStoreStatementBuilder(std::string baseTableName);

	std::string table(int order) const;
	std::string column(int p) const;
	std::string where(int order) const;
	std::string qmarks(int order) const;

	std::string createTable(int order) const;
	std::string createIndex1(int order) const;
	std::string createIndex2(int order) const;
	std::string ngramExists(int order) const;
	std::string ngramFetch(int order) const;
	std::string ngramInsert(int order) const;
	std::string ngramIncrement(int order) const;
	std::string prefixCount(int order) const;
	std::string prefixFetch(int order) const;

	private:
		std::string _baseTableName{};
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

		void createTable(int order);

	private:
		zidcu::Database &_db;
		std::string _baseTableName{};

		ngramStoreStatementBuilder _builder;
		std::vector<bool> _tableCache{};
};

#endif // NGRAM_HPP
