#pragma once
//#include <stdalign.h>
#include <stdint.h>

#ifndef __cplusplus
#include <stdbool.h>

#define alignof _Alignof
#define alignas _Alignas
#endif

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

typedef bool b8;
typedef u32  b32;

#define cast(type, v) (*(type*)&v)
#define arrlen(arr) (sizeof(arr)/sizeof(*(arr)))

#define for_each_arr(type, it, arr) for(type it = (arr); it < (arr) + arrlen(arr); it++)
#define for_each_arr_rev(type, it, arr) for(type it = (arr) + arrlen(arr) - 1; it >= (arr); it--)

//#include <stdlib.h>	//until rand is replaced
//static inline s32 min(s32 a, s32 b) { return (a < b) ? a : b; }
//static inline s32 max(s32 a, s32 b) { return (a > b) ? a : b; }
//static inline s32 clamp(s32 val, s32 min_val, s32 max_val) { return min(max(val, min_val), max_val); }
