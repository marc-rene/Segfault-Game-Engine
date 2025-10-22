#pragma once

#include <string_view>
#include "TypeDefinitions.hpp"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// https://medium.com/@ryan_forrester_/using-switch-statements-with-strings-in-c-a-complete-guide-efa12f64a59d
constexpr uFatty hash(std::string_view str) {
	uFatty hash = 0;
	for (char c : str) {
		hash = (hash * 131) + c;
	}
	return hash;
}

constexpr uFatty operator"" _hash(const char* str, size_t len) {
	return hash(std::string_view(str, len));
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
