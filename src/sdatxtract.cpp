#include <filesystem>
#include <fstream>

#include "main.h"
#include "sdatxtract.hpp"
#include "sdat.hpp"
#include "common.hpp"

extern "C" {
#include "swar.h"
#include "decoder/nsstrm.h"
#include "decoder/nsswav.h"
#include "decoder/sseq2mid.h"
}

bool SdatXI_FindSdat(const std::string &filepath, std::vector<SdatX> &sdats) {
	uint8_t sdatMagic[] = {'S','D','A','T',0xFF,0xFE,0x00,0x01};

	std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
	uint32_t mlength = ifs.tellg();

	if (!ifs.good() || mlength < 0x400)
		return false;

	MemFile mfile = MemFile(filepath, mlength);

	{
		ifs.seekg(0, std::ios::beg);
		ifs.read(reinterpret_cast<char*>(mfile.GetRawPtr()), mlength);
		ifs.close();
	}

	for (uint32_t i = 0; i + 4 < mfile.GetSize(); i++) {
		if(!memcmp((mfile.GetRawPtr() + i), sdatMagic, sizeof(sdatMagic))) {
			verbose("SDAT found!\n");

			mfile.m_Offset = i+0xC;
			bool p1 = mfile.Read<uint16_t>() < 0x100;
			mfile.m_Offset = i+0x10;
			bool p2 = mfile.Read<uint32_t>() < 0x100;

			if (p1 && p2) {
				verbose("SDAT confirmed!\n");
				verbose("Reading sdat %d.\n", sdats.size());

				mfile.m_Offset = i+0x08;
				sdats.emplace_back(std::to_string(sdats.size()), mfile.GetRawPtr() + i, mfile.Read<uint32_t>());
			}
			else {
				verbose("SDAT rejected!\n");
			}
		}
	}

	printf("Total SDATs found: %d\n", sdats.size());
	return true;
}

bool SdatX::Init(const std::string &filepath, std::vector<SdatX> &sdats, bool &isNds) {
	if (!File::Exists(filepath)) {
		error("Failed to open file.\n");
		return false;
	}

	if (!SDAT::Verify(filepath)) {
		isNds = true;
		SdatXI_FindSdat(filepath, sdats);
	} else
		sdats.emplace_back(filepath);

	return true;
}

SdatX::SdatX(const std::string &filepath) : m_Filepath(filepath)
{
	if (SDAT::Verify(m_Filepath)) {
		std::ifstream ifs(m_Filepath, std::ios::binary | std::ios::ate);
		m_Length = ifs.tellg();

		if (ifs.good()) {
			m_File = std::make_shared<MemFile>(m_Filepath, (uint32_t)m_Length);

			ifs.seekg(0, std::ios::beg);
			ifs.read(reinterpret_cast<char*>(m_File->GetRawPtr()), m_Length);
			ifs.close();
		}
	} else {
		error("Failed to open file.\n");
	}
}

SdatX::SdatX(const std::string &name, uint8_t* data, uint32_t size) : m_Filepath(name)
{
	m_File = std::make_shared<MemFile>(m_Filepath, data, size);
}

SdatX::~SdatX()
{
	m_File = nullptr;
}

bool SdatX::Write()
{
	std::string outfile = "sdat_file_"+std::string(fs::path(m_Filepath).stem())+".sdat";
	printf("Outputting to file %s\n", outfile.c_str());

	std::ofstream ofs(outfile, std::ofstream::binary);
	ofs.write(reinterpret_cast<const char*>(m_File->GetRawPtr()), m_File->GetSize());
	ofs.close();

	return true;
}

