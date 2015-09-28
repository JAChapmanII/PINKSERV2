#include "variable.hpp"
using std::string;
using std::vector;
using std::map;

#include <algorithm>
using std::transform;
using std::max;

#include <limits>
using std::numeric_limits;

#include <cmath>

#include "util.hpp"
using util::split;
using util::trimWhitespace;
using util::fromString;
using util::endsWith;

bool doubleIsZero(double d);
bool doubleIsZero(double d) { return abs(d) < numeric_limits<double>::epsilon(); }

Variable coerce(const Variable &v, Type t);
Variable coerce(const Variable &v, Type t) {
	switch(t) {
		case Type::Integer:
			return v.asInteger();
		case Type::Double:
			return v.asDouble();
		case Type::Boolean:
			return v.asBoolean();
		case Type::String:
			return v.asString();
		default: throw (string)"cannot coerce to that";
	}
}

vector<string> makeList(string lists) {
	vector<string> list = split(lists, ",");
	transform(list.begin(), list.end(), list.begin(), trimWhitespace);
	return list;
}

Variable::Variable() : value(), permissions(), type(Type::String) {
}
Variable::Variable(const char *ivalue, Permissions ip) :
		value(), permissions(ip), type(Type::String) {
	value.s = ivalue;
}
Variable::Variable(string ivalue, Permissions ip) :
		value(), permissions(ip), type(Type::String) {
	value.s = ivalue;
}
Variable::Variable(bool ivalue, Permissions ip) :
		value(), permissions(ip), type(Type::Boolean) {
	value.b = ivalue;
}
Variable::Variable(double ivalue, Permissions ip) :
		value(), permissions(ip), type(Type::Double) {
	value.d = ivalue;
}
Variable::Variable(long ivalue, Permissions ip) :
		value(), permissions(ip), type(Type::Integer) {
	value.l = ivalue;
}
Variable::Variable(const Variable &rhs) :
		value(rhs.value), permissions(rhs.permissions), type(rhs.type) {
}

Variable Variable::asString() const {
	return Variable(this->toString(), this->permissions);
}
Variable Variable::asDouble() const {
	switch(this->type) {
		case Type::Boolean:
			return Variable(this->value.b ? 1.0 : 0.0, this->permissions);
		case Type::Integer:
			return Variable((double)this->value.l, this->permissions);
		case Type::String:
			return Variable(fromString<double>(this->value.s), this->permissions);
		case Type::Double:
		default:
			return Variable(*this);
	}
}
Variable Variable::asInteger() const {
	switch(this->type) {
		case Type::Boolean:
			return Variable(this->value.b ? 1L : 0L, this->permissions);
		case Type::Double:
			return Variable((long)this->value.d, this->permissions);
		case Type::String:
			return Variable(fromString<long>(this->value.s), this->permissions);
		case Type::Integer:
		default:
			return Variable(*this);
	}
}
Variable Variable::asBoolean() const {
	switch(this->type) {
		case Type::Integer:
			return Variable(this->value.l == 0 ? false : true, this->permissions);
		case Type::Double:
			return Variable(doubleIsZero(this->value.d) ? false : true, this->permissions);
		case Type::String:
			if(this->value.s == "false" || this->value.s == "0" || this->value.s.empty())
				return Variable(false, this->permissions);
			return Variable(true, this->permissions);
		case Type::Boolean:
		default:
			return Variable(*this);
	}
}

string Variable::toString() const {
	switch(this->type) {
		case Type::Boolean:
			return (this->value.b ? "true" : "false");
		case Type::Double:
			return util::asString(this->value.d);
		case Type::Integer:
			return util::asString(this->value.l);
		case Type::String:
		default:
			return this->value.s;
	}
}

bool Variable::areOf(const Variable &v1, const Variable &v2, Type t1, Type t2) {
	if(v1.type == t1 && v2.type == t2)
		return true;
	if(v1.type == t2 && v2.type == t1)
		return true;
	return false;
}
bool Variable::eitherIs(const Variable &v1, const Variable &v2, Type t) {
	if(v1.type == t || v2.type == t)
		return true;
	return false;
}
bool Variable::bothAre(const Variable &v1, const Variable &v2, Type t) {
	if(v1.type == t && v2.type == t)
		return true;
	return false;
}

