#pragma once

#include <string_view>
#include <string>
#include <mutex>
#include <chrono>


// https://medium.com/@ryan_forrester_/using-switch-statements-with-strings-in-c-a-complete-guide-efa12f64a59d

/// Hash a String at Compile-time
/// @param string constant to hash
/// @return unsigned 64-bit int hash
constexpr unsigned long long hash(std::string_view string)
{
    unsigned long long hash = 0;
    for (char c : str)
    {
        hash = (hash * 131) + c;
    }
    return hash;
}


/// Convert a String constant "Hello" to an int-64 to use for switch cases
/// @param String constant for compile-time hash
/// @return Unsigned long long hash of the provided string
constexpr unsigned long long operator"" _hash(const char* string_constant, size_t len)
{
    return hash(std::string_view(string_constant, len));
}


// Macro to convert all our modules into a handy enum we can use later!
#define ALL_MODULES					                \
X(CORE)								                \
X(DAFT_PUNK)		      /* Audio */               \
X(DAVINCI)			/* Window + Rendering */	    \
X(NIT_PICK)		       /* ImGui Editor */	        \
X(FILE_WIZARD)		     /* File IO */	            \
X(PACKET_NINJA)		   /* Networking */             \
X(SWARM)		      /* EnTT Wrapper */	        \

enum class E_Modules : __int8
{
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



struct Local_Timer
{
    // Make a timer that we can get duration values for in MS
    Local_Timer()
        : start_time{std::chrono::steady_clock::now()}
    {
    }


    // Get elapsed time so far in miliseconds
    std::chrono::milliseconds Get_Current_Time()
    {
        const std::lock_guard<std::mutex> lock(timer_mutex);
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
    }


    // Get elapsed time so far in double float
    double Get_Current_Time_f()
    {
        const std::lock_guard<std::mutex> lock(timer_mutex);
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time).count();
    }

    
    // WARNING - Will reset the timer... use Get_Current_Time() instead if you ONLY want to get elapsed time in ms
    std::chrono::milliseconds Get_Final_Time()
    {
        const std::lock_guard<std::mutex> lock(timer_mutex);
        auto total = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time);
        start_time = std::chrono::steady_clock::now();
        return total;
    }

    
private:
    std::mutex timer_mutex;
    std::chrono::time_point<std::chrono::steady_clock> start_time;
};
