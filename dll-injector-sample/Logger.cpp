#include "stdafx.h"

void logToFile(std::string message)
{
	WCHAR cOutputPath[200];
	ExpandEnvironmentStringsW(L"%UserProfile%\\Desktop\\log_file.txt", cOutputPath, sizeof(cOutputPath) / sizeof(*cOutputPath));
	std::ofstream log_file(cOutputPath, std::ios_base::out | std::ios_base::app);
	log_file << message << std::endl;
}
void logToFile(std::wstring message)
{
	WCHAR cOutputPath[200];
	ExpandEnvironmentStringsW(L"%UserProfile%\\Desktop\\log_file.txt", cOutputPath, sizeof(cOutputPath) / sizeof(*cOutputPath));
	std::wofstream  log_file(cOutputPath, std::ios_base::out | std::ios_base::app);
	log_file << message << std::endl;
}