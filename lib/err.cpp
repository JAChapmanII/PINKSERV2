#include "err.hpp"
using std::runtime_error;
using std::string;

#include <sstream>
using std::stringstream;

#include <algorithm>
using std::copy;

#include <execinfo.h>
#include <cstdlib>
#include <unistd.h>

#include <iostream>
using std::cerr;
using std::endl;

namespace err {
	ex::ex(string what, string file, long line)
			: runtime_error{what}, _what{what}, _file{file}, _line{line} {
		_backtraceSize = backtrace(_backtrace, MAX_BACKTRACE_DEPTH);
		if(_backtraceSize == MAX_BACKTRACE_DEPTH)
			cerr << "warning: backtrace may have been truncated" << endl;
		this->genWhatC();
	}
	ex::ex(const ex &rhs) : runtime_error{rhs.what()}, _what{rhs._what},
			_file{rhs._file}, _line{rhs._line}, _backtraceSize{rhs._backtraceSize} {
		copy(rhs._backtrace, rhs._backtrace + rhs._backtraceSize, _backtrace);
		this->genWhatC();
	}
	ex::~ex() { if(_whatC) { delete[] _whatC; } }

	void ex::genWhatC() {
		if(_whatC) return;
		auto what = this->toString();
		_whatC = new char[what.size() + 1]{};
		copy(what.begin(), what.end(), _whatC);
	}

	string ex::toString() const {
		stringstream ss{};
		ss << "exception: " << _file << ":" << _line << ": " << _what << ":" << endl;

		char **symbols = backtrace_symbols(_backtrace, _backtraceSize);
		for(size_t i = 1; i < _backtraceSize; ++i) {
			ss << "err::ex\t" << string(symbols[i]) << endl;
		}
		free(symbols);

		return ss.str();
	}
	void ex::printBackTrace() const {
		// we skip the constructor for the ex class
		backtrace_symbols_fd(_backtrace + 1, _backtraceSize - 1, STDERR_FILENO);
	}
	const char* ex::what() const noexcept { return _whatC; }
}

