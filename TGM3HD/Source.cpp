#define WIN32_LEAN_AND_MEAN

#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <windows.h>

// Patch resolution and aspect ratio with VirtualProtectEx and WriteProcessMemory.

static void apply_patches(const HANDLE process)
{
	const auto patch_extern = [process](const uintptr_t addr, const void* buf, const size_t size)
	{
		DWORD old_protect;

		VirtualProtectEx(process, (void*)(addr), size, PAGE_EXECUTE_READWRITE, &old_protect);
		WriteProcessMemory(process, (void*)(addr), buf, size, nullptr);
		VirtualProtectEx(process, (void*)(addr), size, old_protect, &old_protect);
	};

	const auto resolution_x = GetSystemMetrics(SM_CXSCREEN);
	const auto resolution_y = GetSystemMetrics(SM_CYSCREEN);

	const auto fullscreen = (char)(true);
	patch_extern(0x44DCC9, &fullscreen, sizeof(fullscreen));

	patch_extern(0x40D160, &resolution_y, sizeof(resolution_y));
	patch_extern(0x40D165, &resolution_x, sizeof(resolution_x));

	patch_extern(0x40D19A, &resolution_y, sizeof(resolution_y));
	patch_extern(0x40D19F, &resolution_x, sizeof(resolution_x));

	patch_extern(0x41F154, &resolution_x, sizeof(resolution_x));
	patch_extern(0x41F163, &resolution_x, sizeof(resolution_x));
	patch_extern(0x41F176, &resolution_y, sizeof(resolution_y));
	patch_extern(0x41F181, &resolution_y, sizeof(resolution_y));

	patch_extern(0x44DCA6, &resolution_y, sizeof(resolution_y));
	patch_extern(0x44DCAB, &resolution_x, sizeof(resolution_x));
	patch_extern(0x44DCB0, &resolution_y, sizeof(resolution_y));
	patch_extern(0x44DCB5, &resolution_x, sizeof(resolution_x));

	patch_extern(0x44DD2D, &resolution_y, sizeof(resolution_y));
	patch_extern(0x44DD32, &resolution_x, sizeof(resolution_x));
	patch_extern(0x44DD4D, &resolution_y, sizeof(resolution_y));
	patch_extern(0x44DD52, &resolution_x, sizeof(resolution_x));

	const auto aspect_ratio = (float)(resolution_x) / (float)(resolution_y);
	patch_extern(0x44DD6B, &aspect_ratio, sizeof(aspect_ratio));

	patch_extern(0x44E126, &resolution_y, sizeof(resolution_y));
	patch_extern(0x44E12B, &resolution_x, sizeof(resolution_x));

	patch_extern(0x44E198, &resolution_y, sizeof(resolution_y));
	patch_extern(0x44E19D, &resolution_x, sizeof(resolution_x));

	patch_extern(0x44E349, &resolution_y, sizeof(resolution_y));
	patch_extern(0x44E34E, &resolution_x, sizeof(resolution_x));

	patch_extern(0x44E429, &resolution_y, sizeof(resolution_y));
	patch_extern(0x44E42E, &resolution_x, sizeof(resolution_x));

	patch_extern(0x450E5B, &resolution_y, sizeof(resolution_y));
	patch_extern(0x450E60, &resolution_x, sizeof(resolution_x));

	patch_extern(0x450E90, &resolution_y, sizeof(resolution_y));
	patch_extern(0x450E95, &resolution_x, sizeof(resolution_x));

	patch_extern(0x450ED7, &resolution_y, sizeof(resolution_y));
	patch_extern(0x450EDC, &resolution_x, sizeof(resolution_x));
}

// Use VirtualAllocEx and WriteProcessMemory to insert the DLL path into the external
// process, then CreateRemoteThread to make it call LoadLibrary.

void inject_dll(const HANDLE process, const char* dll_path)
{
	const auto buf_len = strlen(dll_path) + 1;
	auto* buf = VirtualAllocEx(process, nullptr, buf_len, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	WriteProcessMemory(process, buf, dll_path, buf_len, nullptr);
	CreateRemoteThread(process, nullptr, 0, (LPTHREAD_START_ROUTINE)(LoadLibrary), buf, 0, nullptr);
}

// Launch TGM3 suspended, patch the resolution, inject JVSEmu and start the main thread.

int main(const int argc, const char* argv[])
{
	PROCESS_INFORMATION proc_info;
	STARTUPINFO startup_info = { 0 };
	startup_info.cb = sizeof(startup_info);

	char cmdline[256];
	strcpy_s(cmdline, "game");

	for (auto i = 1; i < argc; i++)
	{
		strcat_s(cmdline, " ");
		strcat_s(cmdline, argv[i]);
	}

	CreateProcess(nullptr, cmdline, nullptr, nullptr, false, CREATE_SUSPENDED, nullptr, nullptr, &startup_info, &proc_info);

	apply_patches(proc_info.hProcess);
	inject_dll(proc_info.hProcess, "jvsemu.dll"); // Inject JVSEmu, rename if other library is used.
	ResumeThread(proc_info.hThread);

	return 0;
}