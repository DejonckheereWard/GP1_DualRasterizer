#include "pch.h"
#include "Utils.h"

// Made the CPP file because of linker errors when i had it defined in the header file.

void Utils::PrintColor(const std::string& text, TextColor textColor, const std::string& end)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hConsole, static_cast<WORD>(textColor));
	std::cout << text << end;
	SetConsoleTextAttribute(hConsole, static_cast<WORD>(TextColor::White));
}
