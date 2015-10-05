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


IUIAutomation* g_pAutomation;

BOOL InitializeUIAutomation()
{
	CoInitialize(nullptr);
	auto hr = CoCreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER, IID_IUIAutomation, reinterpret_cast<void**>(&g_pAutomation));
	return (SUCCEEDED(hr));
}


#define SAFE_RELEASE(element) if(element != nullptr) { element->Release(); element = nullptr; }

HRESULT listTree(int level, IUIAutomationElement* rootNode, IUIAutomationTreeWalker* walker)
{
	auto hr = S_OK;
	IUIAutomationElement* element = nullptr;
	auto base = std::wstring(level, L'-');
	hr = walker->GetFirstChildElement(rootNode, &element);
	if (FAILED(hr) || element == nullptr)
	{
		return hr;
	}

	while (element != nullptr)
	{
		BOOL isControl;
		hr = element->get_CurrentIsControlElement(&isControl);

		if (FAILED(hr))
		{
			logToFile(base + L"Failed element is control type");
			logToFile(base + L"Error code: " + std::to_wstring(hr));
		}
		else if (isControl)
		{
			BSTR controlName;
			BSTR controlType;
			CONTROLTYPEID controlTypeId;
			hr = element->get_CurrentName(&controlName);
			hr = element->get_CurrentLocalizedControlType(&controlType);
			hr = element->get_CurrentControlType(&controlTypeId);

			if (FAILED(hr))
			{
				logToFile(base + L"Failed element control type");
				logToFile(base + L"Error code: " + std::to_wstring(hr));
			}
			if (controlName != nullptr)
			{
				logToFile(base + L" NAME: " + static_cast<std::wstring>(controlName));
			}
			if (controlType != nullptr)
			{
				logToFile(base + L" TYPE: " + static_cast<std::wstring>(controlType));
			}
			if (controlTypeId != NULL)
			{
				logToFile(base + L" ID: " + std::to_wstring(controlTypeId));
			}
		}

		listTree(level * 2, element, walker);

		IUIAutomationElement* next = nullptr;
		hr = walker->GetNextSiblingElement(element, &next);
		if (FAILED(hr))
		{
			logToFile(base + L"Failed next element");
			logToFile(base + L"Error code: " + std::to_wstring(hr));
		}
		SAFE_RELEASE(element);
		element = next;
	}

	return S_OK;
}

HRESULT buildFullTree(IUIAutomationElement* rootNode)
{
	auto hr = S_OK;
	IUIAutomationTreeWalker* walker = nullptr;
	hr = g_pAutomation->get_ControlViewWalker(&walker);

	if (FAILED(hr) || walker == nullptr)
	{
		logToFile("Failed walker");
		logToFile("Error code: " + std::to_string(hr));
		return hr;
	}

	hr = listTree(1, rootNode, walker);
	SAFE_RELEASE(walker);
	return hr;
}

void asyncThreadFunction(void*)
{
	logToFile("-------------------------------------");
	logToFile("Injected");

	if (!InitializeUIAutomation())
	{
		logToFile("Failed to initialize UI Automation");
	}

	IUIAutomationElement* root_element = nullptr;
	IUIAutomationElement* chromeWindow = nullptr;
	IUIAutomationCondition* condition = nullptr;
	IUIAutomationElementArray* foundArray = nullptr;

	auto hr = g_pAutomation->GetRootElement(&root_element);
	if (FAILED(hr) || root_element == nullptr)
	{
		logToFile("Failed root element");
		logToFile("Error code: " + std::to_string(hr));
		goto cleanup;
	}

	VARIANT varProp;
	varProp.vt = VT_INT;
	varProp.intVal = GetCurrentProcessId();
	hr = g_pAutomation->CreatePropertyCondition(UIA_ProcessIdPropertyId, varProp, &condition);
	VariantClear(&varProp);

	if (FAILED(hr) || condition == nullptr)
	{
		logToFile("Failed condition");
		logToFile("Error code: " + std::to_string(hr));
		goto cleanup;
	}

	hr = root_element->FindFirst(TreeScope_Children, condition, &chromeWindow);
	SAFE_RELEASE(condition);

	if (FAILED(hr) || chromeWindow == nullptr)
	{
		logToFile("Failed chrome window");
		logToFile("Error code: " + std::to_string(hr));
		goto cleanup;
	}
	else
	{
		BSTR retVal;
		chromeWindow->get_CurrentName(&retVal);
		logToFile("Found chrome window successfully");
		logToFile(retVal);

		hr = buildFullTree(chromeWindow);
		if (FAILED(hr))
		{
			logToFile("Failed to build component tree");
			logToFile("Error code: " + std::to_string(hr));
			goto cleanup;
		}

		VARIANT varProp2;
		varProp2.vt = VT_INT;
		varProp2.intVal = UIA_EditControlTypeId;
		IUIAutomationCondition* editControlCondition = nullptr;
		IUIAutomationElement* foundElement = nullptr;

		hr = g_pAutomation->CreatePropertyCondition(UIA_ControlTypePropertyId, varProp2, &editControlCondition);

		if (FAILED(hr) || editControlCondition == nullptr)
		{
			logToFile("Failed to edit control condition");
			logToFile("Error code: " + std::to_string(hr));
			goto cleanup;
		}


		hr = chromeWindow->FindFirst(TreeScope_Descendants, editControlCondition, &foundElement);
		SAFE_RELEASE(editControlCondition);

		if (FAILED(hr) || foundElement == nullptr)
		{
			logToFile("Failed to find edit control element");
			logToFile("Error code: " + std::to_string(hr));
			goto cleanup;
		}

		IValueProvider* valueProvider = nullptr;
		foundElement->GetCurrentPattern(UIA_ValuePatternId, reinterpret_cast<IUnknown**>(&valueProvider));
		valueProvider->SetValue(L"Successfully hijacked - called from DLL");
	}

cleanup:
	SAFE_RELEASE(root_element);
	SAFE_RELEASE(chromeWindow);
	SAFE_RELEASE(foundArray);
	SAFE_RELEASE(g_pAutomation);

	logToFile("Ejected");
	logToFile("-------------------------------------");


	//Wait for unload?
	while (!unloadDll)
	{
		Beep(4000, 1000);
		Sleep(1000L);
	}
	FreeLibrary(hThisDll);
	hThisDll = nullptr;
	hRemoteThread = nullptr;
}
