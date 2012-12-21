#include "todo.hpp"
using std::string;
using std::fstream;
using boost::smatch;

#include <iostream>
using std::endl;

TodoFunction::TodoFunction(string todoName) : // {{{
		Function("todo", "Adds a string to the todo file.", "^!todo\\s+(.*)"),
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

