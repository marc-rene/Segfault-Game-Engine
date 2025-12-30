#pragma once

#define USE_SEGFAULT_TYPES

#ifdef USE_SEGFAULT_TYPES

#define mint		__int8      // "Mini" integer
#define uMint		uint8_t
//#define short		__int16     // Ensure all shorts are the same, regardless of compiler
//#define uShort		uint16_t
//#define int         __int32
//#define uInt		uint32_t
#define fatty 		__int64     // BIG
#define uFatty 		uint64_t
#define mutex_lock	std::lock_guard<std::mutex>
#define BIT(x)		(1 << x)

#endif