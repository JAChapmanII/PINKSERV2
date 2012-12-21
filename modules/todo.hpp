#ifndef MODULES_TODO_HPP
#define MODULES_TODO_HPP

#include "function.hpp"
#include <string>
#include <fstream>

// add something for me to do
class TodoFunction : public Function {
	public:
		TodoFunction(std::string todoName);
		virtual std::string run(ChatLine line, boost::smatch matches);

	protected:
		std::ofstream m_file;
};

#endif // MODULES_TODO_HPP
