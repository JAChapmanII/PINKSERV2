#ifndef DB_HPP
#define DB_HPP

#include <string>
#include <sqlite3.h>

namespace zidcu {
	struct Transaction;
	struct Statement;

	struct Database {
		Database();
		Database(std::string fileName);
		~Database();

		sqlite3 *getDB();

		Transaction transaction();

		Database(const Database &rhs) = delete;
		Database *operator=(const Database &rhs) = delete;

		void open(std::string fileName);

		private:
			bool _opened{false};
			std::string _fileName{};
			sqlite3 *_db{nullptr};
			Statement *_startTransaction{nullptr};
			Statement *_commitTransaction{nullptr};
	};

	struct Result;

	struct Statement {
		Statement(Database &db, std::string sql);
		~Statement();

		void bind(int idx, int val);
		void bind(int idx, std::string val);

		Result execute();

		Statement(const Statement &rhs) = delete;
		Statement *operator=(const Statement &rhs) = delete;

		std::string sql() const;
		sqlite3_stmt *getStatement();

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
		std::string getString(int idx);

		private:
			int _rc{SQLITE_ERROR};
			Statement &_statement;
	};

	struct Transaction {
		Transaction(Statement &start, Statement &end);
		~Transaction();

		private:
			Statement &_start;
			Statement &_end;
	};
}

#endif // DB_HPP
