#include "stdafx.h"

void logToFile(std::string message)
{
	std::ofstream log_file(
		"D:/log_file.txt", std::ios_base::out | std::ios_base::app);
	log_file << message << std::endl;
}
void logToFile(std::wstring message)
{
	std::wofstream  log_file(
		"D:/log_file.txt", std::ios_base::out | std::ios_base::app);
	log_file << message << std::endl;
}