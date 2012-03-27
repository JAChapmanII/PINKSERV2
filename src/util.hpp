#ifndef UTIL_HPP
#define UTIL_HPP

#include <vector>
#include <string>
#include <map>

namespace util {
	std::vector<std::string> split(std::string str, std::string on = " \t\r\n");
	std::string join(std::vector<std::string> strs, std::string with = ", ");

	template<typename T>
			bool contains(std::vector<T> vec, T val);
	template<typename K, typename V>
			bool contains(std::map<K, V> map, K key);
}

#include "util.imp"

#endif // UTIL_HPP
