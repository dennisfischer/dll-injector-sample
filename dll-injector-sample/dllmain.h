#pragma once
#include <SDKDDKVer.h>
#include <string>
#include <process.h>
#include <uiautomation.h>
#include "Logger.h"

#define SAFE_RELEASE(element) if(element != nullptr) { element->Release(); element = nullptr; }

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved);
void __stdcall UnloadDLL();

void InjectDLL();
void asyncThreadFunction(void* pvoid);
LRESULT __stdcall hookProc(int code, WPARAM wParam, LPARAM lParam);
BOOL InitializeUIAutomation();
HRESULT listTree(int level, IUIAutomationElement* rootNode, IUIAutomationTreeWalker* walker);
HRESULT buildFullTree(IUIAutomationElement* rootNode);
