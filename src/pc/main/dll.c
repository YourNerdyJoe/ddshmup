#pragma warning(disable : 5105)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <assert.h>
#include "dll.h"
#include <string.h>

static inline char* stpcpy(char* dst, const char* src) {
	while(*src) *dst++ = *src++;
	*dst = 0;
	return dst;
}

static inline void* mempcpy(void* dst, const void* src, size_t n) {
	return (u8*)memcpy(dst, src, n) + n;
}

Dll* dllLoad(const char* dll_name) {
	size_t dll_name_len = strlen(dll_name);

	char* dll_path = _alloca(dll_name_len + 5);
	strcpy(mempcpy(dll_path, dll_name, dll_name_len), ".dll");

#ifndef RELEASE_BUILD
	char* dll_copy_path = _alloca(dll_name_len + 10);
	strcpy(mempcpy(dll_copy_path, dll_name, dll_name_len), "_copy.dll");

	CopyFileA(dll_path, dll_copy_path, FALSE);
	dll_path = dll_copy_path;
#endif
	void* dll = LoadLibraryA(dll_path);
	assert(dll != NULL);
	return dll;
}

void dllUnload(Dll* dll) {
	FreeLibrary((HMODULE)dll);
}

u64 getFileModifiedTime(const char* filename) {
	union {
		u64 timestamp;
		FILETIME filetime;
	} modified;
	HANDLE f = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, NULL);
	GetFileTime(f, NULL, NULL, &modified.filetime);
	CloseHandle(f);
	return modified.timestamp;
}

FnPtr dllGetFunc(Dll* dll, const char* name) {
	return (FnPtr)GetProcAddress((HMODULE)dll, name);
}

void dllGetFuncs(Dll* dll, FnPtr fns[], const char* names[], size_t count) {
	for(size_t i = 0; i < count; i++) {
		fns[i] = dllGetFunc(dll, names[i]);
	}
}
