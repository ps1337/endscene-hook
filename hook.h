#ifndef HOOK_H
#define HOOK_H

#include <Windows.h>
#include <iostream>
#include <string>

void* WINAPI hookFn(char* hookedFn, char* hookFn, int copyBytesSize, unsigned char* backupBytes, std::string descr);
BOOL WINAPI restore(char* fn, unsigned char* writeBytes, int writeSize, std::string descr);

#endif