#include <Windows.h>

__declspec(dllexport) BOOL SetKeyBdHook(HOOKPROC proc);
__declspec(dllexport) BOOL ResetKeyBdHook();
//__declspec(dllexport) LRESULT __stdcall KeyBdHookProc(int,WPARAM,LPARAM);
