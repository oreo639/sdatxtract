#pragma once

#include "memfile.hpp"

class SdatX {
	public:

		SdatX(const std::string &filepath);
		SdatX(const std::string &name, uint8_t* data, uint32_t size);
		~SdatX();

		bool Extract();
		bool Write();

		static bool Init(const std::string &filepath, std::vector<SdatX> &sdats, bool &isNds);

	private:

		std::string m_Filepath;
		std::streamoff m_Length;

		std::shared_ptr<MemFile> m_File = nullptr;

		//std::map<int, Swar*> Swars;

};
