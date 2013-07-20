#include "brain.hpp"
using std::ostream;
using std::istream;
using std::string;

// TODO: this is all very brittle
ostream &brain::write(ostream &out, uint8_t *bstream, size_t length) {
	for(size_t i = 0; i < length; ++i)
		out << (unsigned char)bstream[i];
	return out;
}
istream &brain::read(istream &in, uint8_t *bstream, size_t length) {
	for(size_t i = 0; i < length; ++i)
		bstream[i] = (uint8_t)in.get();
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

ostream &brain::write(ostream &out, const Variable &variable) {
	switch(variable.type) {
		case Type::String:
			write(out, 1);
			write(out, variable.value.s);
			break;
		case Type::Double:
			write(out, 2);
			write(out, variable.value.d);
			break;
		case Type::Integer:
			write(out, 3);
			write(out, variable.value.l);
			break;
		case Type::Boolean:
			write(out, 4);
			write(out, variable.value.b);
			break;
		default:
			throw (string)"unable to write unknown type";
	}
	return out;
}
istream &brain::read(istream &in, Variable &variable) {
	unsigned type = 0;
	read(in, type);
	switch(type) {
		case 1:
			variable.type = Type::String;
			read(in, variable.value.s);
			break;
		case 2:
			variable.type = Type::Double;
			read(in, variable.value.d);
			break;
		case 3:
			variable.type = Type::Integer;
			read(in, variable.value.l);
			break;
		case 4:
			variable.type = Type::Boolean;
			read(in, variable.value.b);
			break;
		default:
			throw (string)"unable to read unknown type";
	}
	return in;
}

ostream &brain::write(ostream &out, const Event &variable) {
	write(out, variable.body);
	return out;
}
istream &brain::read(istream &in, Event &variable) {
	read(in, variable.body);
	return in;
}

