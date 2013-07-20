#ifndef BRAIN_HPP
#define BRAIN_HPP

#include <fstream>
#include <string>
#include <map>
#include <list>
#include <utility>
#include <vector>
#include "variable.hpp"

namespace brain {
	std::ostream &write(std::ostream &out, uint8_t *bstream, size_t length);
	std::istream &read(std::istream &in, uint8_t *bsream, size_t length);

	std::ostream &write(std::ostream &out, const std::string &variable);
	std::istream &read(std::istream &in, std::string &variable);

	std::ostream &write(std::ostream &out, const Variable &variable);
	std::istream &read(std::istream &in, Variable &variable);

	template<typename K, typename V> std::ostream &write(
			std::ostream &out, std::map<K, V> &variable);
	template<typename K, typename V> std::istream &read(
			std::istream &in, std::map<K, V> &variable);

	template<typename K, typename V> std::ostream &write(
			std::ostream &out, std::vector<std::pair<K, V>> &variable);
	template<typename K, typename V> std::istream &read(
			std::istream &in, std::vector<std::pair<K, V>> &variable);

	template<typename T> std::ostream &write(std::ostream &out, T variable);
	template<typename T> std::istream &read(std::istream &in, T &variable);
}

#include "brain.imp"

#endif // BRAIN_HPP
