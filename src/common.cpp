#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

#include <stdint.h>

#include "common.hpp"

bool Common::ShowWarnings = false;
std::vector<std::string> Common::Log;

void Common::Warning(MemFile& mf, std::string msg)
{
	if (ShowWarnings)
	{
		std::cerr << std::hex << std::setfill('0') << std::uppercase << std::endl;
		std::cerr << "WARNING IN\t" << mf.GetFilename() << std::endl;
		std::cerr << "AT POSITION\t0x" << std::setw(8) << mf.GetOffset() << std::endl;
		std::cerr << "MESSAGE\t\t" << msg << std::endl;
		std::cerr << std::endl;
	}
}

void Common::Analyse(MemFile& mf, std::string tag, uint32_t val)
{
	Common::Log.push_back(mf.GetFilename() + "," + tag + "," + std::to_string(val));
}

void Common::Dump(std::string fileName)
{
	std::ofstream ofs(fileName);
	ofs << "fileName,tag,val" << std::endl;

	for (size_t i = 0; i < Common::Log.size(); ++i)
	{
		ofs << Common::Log[i] << std::endl;
	}

	ofs.close();
}