Variable Variable::operator+(const Variable &rhs) const {
	if(this->type == rhs.type) {
		switch(this->type) {
			case Type::Boolean: return (this->asInteger() + rhs.asInteger()).asBoolean();
			case Type::Integer: return Variable(this->value.l + rhs.value.l, this->permissions);
			case Type::Double: return Variable(this->value.d + rhs.value.d, this->permissions);
			case Type::String: return Variable(this->value.s + rhs.value.s, this->permissions);
			default:
				throw (string)"+ with same types, but unknown types?";
		}
	}
	if(areOf(*this, rhs, Type::Double, Type::Integer))
		return this->asDouble() + rhs.asDouble();
	if(eitherIs(*this, rhs, Type::String))
		return this->asString() + rhs.asString();
	if(this->type == Type::Boolean)
		return coerce(*this, rhs.type) + rhs;
	if(rhs.type == Type::Boolean)
		return coerce(rhs, this->type) + *this;
	throw (string)"+ not implemented on these types";
}
Variable Variable::operator*(const Variable &rhs) const {
	if(this->type == rhs.type) {
		switch(this->type) {
			case Type::Boolean: return (this->asInteger() * rhs.asInteger()).asBoolean();
			case Type::Integer: return Variable(this->value.l * rhs.value.l, this->permissions);
			case Type::Double: return Variable(this->value.d * rhs.value.d, this->permissions);
			case Type::String:
					throw (string)"cannot multiply strings";
			default: break;
		}
	}
	if(areOf(*this, rhs, Type::Double, Type::Integer))
		return this->asDouble() * rhs.asDouble();
	if(areOf(*this, rhs, Type::String, Type::Integer)) {
		string ret;
		// TODO:
		long times = (rhs.value.l > 512) ? 512 : rhs.value.l;
		ret.reserve(times * this->value.s.length());
		for(long i = 0; i < times; ++i)
			ret += this->value.s;
		return Variable(ret, this->permissions);
	}
	throw (string)"* not implemented on these types";
}
Variable Variable::operator-(const Variable &rhs) const {
	if(this->type == rhs.type) {
		switch(this->type) {
			// this is equivalent to (a and (not b))
			case Type::Boolean: return Variable(
					rhs.asBoolean().value.b ? false : this->asBoolean().value.b, this->permissions);
			case Type::Integer: return Variable(this->value.l - rhs.value.l, this->permissions);
			case Type::Double: return Variable(this->value.d - rhs.value.d, this->permissions);
			case Type::String: {
				string here = this->toString(), there = rhs.toString();
				if(endsWith(here, there))
					return Variable(here.substr(0, here.length() - there.length()), this->permissions);
				else
					return Variable(here, this->permissions);
					// TODO: throw instead?
					//throw (string)"-: this does not end with rhs";
			}
			default:
				throw (string)"- with same types, but unknown types?";
		}
	}
	if(areOf(*this, rhs, Type::Double, Type::Integer))
		return this->asDouble() - rhs.asDouble();
	if(this->type == Type::String && rhs.type == Type::Integer) {
		string here = this->toString();
		if(rhs.value.l < 0)
			throw (string)"cannot subtract negative length from string";
		if((size_t)rhs.value.l >= here.length())
			return Variable((string)"", this->permissions).asString();
		return Variable(here.substr(0, here.length() - rhs.value.l), this->permissions);
	}
	if(this->type == Type::Boolean)
		return coerce(*this, rhs.type) - rhs;
	if(rhs.type == Type::Boolean)
		return coerce(rhs, this->type) - *this;
	throw (string)"- not implemented on these types";
}
Variable Variable::operator/(const Variable &rhs) const {
	if(this->type == rhs.type) {
		switch(this->type) {
			case Type::Boolean: return Variable(
					!rhs.asBoolean().value.b || this->asBoolean().value.b, this->permissions);
			case Type::Integer:
				if(rhs.value.l == 0)
					throw (string)"error: division by zero";
				return Variable(this->value.l / rhs.value.l, this->permissions);
			case Type::Double:
				if(doubleIsZero(rhs.value.d))
					throw (string)"error: fdivision by zero";
				return Variable(this->value.d / rhs.value.d, this->permissions);
			case Type::String:
				throw (string)"cannot divide strings";
			default:
				throw (string)"- with same types, but unknown types?";
		}
	}
	if(areOf(*this, rhs, Type::Double, Type::Integer))
		return this->asDouble() / rhs.asDouble();

	throw (string)"/ not implemented on these types";
}
Variable Variable::operator%(const Variable &rhs) const {
	if(!bothAre(*this, rhs, Type::Integer))
		throw (string)"% is only implemented on integers";
	return Variable(this->value.l % rhs.value.l, this->permissions);
}

