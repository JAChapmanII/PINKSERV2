#include "function.hpp"
using std::string;
using std::ostream;
using std::istream;
using boost::smatch;

/*
	TODO: old defaults?
	m_name("Base function")
	m_help("Base function; cannot be invoked")
	m_regex("^$")
*/

Function::Function(string iname, string ihelp, string iregex) :
		m_write(false), m_name(iname), m_help(ihelp), m_regex(iregex) {
}
Function::Function(string iname, string ihelp, string iregex, bool write) :
		m_write(write), m_name(iname), m_help(ihelp), m_regex(iregex) {
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
	return this->m_name;
}
string Function::help() const {
	return this->m_help;
}
string Function::regex() const {
	return this->m_regex;
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

