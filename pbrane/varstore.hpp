#ifndef VARSTORE_HPP
#define VARSTORE_HPP

#include <string>
#include <vector>
#include "db.hpp"
#include "variable.hpp"
#include "permission.hpp"

struct VarStore {
	VarStore(zidcu::Database &db, std::string varTableName, std::string permTableName);

	Variable get(std::string name);
	std::string getString(std::string name);
	Variable set(std::string name, Variable var);
	Variable set(std::string name, std::string val);

	bool defined(std::string name);
	void erase(std::string name);

	std::vector<std::string> getList(std::string variable);

	private:
		void createTables();

	private:
		zidcu::Database &_db;
		std::string _varTable;
		std::string _permTable;
		bool _tablesCreated{false};
};

#endif // VARSTORE_HPP
