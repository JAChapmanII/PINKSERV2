#ifndef TODO_HPP
#define TODO_HPP

#include <string>
#include "db.hpp"
#include "journal.hpp"

struct TODOs {
	TODOs(Journal &journal, zidcu::Database &db, std::string table = "todos");

	bool push(std::string text);

	sqlite_int64 unresolvedCount();

	void createTables();

	private:
		Journal &_journal;
		zidcu::Database &_db;
		std::string _table;
		bool _tablesCreated{false};
};

#endif // TODO_HPP
