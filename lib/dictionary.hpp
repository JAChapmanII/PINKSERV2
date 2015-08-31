#ifndef DICTIONARY_HPP
#define DICTIONARY_HPP

#include <string>
#include <sqlite3.h>
#include "db.hpp"

enum class Anchor { Start, End, Count, ReservedCount = 0x100 };

struct Dictionary {
	Dictionary(zidcu::Database &db, std::string tableName = "dictionary");

	std::string get(sqlite_uint64 key);
	std::string operator[](sqlite_uint64 key);

	sqlite_uint64 get(std::string value);
	sqlite_uint64 operator[](std::string value);

	sqlite_uint64 size();

	bool isAnchor(sqlite_uint64 key);
	bool isInvalidAnchor(sqlite_uint64 key);

	void createTable();

	private:
		void insert(std::string value);

	private:
		zidcu::Database &_db;
		std::string _tableName{};
		bool _createdTable{false};
		// TODO: minimal DAFSA for perfect hashing?
};

#endif // DICTIONARY_HPP
