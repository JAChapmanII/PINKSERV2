#include "variable.hpp"
using std::string;
using std::to_string;
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

string typeToString(Type type) {
	switch(type) {
		case String: return "string";
		case Number: return "number";
		case Boolean: return "bool";
		case Function: return "function";
		case Void: return "void";
	}
	return "{invalid}";
}
Type typeFromString(string str) {
	if(str == "string") return Type::String;
	if(str == "number") return Type::Number;
	if(str == "bool") return Type::Boolean;
	if(str == "function") return Type::Function;
	return Type::Void;
}

bool numberIsZero(Number d);
bool numberIsZero(Number d) {
	return abs(d) < numeric_limits<Number>::epsilon();
}

Variable coerce(const Variable &v, Type t);
Variable coerce(const Variable &v, Type t) {
	switch(t) {
		case Type::Number:
			return v.asNumber();
		case Type::Boolean:
			return v.asBoolean();
		case Type::String:
			return v.asString();
		case Type::Function:
			return v.asFunction();
		default: throw (string)"cannot coerce to that";
	}
}

vector<string> makeList(string lists) {
	vector<string> list = split(lists, ",");
	transform(list.begin(), list.end(), list.begin(), trimWhitespace);
	return list;
}

Number Variable::toNumber() const {
	return fromString<Number>(this->value);
}

// TODO: perms
Variable::Variable() { }
Variable::Variable(bool ivalue)
	: type{Type::Boolean}, value{ivalue ? "true" : "false"} { }
Variable::Variable(long ivalue) : type{Type::Number}, value{to_string(ivalue)} { }
Variable::Variable(double ivalue) : type{Type::Number}, value{to_string(ivalue)} { }
Variable::Variable(string ivalue) : type{Type::String}, value{ivalue} { }
Variable::Variable(const Variable &rhs) : type{rhs.type}, value{rhs.value} { }

Variable Variable::asString() const {
	return Variable(this->toString());
}
Variable Variable::asNumber() const {
	switch(this->type) {
		case Type::Boolean:
			return Variable(this->value == "true" ? 1L : 0L);
		case Type::String:
			return Variable(fromString<double>(this->value));
		default:
			return Variable(*this);
	}
}
Variable Variable::asBoolean() const {
	switch(this->type) {
		case Type::Number:
			return Variable(numberIsZero(this->toNumber()) ? false : true);
		case Type::String:
			if(this->value == "false" || this->value == "0" || this->value.empty())
				return Variable(false);
			return Variable(true);
		case Type::Boolean:
		default:
			return Variable(*this);
	}
}
Variable Variable::asFunction() const {
	Variable ret(*this);
	ret.type = Type::Function;
	return ret;
}

string Variable::toString() const { return this->value; }

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
			case Type::Boolean: return (this->asNumber() + rhs.asNumber()).asBoolean();
			case Type::Number: return Variable(this->toNumber() + rhs.toNumber());
			case Type::String: return Variable(this->value + rhs.value);
			default:
				throw (string)"+ with same types, but unknown types?";
		}
	}
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
			case Type::Boolean: return (this->asNumber() * rhs.asNumber()).asBoolean();
			case Type::Number: return Variable(this->toNumber() * rhs.toNumber());
			case Type::String:
					throw (string)"cannot multiply strings";
			default: break;
		}
	}
	if(areOf(*this, rhs, Type::String, Type::Number)) {
		string ret;
		// TODO:
		long times = (rhs.toNumber() > 512) ? 512 : rhs.toNumber();
		ret.reserve(times * this->value.length());
		for(long i = 0; i < times; ++i)
			ret += this->value;
		return Variable(ret);
	}
	throw (string)"* not implemented on these types";
}
Variable Variable::operator-(const Variable &rhs) const {
	if(this->type == rhs.type) {
		switch(this->type) {
			// this is equivalent to (a and (not b))
			case Type::Boolean: return Variable(rhs.isTrue() ? false : this->isTrue());
			case Type::Number: return Variable(this->toNumber() - rhs.toNumber());
			case Type::String: {
				string here = this->toString(), there = rhs.toString();
				if(endsWith(here, there))
					return Variable(here.substr(0, here.length() - there.length()));
				else
					return Variable(here);
					// TODO: throw instead?
					//throw (string)"-: this does not end with rhs";
			}
			default:
				throw (string)"- with same types, but unknown types?";
		}
	}
	if(this->type == Type::String && rhs.type == Type::Number) {
		string here = this->toString();
		if(rhs.toNumber() < 0)
			throw (string)"cannot subtract negative length from string";
		if((size_t)rhs.toNumber() >= here.length())
			return Variable((string)"").asString();
		return Variable(here.substr(0, here.length() - rhs.toNumber()));
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
			case Type::Boolean: return Variable(!rhs.isTrue() || this->isTrue());
			case Type::Number:
				if(numberIsZero(rhs.toNumber()))
					throw (string)"error: division by zero";
				return Variable(this->toNumber() / rhs.toNumber());
			case Type::String:
				throw (string)"cannot divide strings";
			default:
				throw (string)"- with same types, but unknown types?";
		}
	}

	throw (string)"/ not implemented on these types";
}
Variable Variable::operator%(const Variable &rhs) const {
	if(!bothAre(*this, rhs, Type::Number))
		throw (string)"% is only implemented on integers";
	return Variable((long)this->toNumber() % (long)rhs.toNumber());
}

