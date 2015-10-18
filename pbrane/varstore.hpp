#ifndef VARSTORE_HPP
#define VARSTORE_HPP

#include <string>
#include <vector>
#include <map>
#include "db.hpp"
#include "variable.hpp"
#include "permission.hpp"

struct VarStore {
	VarStore(zidcu::Database &db, std::string varTableName = "vars");

	Variable get(std::string name);
	Variable set(std::string name, Variable var);

	bool defined(std::string name);
	void erase(std::string name);

	std::vector<Variable> getList(std::string variable);

	std::vector<std::string> get();
	std::vector<std::string> getVariablesOfType(Type type);

	private:
		void createTables();

	private:
		zidcu::Database &_db;
		std::string _varTable;
		// TODO: permissions
		bool _tablesCreated{false};
};

struct LocalVarStore {
	LocalVarStore();

	Variable get(std::string name);
	Variable set(std::string name, Variable var);

	bool defined(std::string name);
	void erase(std::string name);

	std::vector<Variable> getList(std::string variable);

	std::vector<std::string> get();
	std::vector<std::string> getVariablesOfType(Type type);

	private:
		std::map<std::string, Variable> _vars{};
};

struct TransactionalVarStore {
	TransactionalVarStore(VarStore &store);
	~TransactionalVarStore();

	Variable get(std::string name);
	Variable set(std::string name, Variable var);

	bool defined(std::string name);
	void erase(std::string name);

	std::vector<Variable> getList(std::string variable);

	std::vector<std::string> get();
	std::vector<std::string> getVariablesOfType(Type type);

	void abort();

	private:
		VarStore &_store;
		LocalVarStore _lstore{};
		bool _commit{true};
};

#endif // VARSTORE_HPP
