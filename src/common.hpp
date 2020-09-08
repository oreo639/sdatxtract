#pragma once

#include <stack>
#include <iostream>
#include <iomanip>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "memfile.hpp"

struct Common
{
	static bool ShowWarnings;
	static std::stack<std::string> FileNames;
	static std::vector<std::string> Log;

	template<typename T>
	static bool Assert(MemFile& mf, T expected, T found)
	{
		if (found != expected)
		{
			std::cerr << std::hex << std::setfill('0') << std::uppercase << std::endl;
			std::cerr << "ERROR IN\t" << mf.GetFilename() << std::endl;
			std::cerr << "AT POSITION\t0x" << std::setw(8) << mf.GetOffset() << std::endl;
			std::cerr << "BYTES READ\t0x" << std::setw(8) << sizeof(T) << std::endl;
			std::cerr << "EXPECTED\t0x" << std::setw(8) << expected << std::endl;
			std::cerr << "INSTEAD GOT\t0x" << std::setw(8) << found << std::endl;
			std::cerr << std::endl;

			return false;
		}

		return true;
	}

	template<typename T>
	static void Error(MemFile& mf, std::string expected, T found)
	{
		std::cerr << std::hex << std::setfill('0') << std::uppercase << std::endl;
		std::cerr << "ERROR IN\t" << mf.GetFilename() << std::endl;
		std::cerr << "AT POSITION\t0x" << std::setw(8) << mf.GetOffset() << std::endl;
		std::cerr << "BYTES READ\t0x" << std::setw(8) << sizeof(T) << std::endl;
		std::cerr << "EXPECTED\t" << expected << std::endl;
		std::cerr << "INSTEAD GOT\t0x" << std::setw(8) << found << std::endl;
		std::cerr << std::endl;
	}

	static void Warning(MemFile& mf, std::string msg);
	static void Analyse(MemFile& mf, std::string tag, uint32_t val);
	static void Dump(std::string fileName);
};