Variable Variable::operator&(const Variable &rhs) const {
	return Variable(this->asBoolean().value.b && rhs.asBoolean().value.b,
			this->permissions);
}
Variable Variable::operator|(const Variable &rhs) const {
	return Variable(this->asBoolean().value.b || rhs.asBoolean().value.b,
			this->permissions);
}

bool Variable::operator<(const Variable &rhs) const {
	if(this->type > rhs.type)
		return rhs < *this;
	if(this->type == rhs.type) {
		switch(this->type) {
			case Type::Boolean: return (this->asInteger().value.l < rhs.asInteger().value.l);
			case Type::Integer: return (this->value.l < rhs.value.l);
			case Type::Double: return (this->value.d < rhs.value.d);
			case Type::String: return (this->value.s < rhs.value.s);
			default:
				throw (string)"< with same types, but unknown types?";
		}
	}
	if(areOf(*this, rhs, Type::Double, Type::Integer))
		return this->asDouble() < rhs.asDouble();
	return this->asString() < rhs.asString();
	// TODO: ?
	//throw (string)"< not implemented on these types";
}
bool Variable::operator>(const Variable &rhs) const {
	if(this->type > rhs.type)
		return rhs > *this;
	if(this->type == rhs.type) {
		switch(this->type) {
			case Type::Boolean: return (this->asInteger().value.l > rhs.asInteger().value.l);
			case Type::Integer: return (this->value.l > rhs.value.l);
			case Type::Double: return (this->value.d > rhs.value.d);
			case Type::String: return (this->value.s > rhs.value.s);
			default:
				throw (string)"> with same types, but unknown types?";
		}
	}
	if(areOf(*this, rhs, Type::Double, Type::Integer))
		return this->asDouble() > rhs.asDouble();
	return this->asString() > rhs.asString();
	// TODO: ?
	//throw (string)"> not implemented on these types";
}
bool Variable::operator==(const Variable &rhs) const {
	// TODO: == vs === ? I don't think I like triple equals...
	if(this->type != rhs.type)
		return false;
	switch(this->type) {
		case Type::Boolean: return this->value.b == rhs.value.b;
		case Type::Integer: return this->value.l == rhs.value.l;
		case Type::Double: return doubleIsZero(this->value.d - rhs.value.d);
		case Type::String: return this->value.s == rhs.value.s;
		default:
			throw (string)"== messed up badly?";
	}
}

bool Variable::operator!=(const Variable &rhs) const {
	return !(*this == rhs);
}
bool Variable::operator<=(const Variable &rhs) const {
	return !(*this > rhs);
}
bool Variable::operator>=(const Variable &rhs) const {
	return !(*this < rhs);
}

Variable Variable::compare(const Variable &rhs, string comparison) const {
	if(comparison == "==") return Variable(*this == rhs, this->permissions);
	if(comparison == "!=") return Variable(*this != rhs, this->permissions);
	if(comparison == "<=") return Variable(*this <= rhs, this->permissions);
	if(comparison == ">=") return Variable(*this >= rhs, this->permissions);
	if(comparison == "<") return Variable(*this < rhs, this->permissions);
	if(comparison == ">") return Variable(*this > rhs, this->permissions);
	throw comparison + " is not valid comparison";
}

bool Variable::isTrue() const {
	return this->asBoolean().value.b;
}
bool Variable::isFalse() const {
	return !(this->asBoolean().value.b);
}

Variable &Variable::operator=(const std::string &rhs) {
	*this = this->asString();
	this->value.s = rhs;
	return *this;
}
Variable &Variable::operator+=(const string &rhs) {
	*this = this->asString();
	this->value.s += rhs;
	return *this;
}

Variable Variable::parse(const string &rhs) {
	if(rhs.empty())
		return Variable(rhs, Permissions());
	if(rhs == "true")
		return Variable(true, Permissions());
	if(rhs == "false")
		return Variable(false, Permissions());
	bool notInteger = false;
	for(char c : rhs)
		if(c < '0' || c > '9')
			notInteger = true;
	if(!notInteger)
		return Variable(fromString<long>(rhs), Permissions());
	bool notDouble = false;
	for(size_t i = 0; i < rhs.length(); ++i)
		if(i == 0 && rhs[i] == '-')
			continue;
		else if(!(rhs[i] == '.' || (rhs[i] >= '0' && rhs[i] <= '9')))
			notDouble = true;
	if(!notDouble)
		return Variable(fromString<double>(rhs), Permissions());
	return Variable(rhs, Permissions());
}

