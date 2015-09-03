#include "err.hpp"

namespace zidcu {
	template<typename T, typename... Ts>
			std::experimental::optional<T> Database::executeScalar(std::string sql, Ts... args) {
		auto &statement = (*this)[sql];
		statement.bindAll(args...);
		return statement.executeScalar<T>();
	}
	template<typename... Ts>
			void Database::executeVoid(std::string sql, Ts... args) {
		auto &statement = (*this)[sql];
		statement.bindAll(args...);
		statement.executeVoid();
	}

	template<typename... T> void Statement::bindAll(T... args) {
		this->bindAll<0>(args...);
	}
	template<int idx, typename T>
			void Statement::bindAll(T value) {
		this->bind(idx + 1, value);
	}
	template<int idx, typename T1, typename... Ts>
			void Statement::bindAll(T1 arg1, Ts... args) {
		this->bindAll<idx>(arg1);
		this->bindAll<idx + 1>(args...);
	}
	template<int> void Statement::bindAll() { }

	template<typename T> std::experimental::optional<T> Statement::executeScalar() {
		auto result = this->execute();
		if(result.status() == SQLITE_DONE)
			return std::experimental::optional<T>{};
		if(result.status() == SQLITE_ROW)
			return std::experimental::optional<T>{result.get<T>(0)};

		throw make_except("expected row not: " + std::to_string(result.status()));
	}

	template<typename T> T Result::get(int) {
		throw make_except("unexpected type");
	}
}

