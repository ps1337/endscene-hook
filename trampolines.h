#ifndef TRAMPOLINES_H
#define TRAMPOLINES_H

// pointer to a function that's like the original EndScene()
typedef HRESULT(APIENTRY *endSceneFunc)(LPDIRECT3DDEVICE9 pDevice);

extern endSceneFunc trampEndScene;

#endif