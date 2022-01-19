#include <windows.h>

#include <fstream> // used for reading std::ifstream input file
#include <string>
#include <vector>


//TODO consider to give that file a better home / name?
#define DNS_FILE_NAME "amax-redirect.cfg" // input file, must sit next to the amax-redirect.asi


typedef struct {
	uintptr_t addy; // offset from start Blur.exe where the entry is located
	std::string org_host; // original hostname, in case we would want to restore it. TODO implement restore on unload?
	std::string new_host; // new hostname
} Redirect;


// loads and parses entries from text file
std::vector<Redirect> load_redirects(const char* filename) {
	std::vector<Redirect> vec_redirects;
	std::string str_addy;
	uintptr_t addy;
	std::string org_host;
	std::string new_host;
	std::ifstream f(filename);
	while (f >> str_addy >> org_host >> new_host) { // this just eats every 3 string (anything), doesnt go line by line
		if (str_addy[0] != '#') { // idk something with comments
			if ((addy = std::strtol(str_addy.c_str(), NULL, 0)) > 0x0) {
				// https://man7.org/linux/man-pages/man3/strtol.3.html
				// (EXAMPLE: strtol("0xF") = 15 ) // converts string repr of number into actual number (accepts hex repr)
				if (new_host.length() <= org_host.length()) {
					Redirect r { addy, org_host, new_host };
					vec_redirects.push_back(r);
				} else {
					// Inform user (by console or log file?) that one of the entries in filename is too long.
					// TBH, I dont know how long (# of chars) they are allowed to be, should we check in Cheat-Engine?
				}
			} else {
				// Could not convert string representation (hex 0x?) to actual addy
			}
		} else {
			// It was a silly comment starting with '#'
		}
	}
	return vec_redirects;
}


bool set_redirects(uintptr_t moduleBase, std::vector<Redirect> vec_redirects) {
	bool ok = true;
	unsigned char* dst; // where we will write the new host string to
	DWORD srcProtection, _; // for VirtualProtect, idk if its really needed?
	for (Redirect r : vec_redirects) {
		dst = (unsigned char*) (moduleBase + r.addy); // addy stored in cfg = offset from Blur.exe; so we add it to the base (moduleBase is almost always 0x40000)
		VirtualProtect(dst, r.org_host.length(), PAGE_EXECUTE_READWRITE, &srcProtection);
		// copy chars over, im sure there is more elegant way to do it...
		for (unsigned int i = 0; i < r.new_host.length(); i++) {
			// checking if the original chars match, unused so far...
			ok = ok && (dst[i] == r.org_host[i]);
			dst[i] = r.new_host[i];
		}
		// set the rest to null chars just in case
		for (unsigned int i = r.new_host.length(); i < r.org_host.length(); i++) {
			dst[i] = '\0';
		}
		VirtualProtect(dst, r.org_host.length(), srcProtection, &_);
	}
	return ok;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH: {
			uintptr_t moduleBase = (uintptr_t) GetModuleHandle(NULL);
			std::vector<Redirect> vec_redirects = load_redirects(DNS_FILE_NAME);
			bool ok = set_redirects(moduleBase, vec_redirects);
			if (ok) {
				// Inform?
			} else {
				// IDK, restore?
			}
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
