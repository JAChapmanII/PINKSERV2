#ifndef BRAIN_HPP
#define BRAIN_HPP

#include <fstream>
#include <string>
#include <map>
#include <list>
#include <utility>
#include "global.hpp"

namespace brain {
	std::ostream &write(std::ostream &out, unsigned &variable);
	std::istream &read(std::istream &in, unsigned &variable);

	std::ostream &write(std::ostream &out, const std::string &variable);
	std::istream &read(std::istream &in, std::string &variable);

	std::ostream &write(std::ostream &out, global::ChatLine &variable);
	std::istream &read(std::istream &in, global::ChatLine &variable);

	template<typename K, typename V> std::ostream &write(
			std::ostream &out, std::map<K, V> &variable);
	template<typename K, typename V> std::istream &read(
			std::istream &in, std::map<K, V> &variable);

	template<typename K, typename V> std::ostream &write(
			std::ostream &out, std::vector<std::pair<K, V>> &variable);
	template<typename K, typename V> std::istream &read(
			std::istream &in, std::vector<std::pair<K, V>> &variable);

	/*
	template<typename T> std::ostream &write(std::ostream &out, T &variable);
	template<typename T> std::istream &read(std::istream &in, T &variable);
	*/
}

#include "brain.imp"

#endif // BRAIN_HPP
