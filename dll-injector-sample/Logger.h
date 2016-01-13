#pragma once
#include <string>
#include <fstream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void logToFile(std::string message);
void logToFile(std::wstring message);