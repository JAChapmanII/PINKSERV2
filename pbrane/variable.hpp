#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <string>
#include <vector>
#include <map>
#include "permission.hpp"

enum class Type { String, Number, Boolean, Function, Void };
using Value = std::string;
using Number = double;

std::string typeToString(Type type);
Type typeFromString(std::string str);

class Variable {
	public:
		Variable(); // void constructor
		//template<typename T> Variable(T ivalue);
		Variable(bool ivalue);
		Variable(long ivalue);
		Variable(double ivalue);
		Variable(std::string ivalue);
		// bool -> bool
		// numeric -> number
		// catch all -> string
		Variable(const Variable &rhs);

		// just swaps type tag?
		Variable asString() const;
		Variable asNumber() const;
		Variable asBoolean() const;
		Variable asFunction() const;

		std::string toString() const;

		Variable operator+(const Variable &rhs) const;
		Variable operator*(const Variable &rhs) const;
		Variable operator-(const Variable &rhs) const;
		Variable operator/(const Variable &rhs) const;
		Variable operator%(const Variable &rhs) const;

		Variable operator&(const Variable &rhs) const;
		Variable operator|(const Variable &rhs) const;

		bool operator<(const Variable &rhs) const;
		bool operator>(const Variable &rhs) const;
		bool operator==(const Variable &rhs) const;

		bool operator!=(const Variable &rhs) const;
		bool operator<=(const Variable &rhs) const;
		bool operator>=(const Variable &rhs) const;

		Variable compare(const Variable &rhs, std::string comparison) const;

		Variable &operator=(const std::string &rhs);
		Variable &operator+=(const std::string &rhs);

		bool isTrue() const;
		bool isFalse() const;

		double toNumber() const;

		static bool areOf(const Variable &v1, const Variable &v2, Type t1, Type t2);
		static bool eitherIs(const Variable &v1, const Variable &v2, Type t);
		static bool bothAre(const Variable &v1, const Variable &v2, Type t);

		static Variable parse(const std::string &rhs);

		Type type{Type::Void};
		Value value{};
		Permissions permissions{};
};

std::vector<std::string> makeList(std::string lists);

#endif // VARIABLE_HPP
