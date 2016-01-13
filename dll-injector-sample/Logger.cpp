#include "Logger.h"

//Log a string to log_file.txt located on the desktop
void logToFile(std::string message)
{
	wchar_t cOutputPath[200];
	ExpandEnvironmentStringsW(L"%UserProfile%\\Desktop\\log_file.txt", cOutputPath, sizeof(cOutputPath) / sizeof(*cOutputPath));
	std::ofstream log_file(cOutputPath, std::ios_base::out | std::ios_base::app);
	log_file << message << std::endl;
}

//Log a wstring to log_file.txt located on the desktop
void logToFile(std::wstring message)
{
	wchar_t cOutputPath[200];
	ExpandEnvironmentStringsW(L"%UserProfile%\\Desktop\\log_file.txt", cOutputPath, sizeof(cOutputPath) / sizeof(*cOutputPath));
	std::wofstream log_file(cOutputPath, std::ios_base::out | std::ios_base::app);
	log_file << message << std::endl;
}