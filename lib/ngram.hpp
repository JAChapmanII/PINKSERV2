#ifndef NGRAM_HPP
#define NGRAM_HPP

#include <vector>
#include <string>
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

struct ngramStore {
	ngramStore(db::Database &db, std::string baseTableName);

	chain_t fetch(ngram_t ngram);
	void increment(ngram_t ngram);
	bool exists(ngram_t ngram);

	private:
		std::string table(int order);
		std::string column(int p);
		std::string where(int order);
		void bind(db::Statement &statement, ngram_t &ngram);
		std::string qmarks(int order);

		void createTable(int order);

	private:
		db::Database &_db;
		std::string _baseTableName;
};

#endif // NGRAM_HPP
