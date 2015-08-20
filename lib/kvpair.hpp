#ifndef KVPAIR_HPP
#define KVPAIR_HPP

#include <string>
#include "db.hpp"

struct KVPair {
	std::string key{};
	std::string value{};
};

struct KVPairDB {
	KVPairDB(std::string idb, std::string itable);
	~KVPairDB();

	bool exists(std::string key);
	std::string *getValue(std::string key);
	void insertPair(KVPair pair);
	void setPair(KVPair pair);

	KVPairDB(const KVPairDB &rhs) = delete;
	KVPairDB *operator=(KVPairDB &rhs) = delete;

	protected:
		std::string _table;
		db::Database _db;
};


#endif // KVPAIR_HPP
