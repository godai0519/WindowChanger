#include <Windows.h>
#include "KeyBdHook.h"

#pragma comment(linker, "/section:shared,rws")
#pragma data_seg("shared")

HHOOK hKeyHook = NULL;
HINSTANCE dll_hInst = NULL;

#pragma data_seg()

BOOL WINAPI DllMain(HINSTANCE hInstance,DWORD dwReason,LPVOID lpReserved)
{
	switch(dwReason){
	case DLL_PROCESS_ATTACH:
		dll_hInst = hInstance;
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

__declspec(dllexport) BOOL SetKeyBdHook(HOOKPROC proc)
{
	hKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, proc, dll_hInst, 0);
	if(hKeyHook == NULL) return FALSE;
	return TRUE;
}
__declspec(dllexport) BOOL ResetKeyBdHook()
{
	if(UnhookWindowsHookEx(hKeyHook) == 0) return FALSE;
	return TRUE;
}
