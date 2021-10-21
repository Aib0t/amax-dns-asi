#include <windows.h>
#include <winsock.h>
#include <libloaderapi.h>
#include <stdint.h>
//#include <WinUser.h>

#pragma comment(lib, "ws2_32.lib") 
#pragma comment(lib, "user32.lib") 


#define HOOK_NET_FUNC_ADDY 0x92213B
#define OP_CALL_LEN 6

#define SERVER "amax-emu.com"


struct hostent* __stdcall hook_gethostbyname(const char* name) {
	//MessageBoxA(NULL, "EY", "hook_gethostbyname", MB_OK);
	if (!strcmp(name, "blur-pc-live.auth.mmp3.demonware.net") || !strcmp(name, "blur-pc-live.lsg.mmp3.demonware.net")) {
		name = SERVER;
	}
	return gethostbyname(name);
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
	switch(fdwReason) { 
		case DLL_PROCESS_ATTACH: {
			uintptr_t moduleBase = (uintptr_t) GetModuleHandle(NULL);
			void* src = (void*) (moduleBase + HOOK_NET_FUNC_ADDY);
			void* callArg = VirtualAlloc(0, sizeof(uintptr_t), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			*(uintptr_t*) callArg = (uintptr_t) hook_gethostbyname;

			DWORD srcProtection, _;
			VirtualProtect(src, OP_CALL_LEN, PAGE_EXECUTE_READWRITE, &srcProtection);
			*(uintptr_t*) (((uint8_t*)src) + 2) = (uintptr_t) callArg;
			VirtualProtect(src, OP_CALL_LEN, srcProtection, &_);

			//MessageBoxA(NULL, "DLL", "DllMain", MB_OK);
			break;

		} case DLL_THREAD_ATTACH: { // Do thread-specific initialization.
			break;

		} case DLL_THREAD_DETACH: { // Do thread-specific cleanup.
			break;

		} case DLL_PROCESS_DETACH: { // Perform any necessary cleanup.
			break;
		}
	}
	return TRUE;
}
