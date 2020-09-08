#pragma once

#include "memfile.hpp"

class SdatX {
	public:

		SdatX(const std::string &filepath);
		SdatX(const std::string &name, uint8_t* data, uint32_t size);
		~SdatX();

		bool Extract();
		bool Write();

		static std::vector<SdatX> Init(const std::string &filepath, bool &isNds);

	private:

		std::string m_Filepath;
		std::streamoff m_Length;

		std::shared_ptr<MemFile> m_File = nullptr;

		//std::map<int, Swar*> Swars;

};
