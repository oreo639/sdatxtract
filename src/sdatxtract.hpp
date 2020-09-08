#pragma once

#include "memfile.hpp"

struct SdatX {
	std::string m_Filepath;
	std::streamoff m_Length;

	MemFile* m_File;

	//std::map<int, Swar*> Swars;

	SdatX(const std::string &filepath);
	~SdatX();

	bool Extract();
};
