#ifndef KVPAIR_HPP
#define KVPAIR_HPP

#include <string>
#include <sqlite3.h>

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
		std::string _dbFilename;
		std::string _table;
		sqlite3 *_db;
};


#endif // KVPAIR_HPP