bool SdatX::Extract()
{
	if (!m_File)
		return false;

	SdatFiles files;

	printf("Processing file %s\n", m_Filepath.c_str());
	printf("Reading SDAT.\n");
	if (!SDAT::ReadHeader(*m_File, files))
		return false;

	int numSSEQ = 0, numSTRM = 0, numSWAR = 0, numSBNK = 0;

	std::string outdir = fs::path(m_Filepath).stem();
	printf("Outputting to directory %s\n", outdir.c_str());
	fs::create_directory(outdir);
	fs::current_path(outdir);

	fs::create_directory(DIR_SSEQ);
	fs::current_path(DIR_SSEQ);
	printf("Writing SSEQ:\n");
	for (uint32_t i = 0; i < files.sseqs.size(); i++) {
		SdatSseq sseq = files.sseqs[i];

		verbose("Prossessing sseq #%d:  ", i);
		verbose("File %s\n", sseq.FileName.c_str());
		processIndicator();

		if(bDecodeFile){
			Sseq2mid *nssseq = sseq2midCreate(sseq.Offset, sseq.Length, false);
			if(!nssseq){
				printf("SSEQ open error.\n");
				continue;
			}
			sseq2midSetLoopCount(nssseq, 1);
			sseq2midNoReverb(nssseq, false);
			if(!sseq2midConvert(nssseq)) {
				printf("SSEQ convert error.\n");
				sseq2midDelete(nssseq);
				continue;
			}
			if(!sseq2midWriteMidiFile(nssseq, (sseq.FileName+".mid").c_str())) {
				printf("MIDI write error.\n");
				sseq2midDelete(nssseq);
				continue;
			}
			else numSSEQ++;
			sseq2midDelete(nssseq);
		} else {
			std::ofstream ofs(sseq.FileName+".sseq", std::ofstream::binary);
			ofs.write(reinterpret_cast<const char*>(sseq.Offset), sseq.Length);
			ofs.close();
			numSSEQ++;
		}
	}
	printf("\n");
	fs::current_path("..");

	fs::create_directory(DIR_SBNK);
	fs::current_path(DIR_SBNK);
	printf("Writing SBNK:\n");
	for (uint32_t i = 0; i < files.sbnks.size(); i++) {
		SdatSbnk sbnk = files.sbnks[i];

		verbose("Prossessing sbnk #%d:  ", i);
		verbose("File %s\n", sbnk.FileName.c_str());
		processIndicator();

		std::ofstream ofs(sbnk.FileName+".sbnk", std::ofstream::binary);
		ofs.write(reinterpret_cast<const char*>(sbnk.Offset), sbnk.Length);
		ofs.close();
		numSBNK++;
	}
	printf("\n");
	fs::current_path("..");

	fs::create_directory(DIR_SWAR);
	fs::current_path(DIR_SWAR);
	printf("Extracting SWAR:\n");
	for (uint32_t i = 0; i < files.swars.size(); i++) {
		SdatSwar swar = files.swars[i];

		verbose("Prossessing Swar #%d:    ", i);
		verbose("File %s\n", swar.FileName.c_str());
		processIndicator();

		if(bDecodeFile || bGetSwav){
			fs::create_directory(swar.FileName);
			fs::current_path(swar.FileName);

			SWAR swarx;
			int ret = 0;
			if((ret = SWAREX_init(&swarx, swar.Offset, swar.Length))) {
				if (ret == SWARE_BAD)
					printf("SWAR open error: Did not pass validation.\nMay be corrupted?\n");
				if (ret == SWARE_EMPTY)
					printf("SWAR open error: No files found to extract.\n");

				SWAREX_exit(&swarx);
				fs::current_path("..");
				continue;
			}

			for(uint32_t j = 0; j < swarx.filenum; j++) {
				std::string fname = std::to_string(j);
				SWAV swav;
				verbose("Swav processed %d\n", j);
				if(SWAV_genSwav(&swarx.file[j], &swav)){
					printf("SWAV create error.\n");
					SWAV_clear(&swav);
					fs::current_path("..");
					continue;
				}
				if (bDecodeFile) {
					NSSwav *nsswav = nsSwavCreate(swav.swavimage, swav.swavsize);
					if(!nsswav){
						printf("SWAV open error.\n");
						SWAV_clear(&swav);
						fs::current_path("..");
						continue;
					}

					if(!nsSwavWriteToWaveFile(nsswav, (fname+".wav").c_str())){
						printf("WAVE write error.\n");
						nsSwavDelete(nsswav);
						SWAV_clear(&swav);
						fs::current_path("..");
						continue;
					}
					nsSwavDelete(nsswav);
				} else {
					std::ofstream ofs(fname+".swav", std::ofstream::binary);
					ofs.write(reinterpret_cast<const char*>(swav.swavimage), swav.swavsize);
					ofs.close();
				}
				numSWAR++;
				SWAV_clear(&swav);
			}
			SWAREX_exit(&swarx);
			fs::current_path("..");
		} else {
			std::ofstream ofs(swar.FileName+".swar", std::ofstream::binary);
			ofs.write(reinterpret_cast<const char*>(swar.Offset), swar.Length);
			ofs.close();
			numSWAR++;
		}
	}
	printf("\n");
	fs::current_path("..");

	fs::create_directory(DIR_STRM);
	fs::current_path(DIR_STRM);
	printf("Writing STRM:\n");
	for (uint32_t i = 0; i < files.strms.size(); i++) {
		SdatStrm strm = files.strms[i];

		verbose("Prossessing strm #%d:  ", i);
		verbose("File %s\n", strm.FileName.c_str());
		processIndicator();

		if(bDecodeFile){
			//STRM open
			NSStrm *nsstrm = nsStrmCreate(strm.Offset, strm.Length);
			if (!nsstrm) {
				printf("STRM open error.\n");
				continue;
			}
			if(!nsStrmWriteToWaveFile(nsstrm, (strm.FileName+".wav").c_str())) {
				printf("WAVE write error.\n");
				nsStrmDelete(nsstrm);
				continue;
			}
			else numSTRM++;
			nsStrmDelete(nsstrm);
		} else {
			std::ofstream ofs(strm.FileName+".strm", std::ofstream::binary);
			ofs.write(reinterpret_cast<const char*>(strm.Offset), strm.Length);
			ofs.close();
			numSTRM++;
		}
	}
	printf("\n");
	fs::current_path("..");

	fs::current_path(".."); // outdir

	printf("Total written files:\n");
	printf("    SSEQ:%d\n", numSSEQ);
	printf("    SBNK:%d\n", numSBNK);
	printf("    %s:%d\n", bDecodeFile ? "SWAV" : "SWAR", numSWAR);
	printf("    STRM:%d\n", numSTRM);

	return true;
}
