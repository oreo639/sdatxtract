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

SdatX::SdatX(const std::string& filepath) : m_Filepath(filepath)
{
	verbose("======================\n");
	verbose("Options: (1=enabled/0=disabled)\n");
	verbose("bDecodeFile:%d\n", bDecodeFile);
	verbose("bVerboseMessages:%d\n", bVerboseMessages);
	verbose("bUseFname:%d\n", bUseFname);
	verbose("======================\n");

	if (SDAT::Verify(m_Filepath)) {
		std::ifstream ifs(m_Filepath, std::ios::binary | std::ios::ate);
		m_Length = ifs.tellg();

		if (ifs.good()) {
			m_File = new MemFile(m_Filepath, (uint32_t)m_Length);

			ifs.seekg(0, std::ios::beg);
			ifs.read(reinterpret_cast<char*>(m_File->GetRawPtr()), m_Length);
			ifs.close();
		}
	} else {
		error("Failed to open file.\n");
	}
}

SdatX::~SdatX()
{
	if (m_File) {
		delete m_File;
	}
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
				continue;
			}

			for(uint32_t j = 0; j < swarx.filenum; j++) {
				std::string fname = std::to_string(j);
				SWAV swav;
				verbose("Swav processed %d\n", j);
				if(SWAV_genSwav(&swarx.file[j], &swav)){
					printf("SWAV create error.\n");
					SWAV_clear(&swav);
					continue;
				}
				if (bDecodeFile) {
					NSSwav *nsswav = nsSwavCreate(swav.swavimage, swav.swavsize);
					if(!nsswav){
						printf("SWAV open error.\n");
						SWAV_clear(&swav);
						continue;
					}

					if(!nsSwavWriteToWaveFile(nsswav, (fname+".wav").c_str())){
						printf("WAVE write error.\n");
						nsSwavDelete(nsswav);
						SWAV_clear(&swav);
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
