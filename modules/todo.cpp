#include "todo.hpp"
using std::string;
using std::fstream;
using boost::smatch;
using global::ChatLine;

#include <iostream>
using std::endl;

TodoFunction::TodoFunction(string todoName) : Function(), // {{{
		m_file() {
	this->m_file.open(todoName, fstream::app);
} // }}}
string TodoFunction::run(ChatLine line, smatch matches) { // {{{
	if(this->m_file.good()) {
		this->m_file << line.nick << ": " << matches[1] << endl;
		return line.nick + ": recorded";
	}
	return line.nick + ": error: file error";
} // }}}
string TodoFunction::name() const { // {{{
	return "todo";
} // }}}
string TodoFunction::help() const { // {{{
	return "Adds a string to the todo file.";
} // }}}
string TodoFunction::regex() const { // {{{
	return "^!todo\\s+(.*)";
} // }}}

