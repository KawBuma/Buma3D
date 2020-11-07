#include "Buma3DPCH.h"

BOOL APIENTRY DllMain(HMODULE _hmodule, DWORD _ul_reason_for_call, LPVOID  _lp_reserved)
{
    switch (_ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
