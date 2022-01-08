#include <windows.h>
#include <winsock.h>
#include <libloaderapi.h>
#include <stdint.h>

//#include <iostream>
#include <fstream> // used for std::ifstream input file reading
#include <string> // std::string magic
#include <map> // saving string key-value pairs in the input file

#pragma comment(lib, "ws2_32.lib") 
#pragma comment(lib, "user32.lib") 


#define HOOK_NET_FUNC_ADDY 0x92213B
#define OP_CALL_LEN 6

#define DNS_FILE_NAME "amax-redirect.cfg" // input file, must sit next to the amax-redirect.asi


// global map for orignal->redirect domains
std::map<std::string, std::string> dns_redirects;


// parse input file and return domain pairs as map
std::map<std::string, std::string> get_dns_redirects() {
	std::map<std::string, std::string> r = {};
	std::string key, value;
	std::ifstream f(DNS_FILE_NAME);
	while (f >> key >> value) {
		r[key] = value;
	}
	return r;
}


struct hostent* __stdcall hook_gethostbyname(const char* name) {
	if (dns_redirects.count(name)) {
		name = dns_redirects[name].c_str();
	}
	return gethostbyname(name);
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
	switch(fdwReason) { 
		case DLL_PROCESS_ATTACH: {
			dns_redirects = get_dns_redirects();

			uintptr_t moduleBase = (uintptr_t) GetModuleHandle(NULL);
			void* src = (void*) (moduleBase + HOOK_NET_FUNC_ADDY);
			void* callArg = VirtualAlloc(0, sizeof(uintptr_t), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			*(uintptr_t*) callArg = (uintptr_t) hook_gethostbyname;

			DWORD srcProtection, _;
			VirtualProtect(src, OP_CALL_LEN, PAGE_EXECUTE_READWRITE, &srcProtection);
			*(uintptr_t*) (((uint8_t*)src) + 2) = (uintptr_t) callArg;
			VirtualProtect(src, OP_CALL_LEN, srcProtection, &_);
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
