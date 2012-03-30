#include "function.hpp"
using std::string;
using std::ostream;
using std::istream;
using boost::smatch;
using global::ChatLine;

Function::Function() : m_write(false) {
}
Function::~Function() {
}

string Function::run(ChatLine line, smatch matches) {
	return "";
}

string Function::secondary(ChatLine line) {
	return "";
}

string Function::passive(ChatLine line, bool handled) {
	return "";
}

string Function::name() const {
	return "Base function";
}
string Function::help() const {
	return "Base function; cannot be invoked";
}
string Function::regex() const {
	return "^$";
}

ostream &operator<<(ostream &out, Function &function) {
	if(!function.m_write)
		return out;
	return function.output(out);
}
istream &operator>>(istream &in, Function &function) {
	if(!function.m_write)
		return in;
	return function.input(in);
}

ostream &Function::output(ostream &out) {
	return out;
}
istream &Function::input(istream &in) {
	return in;
}

