#include "injectedThread.h"
#include "trampolines.h"
#include "d3d.h"
#include "hook.h"
#include <string>
#include <sstream>

endSceneFunc trampEndScene = nullptr;
unsigned char endSceneBytes[7];

void WINAPI injectedThread(HMODULE hModule) {
    MessageBox(0, "Hax Injected", ":)", 0);
    // wait for the game to be up and running
    Sleep(1000 * 10);

    // Get the function pointer from the dummy device object
    d3dHelper d3dHelper;
    char* ogEndSceneAddress = d3dHelper.d3d9DeviceTable[42];

    // hook EndScene() and retreive the trampoline
    trampEndScene = (endSceneFunc)hookFn(ogEndSceneAddress, (char*)d3dHelper.endSceneHook, 7, endSceneBytes, "EndScene");

    // Keep thread alive until "DELETE" was hit
    while (!GetAsyncKeyState(VK_DELETE)) {
        Sleep(1000);
    }

    // uninject and unhook
    restore(ogEndSceneAddress, endSceneBytes, 7, "EndScene");

    MessageBox(0, "BYE!", ":)", 0);
    FreeLibraryAndExitThread(hModule, 0);
}
