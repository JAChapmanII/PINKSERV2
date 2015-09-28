#ifndef DB_HPP
#define DB_HPP

#include <string>
#include <unordered_map>
#include <experimental/optional>
#include <sqlite3.h>

namespace zidcu {
	struct Transaction;
	struct Statement;
	struct StatementCache;
	struct Result;

	struct Database {
		Database();
		Database(std::string fileName);
		~Database();

		sqlite3 *getDB();

		Transaction transaction();

		Database(const Database &rhs) = delete;
		Database *operator=(const Database &rhs) = delete;

		void open(std::string fileName);

		Statement &operator[](std::string sql);

		template<typename... Ts>
			Result execute(std::string sql, Ts... args);
		template<typename T, typename... Ts>
			std::experimental::optional<T> executeScalar(std::string sql, Ts... args);
		template<typename... Ts>
			void executeVoid(std::string sql, Ts... args);

		private:
			bool _opened{false};
			std::string _fileName{};
			sqlite3 *_db{nullptr};
			Statement *_startTransaction{nullptr};
			Statement *_commitTransaction{nullptr};
			StatementCache *_cache{nullptr};
	};

	struct Statement {
		Statement(Database &db, std::string sql);
		~Statement();

		void bind(int idx, int val);
		void bind(int idx, sqlite_int64 val);
		void bind(int idx, double val);
		void bind(int idx, const char *val);
		void bind(int idx, std::string val);
		template<typename T> void bind(int idx, T val);

		template<typename... T> void bindAll(T... args);

		Result execute();
		template<typename T> std::experimental::optional<T> executeScalar();
		void executeVoid();

		Statement(const Statement &rhs) = delete;
		Statement *operator=(const Statement &rhs) = delete;

		std::string sql() const;
		sqlite3_stmt *getStatement();

		private:
			template<int idx, typename T>
				void bindAll(T value);
			template<int idx, typename T1, typename... Ts>
				void bindAll(T1 arg1, Ts... args);
			template<int>
				void bindAll();

		private:
			Database &_db;
			std::string _sql{};
			sqlite3_stmt *_statement{nullptr};
	};

	struct Result {
		Result(Statement &statement);
		~Result();

		int status();
		int step();

		int getInteger(int idx);
		sqlite_int64 getLong(int idx);
		std::string getString(int idx);

		template<typename T> T get(int idx);

		private:
			int _rc{SQLITE_ERROR};
			Statement &_statement;
	};

	struct Transaction {
		Transaction(Database &db, Statement &start, Statement &end);
		~Transaction();

		private:
			Database &_db;
			Statement &_start;
			Statement &_end;
	};

	struct StatementCache {
		StatementCache(zidcu::Database &db);
		~StatementCache();

		zidcu::Statement &operator[](std::string sql);

		private:
			zidcu::Database &_db;
			std::unordered_map<std::string, zidcu::Statement *> _cache{};
	};
}

#include "db.imp.hpp"

#endif // DB_HPP
