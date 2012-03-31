#include "function.hpp"
using std::string;
using std::ostream;
using std::istream;
using boost::smatch;
using global::ChatLine;

Function::Function() : m_write(false) {
}
Function::Function(bool write) : m_write(write) {
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
	unsigned char w = (unsigned char)function.m_write;
	out << w;
	if(function.m_write)
		return function.output(out);
	else
		return out;
}
istream &operator>>(istream &in, Function &function) {
	unsigned char r = (unsigned char)function.m_write;
	r = in.get();
	if(r)
		return function.input(in);
	else
		return in;
}

ostream &Function::output(ostream &out) {
	return out;
}
istream &Function::input(istream &in) {
	return in;
}

