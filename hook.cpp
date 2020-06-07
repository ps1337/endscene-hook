#include "hook.h"
#include <sstream> // debug

const char *REL_JMP = "\xE9";
const char *NOP = "\x90";
// 1 byte instruction +  4 bytes address
const unsigned int SIZE_OF_REL_JMP = 5;

// adapted from https://guidedhacking.com/threads/simple-x86-c-trampoline-hook.14188/
// hookedFn: The function that's about to the hooked
// hookFn: The function that will be executed before `hookedFn` by causing `hookFn` to take a detour
void *WINAPI hookFn(char *hookedFn, char *hookFn, int copyBytesSize, unsigned char *backupBytes, std::string descr)
{

    if (copyBytesSize < 5)
    {
        // the function prologue of the hooked function
        // should be of size 5 (or larger)
        return nullptr;
    }

    //
    // 1. Backup the original function prologue
    //
    if (!ReadProcessMemory(GetCurrentProcess(), hookedFn, backupBytes, copyBytesSize, 0))
    {
        MessageBox(0, std::string("[hookFn] Failed to Backup Original Bytes for " + descr).c_str(), ":(", 0);
        return nullptr;
    }

    //
    // 2. Setup the trampoline
    // --> Cause `hookedFn` to return to `hookFn` without causing an infinite loop
    // Otherwise calling `hookedFn` directly again would then call `hookFn` again, and so on :)
    //
    // allocate executable memory for the trampoline
    // the size is (amount of bytes copied from the original function) + (size of a relative jump + address)

    char *trampoline = (char *)VirtualAlloc(0, copyBytesSize + SIZE_OF_REL_JMP, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    // steal the first `copyBytesSize` bytes from the original function
    // these will be used to make the trampoline work
    // --> jump back to `hookedFn` without executing `hookFn` again
    memcpy(trampoline, hookedFn, copyBytesSize);
    // append the relative JMP instruction after the stolen instructions
    memcpy(trampoline + copyBytesSize, REL_JMP, sizeof(REL_JMP));

    // calculate the offset between the hooked function and the trampoline
    // --> distance between the trampoline and the original function `hookedFn`
    // this will land directly *after* the inserted JMP instruction, hence subtracting 5
    int hookedFnTrampolineOffset = hookedFn - trampoline - SIZE_OF_REL_JMP;
    memcpy(trampoline + copyBytesSize + 1, &hookedFnTrampolineOffset, sizeof(hookedFnTrampolineOffset));

    //
    // 3. Detour the original function `hookedFn`
    // --> cause `hookedFn` to execute `hookFn` first
    // remap the first few bytes of the original function as RXW
    DWORD oldProtect;
    if (!VirtualProtect(hookedFn, copyBytesSize, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        MessageBox(0, std::string("[hookFn] Failed to set RXW for " + descr).c_str(), ":(", 0);
        return nullptr;
    }

    // best variable name ever
    // calculate the size of the relative jump between the start of `hookedFn` and the start of `hookFn`.
    int hookedFnHookFnOffset = hookFn - hookedFn - SIZE_OF_REL_JMP;

    // Take a relative jump to `hookFn` at the beginning
    // of course, `hookFn` has to expect the same parameter types and values
    memcpy(hookedFn, REL_JMP, sizeof(REL_JMP));
    memcpy(hookedFn + 1, &hookedFnHookFnOffset, sizeof(hookedFnHookFnOffset));

    // restore the previous protection values
    if (!VirtualProtect(hookedFn, copyBytesSize, oldProtect, &oldProtect))
    {
        MessageBox(0, std::string("[hookFn] Failed to Restore Protection for " + descr).c_str(), ":(", 0);
    }

    return trampoline;
}

// Unhook le method
BOOL WINAPI restore(char *fn, unsigned char *writeBytes, int writeSize, std::string descr)
{
    DWORD oldProtect;
    if (!VirtualProtect(fn, writeSize, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        MessageBox(0, std::string("[restore] Failed to set RWX for " + descr).c_str(), ":(", 0);
        return FALSE;
    }

    if (!WriteProcessMemory(GetCurrentProcess(), fn, writeBytes, writeSize, 0))
    {
        MessageBox(0, std::string("[restore] Failed to Write Memory for " + descr).c_str(), ":(", 0);
        return FALSE;
    }

    if (!VirtualProtect(fn, writeSize, oldProtect, &oldProtect))
    {
        MessageBox(0, std::string("[restore] Failed to  Restore Protection for " + descr).c_str(), ":(", 0);
        return FALSE;
    }

    return TRUE;
}
