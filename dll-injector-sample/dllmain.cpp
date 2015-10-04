// dll-injector-sample.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//
#include "stdafx.h"

#pragma data_seg("SHARED")
bool unloadDll = false;
HMODULE hThisDll = nullptr;
HANDLE hRemoteThread = nullptr;
#pragma data_seg()
#pragma comment(linker, "/section:SHARED,RWS")

void InjectDLL();
void asyncThreadFunction(void* pvoid);
LRESULT __stdcall hookProc(int code, WPARAM wParam, LPARAM lParam);
void __stdcall UnloadDLL();

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD fdwReason,
                      LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		if (hThisDll == nullptr && lpReserved == nullptr)
		{
			hThisDll = hModule;
			InjectDLL();
		}
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

void InjectDLL()
{
	hRemoteThread = HANDLE(_beginthread(asyncThreadFunction, 0, nullptr));
}

void __stdcall UnloadDLL()
{
	unloadDll = true;
}

LRESULT __stdcall hookProc(int code, WPARAM wParam, LPARAM lParam)
{
	MessageBoxW(nullptr, L"Test", L"Test", MB_OK);

	if (hRemoteThread == nullptr)
	{
		InjectDLL();
	}

	return CallNextHookEx(nullptr, code, wParam, lParam);
}

void asyncThreadFunction(void*)
{
	while (!unloadDll)
	{
		Beep(4000, 1000);
		Sleep(1000L);
	}
	FreeLibrary(hThisDll);
	hThisDll = nullptr;
	hRemoteThread = nullptr;
}