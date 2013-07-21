#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <string>
#include <vector>
#include <map>
#include "permission.hpp"

enum Type { String, Double, Integer, Boolean };

// TODO: optimize storage of this/union?
struct Value {
	std::string s;
	bool b;
	double d;
	long l;

	Value() : s(), b(false), d(0), l(0) {
	}
};

class Variable {
	public:
		Variable();
		Variable(std::string ivalue, Permissions ip);
		Variable(bool ivalue, Permissions ip);
		Variable(double ivalue, Permissions ip);
		Variable(long ivalue, Permissions ip);
		Variable(const Variable &rhs);

		Variable asString() const;
		Variable asDouble() const;
		Variable asInteger() const;
		Variable asBoolean() const;

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

		static bool areOf(const Variable &v1, const Variable &v2, Type t1, Type t2);
		static bool eitherIs(const Variable &v1, const Variable &v2, Type t);
		static bool bothAre(const Variable &v1, const Variable &v2, Type t);

		static Variable parse(const std::string &rhs);

		Value value;
		Permissions permissions;
		Type type;
};

std::vector<std::string> getList(std::map<std::string, Variable> vars, std::string variable);
std::vector<std::string> makeList(std::string lists);

#endif // VARIABLE_HPP
