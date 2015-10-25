#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <sqlite3.h>

struct Clock {
	static sqlite_int64 now();
};

#endif // CLOCK_HPP
