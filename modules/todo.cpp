#include "todo.hpp"
using std::string;
using std::fstream;

#include <iostream>
using std::endl;

TodoFunction::TodoFunction(string todoName) : m_file() { // {{{
	this->m_file.open(todoName, fstream::app);
} // }}}
string TodoFunction::run(FunctionArguments fargs) { // {{{
	if(this->m_file.good()) {
		this->m_file << fargs.nick << ": " << fargs.matches[1] << endl;
		return fargs.nick + ": recorded";
	}
	return fargs.nick + ": error: file error";
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

