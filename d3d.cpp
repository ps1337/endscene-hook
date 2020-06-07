#include "d3d.h"
#include "trampolines.h"

// Handle to the CS:GO window
HWND hwnd = nullptr;

LPDIRECT3DDEVICE9 d3dDevice = NULL;

d3dHelper::d3dHelper() {
    // get the HWND for the CS:GO window
    EnumWindows(enumGetProcessWindow, GetCurrentProcessId());
    if (!hwnd) {
        MessageBox(0, "[D3D] HWND Not Found", ":(", 0);
        throw std::runtime_error("[D3D] HWND Not Found");
    }

    // obtain the address of hooked d3d functions by creating a
    // dummy device and grabbing the function vTable
    if (!this->getD3D9Device()) {
        MessageBox(0, "[D3D] D3D9 Device Error", ":(", 0);
        throw std::runtime_error("[D3D] D3D9 Device Error");
    }

}

void APIENTRY d3dHelper::endSceneHook(LPDIRECT3DDEVICE9 p_pDevice) {
    // save the parameter, since it' used in order to draw stuff
    if (!d3dDevice) {
        d3dDevice = p_pDevice;
    }

    int h = 16;
    int w = 16;
    drawRectangle(1920/2-(h/2), 1080/2-(w/2), h, w, D3DCOLOR_ARGB(100, 245, 125, 215));
    // call original function using the trampoline
    trampEndScene(d3dDevice);
}

//
// Helpers
//

void d3dHelper::drawRectangle(int x, int y, int h, int w, D3DCOLOR color) {
    D3DRECT r = { x, y, x + w, y + h };
    d3dDevice->Clear(1, &r, D3DCLEAR_TARGET, color, 0, 0);
}


// https://guidedhacking.com/threads/get-direct3d9-and-direct3d11-devices-dummy-device-method.11867/
// create the dummy d3d device and copy the object contents in order to obtain the 
// addresses of function that are about to be hooked
bool d3dHelper::getD3D9Device()
{

    if (!hwnd || !this->d3d9DeviceTable) {
        return false;
    }

    IDirect3D9* d3dSys = Direct3DCreate9(D3D_SDK_VERSION);

    if (!d3dSys) {
        return false;
    }

    IDirect3DDevice9* dummyDev = NULL;

    // options to create dummy device
    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = false;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hwnd;

    HRESULT dummyDeviceCreated = d3dSys->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &dummyDev);

    if (dummyDeviceCreated != S_OK)
    {
        // may fail in windowed fullscreen mode, trying again with windowed mode
        d3dpp.Windowed = !d3dpp.Windowed;

        dummyDeviceCreated = d3dSys->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &dummyDev);

        if (dummyDeviceCreated != S_OK)
        {
            d3dSys->Release();
            return false;
        }
    }

    memcpy(this->d3d9DeviceTable, *reinterpret_cast<void***>(dummyDev), sizeof(this->d3d9DeviceTable));

    dummyDev->Release();
    d3dSys->Release();
    return true;
}

// https://stackoverflow.com/questions/11711417/get-hwnd-by-process-id-c
// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms633498(v=vs.85)
// "To continue enumeration, the callback function must return TRUE; to stop enumeration, it must return FALSE."
BOOL CALLBACK d3dHelper::enumGetProcessWindow(HWND _hwnd, LPARAM lParam)
{
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(_hwnd, &lpdwProcessId);
    if (lpdwProcessId == lParam)
    {
        hwnd = _hwnd;
        return FALSE;
    }

    return TRUE;
}