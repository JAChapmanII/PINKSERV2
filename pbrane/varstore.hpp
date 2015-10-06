#ifndef VARSTORE_HPP
#define VARSTORE_HPP

#include <string>
#include <vector>
#include "db.hpp"
#include "variable.hpp"
#include "permission.hpp"

struct VarStore {
	VarStore(zidcu::Database &db, std::string varTableName = "vars",
			std::string permTableName = "var_perms");

	Variable get(std::string name);
	Variable set(std::string name, Variable var);

	bool defined(std::string name);
	void erase(std::string name);

	std::vector<Variable> getList(std::string variable);

	void markExecutable(std::string name, bool x = true);
	std::vector<std::string> getExecutable();

	private:
		void createTables();

	private:
		zidcu::Database &_db;
		std::string _varTable;
		std::string _permTable;
		bool _tablesCreated{false};
};

#endif // VARSTORE_HPP
