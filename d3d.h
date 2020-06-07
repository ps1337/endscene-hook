#ifndef D3D_H
#define D3D_H

#include "d3d9.h"
#include <stdexcept>

class d3dHelper {
public:
    d3dHelper();
    bool getD3D9Device();
    // The vTable entries of the D3D9 dummy device
    char* d3d9DeviceTable[119];

    static void drawRectangle(int x, int y, int h, int w, D3DCOLOR color);
    static void APIENTRY endSceneHook(LPDIRECT3DDEVICE9 pDevice);

    // Helpers
private:
    static BOOL CALLBACK enumGetProcessWindow(HWND _hwnd, LPARAM lParam);
};

#endif