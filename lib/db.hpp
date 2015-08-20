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

	struct Statement {
		Statement(Database &db, std::string sql);
		~Statement();

		void bind(int idx, int val);
		void bind(int idx, std::string val);

		int step();

		int getInteger(int idx);
		std::string getString(int idx);

		Statement(const Statement &rhs) = delete;
		Statement *operator=(const Statement &rhs) = delete;

		private:
			Database &_db;
			std::string _sql{};
			sqlite3_stmt *_statement{nullptr};
	};
}

#endif // DB_HPP
