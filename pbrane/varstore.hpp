#ifndef VARSTORE_HPP
#define VARSTORE_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include "db.hpp"
#include "variable.hpp"
#include "permission.hpp"

struct VarStore {
	virtual ~VarStore() { }

	virtual Variable get(std::string name) = 0;
	virtual Variable set(std::string name, Variable var) = 0;

	virtual bool defined(std::string name) = 0;
	virtual void erase(std::string name) = 0;

	virtual std::vector<Variable> getList(std::string variable) = 0;

	virtual std::vector<std::string> get() = 0;
	virtual std::vector<std::string> getVariablesOfType(Type type) = 0;
};

struct SqlVarStore : public VarStore {
	SqlVarStore(zidcu::Database &db, std::string varTableName = "vars");

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

struct LocalVarStore : public VarStore {
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

struct TransactionalVarStore : public VarStore {
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
		std::vector<std::string> getLocal();

	private:
		VarStore &_store;
		LocalVarStore _lstore{};
		std::set<std::string> _erased{};
		bool _commit{true};
};

#endif // VARSTORE_HPP
