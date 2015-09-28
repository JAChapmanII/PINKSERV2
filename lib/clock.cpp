#include "clock.hpp"

#include <ctime>

sqlite_int64 Clock::now() {
	return time(NULL); // TODO: fix
}

