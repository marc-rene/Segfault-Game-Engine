#pragma once

#define USE_SEGFAULT_TYPES

#ifdef USE_SEGFAULT_TYPES


using   mint        =   __int8;      // "Mini" integer
using   uMint       =   unsigned char;

using   int_16  =   __int16;
using   uInt_16  =  unsigned __int16;

using   int_32  =   __int32;
using   uInt_32  =  unsigned __int32;

using   int_64  =   __int64;    
using   uInt_64 =   unsigned __int64;

#define BIT(x)		(1 << x)

#define __ADD_L_PREFIX(X) L##X  // Does nothing by itself

// Add the L prefix to a Compile-time constant to make it a wide string eg: L"Salut Pote :)"
#define TO_WIDE_STRING(X) __ADD_L_PREFIX(X)



#endif