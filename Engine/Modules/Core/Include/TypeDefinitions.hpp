#pragma once

#define USE_SEGFAULT_TYPES

#ifdef USE_SEGFAULT_TYPES

#define mint		__int8      // "Mini" integer
#define uMint		uint8_t

#define fatty 		__int64     // BIG
#define uFatty 		uint64_t

#define BIT(x)		(1 << x)

#endif