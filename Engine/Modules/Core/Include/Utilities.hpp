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


#define ALL_MODULES					\
X(LOG)								\
X(DAFT_PUNK)		/* Audio */     \
X(DAVINCI)			/* Window + Rendering */	\
X(FILE_WIZARD)		/* File IO */	\
X(PACKET_NINJA)		/* */			\

enum class E_Modules : uMint {
#define X(name) name,
    ALL_MODULES
#undef X
};


/// @brief Get the string_view of a module
/// @param p_ModuleID which module? "DAFT_PUNK", "DAVINCI", "FILE_WIZARD", etc... 
/// @return std::string_view of module
constexpr std::string ModuleNameToString(E_Modules p_ModuleID)
{

    switch (p_ModuleID)
    {
#define X(name) case E_Modules::name: return #name;
        ALL_MODULES
#undef X

    default:
        return "INVALID_MODULE";
    }
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -