#include "brain.hpp"
using std::ostream;
using std::istream;
using std::string;

#include <arpa/inet.h>

ostream &brain::write(ostream &out, unsigned &variable) {
	uint32_t no = htonl(variable);
	unsigned char *noc = (unsigned char *)&no;
	for(int i = 0; i < 4; ++i)
		out << noc[i];
	return out;
}
istream &brain::read(istream &in, unsigned &variable) {
	unsigned char noc[4];
	for(int i = 0; i < 4; ++i)
		noc[i] = in.get();
	uint32_t no = *(uint32_t *)noc;
	variable = ntohl(no);
	return in;
}

ostream &brain::write(ostream &out, const string &variable) {
	unsigned char length = variable.length();
	out << length;
	for(int i = 0; i < length; ++i)
		out << variable[i];
	return out;
}
istream &brain::read(istream &in, string &variable) {
	int length = in.get();
	variable = "";
	if(length < 0)
		return in;
	for(int i = 0; i < length; ++i) {
		int c = in.get();
		if(c != -1)
			variable += (char)c;
	}
	return in;
}

ostream &brain::write(ostream &out, ChatLine &variable) {
	write(out, variable.nick);
	write(out, variable.target);
	write(out, variable.text);
	out << (unsigned char)variable.real;
	return out;
}
istream &brain::read(istream &in, ChatLine &variable) {
	read(in, variable.nick);
	read(in, variable.target);
	read(in, variable.text);
	variable.real = in.get();
	return in;
}

