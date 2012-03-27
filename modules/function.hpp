#ifndef MODULES_FUNCTION_HPP
#define MODULES_FUNCTION_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <boost/regex.hpp>

// Structure used to pass relavent data to Functions
struct FunctionArguments {
	boost::smatch matches;
	std::string nick;
	std::string user;
	std::string target;
	std::string message;
	bool toUs;
	bool fromOwner;

	std::map<std::string, int> *siMap;

	FunctionArguments() :
			matches(), nick(), user(), target(), message(), toUs(false),
			fromOwner(false), siMap(NULL) {
	}

	FunctionArguments(FunctionArguments &rhs) :
			matches(rhs.matches), nick(rhs.nick), user(rhs.user),
			target(rhs.target), message(rhs.message), toUs(rhs.toUs),
			fromOwner(rhs.fromOwner), siMap(rhs.siMap) {
	}

	private:
		FunctionArguments &operator=(FunctionArguments &rhs);
};

// Base class for all other functions
class Function {
	public:
		virtual ~Function() {}

		virtual std::string run(FunctionArguments fargs) {
			return "";
		}

		virtual std::string name() const {
			return "Base function";
		}
		virtual std::string help() const {
			return "Base function; cannot be invoked";
		}
		virtual std::string regex() const {
			return "^$";
		}
		virtual void reset() {
		}

		friend std::ostream& operator<<(std::ostream &out, Function &function) {
			out << (unsigned char)0x00;
			return out;
		}
		friend std::istream& operator>>(std::istream &in, Function &function) {
			unsigned char null;
			in >> null;
			return in;
		}
};

#endif // MODULES_FUNCTION_HPP
