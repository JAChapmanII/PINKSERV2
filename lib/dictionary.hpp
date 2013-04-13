#ifndef DICTIONARY_HPP
#define DICTIONARY_HPP

#include <map>
#include <iostream>

static unsigned anchorCount = 2;
template<typename K, typename V> class Dictionary {
	public:
		enum Anchor { Start = 1, End, Invalid };
		Dictionary();

		V get(K key);
		V operator[](K key);

		K get(V value);
		K operator[](V value);

		std::istream &read(std::istream &in);
		std::ostream &write(std::ostream &out);

		unsigned size() const;

	protected:
		// TODO: minimal DAFSA for perfect hashing
		std::map<K, V> m_fmap;
		std::map<V, K> m_rmap;
};

#include "dictionary.imp"

#endif // DICTIONARY_HPP
