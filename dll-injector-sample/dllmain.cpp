// dllmain.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
#include "stdafx.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		MessageBox(NULL, L"DLL attached", L"DLL attached", MB_OK);
		break;
	case DLL_THREAD_ATTACH:
		MessageBox(NULL, L"DLL Thread attached", L"DLL Thread attached", MB_OK);
		break;
	case DLL_THREAD_DETACH:
		MessageBox(NULL, L"DLL Thread detached", L"DLL Thread detached", MB_OK);
		break;
	case DLL_PROCESS_DETACH:
		MessageBox(NULL, L"DLL detached", L"DLL detached", MB_OK);
		break;
	}
	return TRUE;
}

