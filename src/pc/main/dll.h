#pragma once
#include "../../types.h"

typedef void(*FnPtr)(void);

struct Dll;
typedef struct Dll Dll;

#ifdef __cplusplus
extern "C" {
#endif

Dll* dllLoad(const char* dll_name);
void dllUnload(Dll* dll);

u64 getFileModifiedTime(const char* filename);

FnPtr dllGetFunc(Dll* dll, const char* name);
void dllGetFuncs(Dll* dll, FnPtr fns[], const char* names[], size_t count);

#ifdef __cplusplus
}
#endif