Variable Variable::operator&(const Variable &rhs) const {
	return Variable(this->isTrue() && rhs.isTrue());
}
Variable Variable::operator|(const Variable &rhs) const {
	return Variable(this->isTrue() || rhs.isTrue());
}

bool Variable::operator<(const Variable &rhs) const {
	if(this->type > rhs.type)
		return rhs < *this;
	if(this->type == rhs.type) {
		switch(this->type) {
			case Type::Boolean: return (this->asNumber().toNumber() < rhs.asNumber().toNumber());
			case Type::Number: return (this->toNumber() < rhs.toNumber());
			case Type::String: return (this->value < rhs.value);
			default:
				throw (string)"< with same types, but unknown types?";
		}
	}
	return this->asString() < rhs.asString();
	// TODO: ?
	//throw (string)"< not implemented on these types";
}
bool Variable::operator>(const Variable &rhs) const {
	if(this->type > rhs.type)
		return rhs > *this;
	if(this->type == rhs.type) {
		switch(this->type) {
			case Type::Boolean: return (this->asNumber().toNumber() > rhs.asNumber().toNumber());
			case Type::Number: return (this->toNumber() > rhs.toNumber());
			case Type::String: return (this->value > rhs.value);
			default:
				throw (string)"> with same types, but unknown types?";
		}
	}
	return this->asString() > rhs.asString();
	// TODO: ?
	//throw (string)"> not implemented on these types";
}
bool Variable::operator==(const Variable &rhs) const {
	// TODO: == vs === ? I don't think I like triple equals...
	if(this->type != rhs.type)
		return false;
	switch(this->type) {
		case Type::Boolean: return this->isTrue() == rhs.isTrue();
		case Type::Number: return numberIsZero(this->toNumber() - rhs.toNumber());
		case Type::String: return this->value == rhs.value;
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
	if(comparison == "==") return Variable(*this == rhs);
	if(comparison == "!=") return Variable(*this != rhs);
	if(comparison == "<=") return Variable(*this <= rhs);
	if(comparison == ">=") return Variable(*this >= rhs);
	if(comparison == "<") return Variable(*this < rhs);
	if(comparison == ">") return Variable(*this > rhs);
	throw comparison + " is not valid comparison";
}

bool Variable::isTrue() const {
	return (this->value == "true" || this->value == "1"); // TODO
}
bool Variable::isFalse() const {
	return !(this->isTrue());
}

// TODO: implement this as template based on construct/parse and copy?
Variable &Variable::operator=(const std::string &rhs) {
	this->type = Type::String;
	this->value = rhs;
	return *this;
}
Variable &Variable::operator+=(const string &rhs) {
	*this = this->asString();
	this->value += rhs;
	return *this;
}

Variable Variable::parse(const string &rhs) {
	if(rhs.empty())
		return Variable(rhs);
	if(rhs == "true")
		return Variable(true);
	if(rhs == "false")
		return Variable(false);
	if(rhs == "void")
		return Variable();
	bool notInteger = false;
	for(char c : rhs)
		if(c < '0' || c > '9')
			notInteger = true;
	if(!notInteger)
		return Variable(fromString<long>(rhs));
	bool notDouble = false;
	for(size_t i = 0; i < rhs.length(); ++i)
		if(i == 0 && rhs[i] == '-')
			continue;
		else if(!(rhs[i] == '.' || (rhs[i] >= '0' && rhs[i] <= '9')))
			notDouble = true;
	if(!notDouble)
		return Variable(fromString<double>(rhs));
	return Variable(rhs);
}

