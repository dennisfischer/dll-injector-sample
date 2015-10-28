=DLL-Injector Sample DLL=

This is a example DLL file, that makes use of the Windows Automation API.
The DLL will free itself once execution ends. If the DLL is not injected into chrome, execution stop immediatly.
Inside chrome, parts of Windows Automation API are executed and the window structure is written to a logFile on the Desktop.
Additionally the value of the current tabs input is changed to show successful code injection.

Windows Automation API could already be used from outside, however you could basically do anything you want because the 
DLL is running inside chromes process.