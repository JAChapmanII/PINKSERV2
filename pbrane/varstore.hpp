#ifndef VARSTORE_HPP
#define VARSTORE_HPP

#include <string>
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

	virtual std::set<std::string> get() = 0;
	virtual std::set<std::string> getVariablesOfType(Type type) = 0;
};

struct SqlVarStore : public VarStore {
	SqlVarStore(zidcu::Database &db, std::string varTableName = "vars");

	Variable get(std::string name);
	Variable set(std::string name, Variable var);

	bool defined(std::string name);
	void erase(std::string name);

	std::set<std::string> get();
	std::set<std::string> getVariablesOfType(Type type);

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

	std::set<std::string> get();
	std::set<std::string> getVariablesOfType(Type type);

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

	std::set<std::string> get();
	std::set<std::string> getVariablesOfType(Type type);

	void abort();

	private:
		std::set<std::string> getLocal();

	private:
		VarStore &_store;
		LocalVarStore _lstore{};
		std::set<std::string> _erased{};
		bool _commit{true};
};

#endif // VARSTORE_HPP
