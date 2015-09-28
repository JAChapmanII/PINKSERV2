#ifndef ERR_HPP
#define ERR_HPP

#define MAX_BACKTRACE_DEPTH 128
#define make_except(what) err::ex{what, __FILE__, __LINE__};

#include <string>
#include <stdexcept>

namespace err {
	struct ex : std::runtime_error {
		ex(std::string what, std::string file, long line);
		ex(const ex &rhs);
		~ex();

		ex &operator=(const ex &rhs) = delete;

		std::string toString() const;
		void printBackTrace() const; // prints to cerr

		virtual const char* what() const noexcept;

		protected:
			void genWhatC();

		protected:
			std::string _what;
			std::string _file;
			long _line;

			size_t _backtraceSize{0};
			void *_backtrace[MAX_BACKTRACE_DEPTH]{};

			char *_whatC{nullptr};
	};
}


#endif // ERR_HPP
