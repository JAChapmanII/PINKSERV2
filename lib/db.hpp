#ifndef DB_HPP
#define DB_HPP

#include <string>
#include <sqlite3.h>

namespace db {
	struct Database {
		Database(std::string fileName);
		~Database();

		sqlite3 *getDB();

		Database(const Database &rhs) = delete;
		Database *operator=(const Database &rhs) = delete;

		private:
			std::string _fileName{};
			sqlite3 *_db{nullptr};
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
}

#endif // DB_HPP
