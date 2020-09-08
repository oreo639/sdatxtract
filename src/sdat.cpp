#include "main.h"
#include "sdat.hpp"
#include "common.hpp"

bool SDAT::Verify(const std::string& filename) {
	FILE *fp = fopen(filename.c_str(), "rb");
	uint8_t sdatMagic[] = {'S','D','A','T'};
	char magic[sizeof(sdatMagic)+1];

	if(!fp) {
		return false;
	}

	fread(magic, 1, sizeof(sdatMagic), fp);

	fclose(fp);

	if(!memcmp(magic, sdatMagic, sizeof(sdatMagic)))
		return true;

	return false;
}

SdatSymb SDATi_ProcessRecordSYMB(MemFile& mf, uint32_t symbOffset, uint32_t recOffset) {
	mf.m_Offset = symbOffset+recOffset;

	uint32_t nEntries = mf.Read<uint32_t>();
	SdatSymb records(nEntries);

	verbose("Records: %ld\n", records.size());

	for (uint32_t i = 0; i < records.size(); i++) {
			uint32_t offset = mf.Read<uint32_t>();

			if (offset != 0)
			{
				records[i].Offset = (mf.GetRawPtr()+symbOffset)+offset;
				records[i].Name   = std::string(reinterpret_cast<const char*>(records[i].Offset));

				verbose("SYMB entry #%d: %s\n", i, records[i].Name.c_str());
			} else {
				records[i].Offset = 0;
				records[i].Name   = "";
			}
	}

	return records;
}

template<typename T>
std::vector<T> SDATi_ProcessRecordINFO(MemFile& mf, uint32_t infoOffset, uint32_t recOffset) {
	mf.m_Offset = infoOffset+recOffset;

	uint32_t nEntries = mf.Read<uint32_t>();
	std::vector<uint32_t> nEntryOffsets(nEntries);

	for (uint32_t i = 0; i < nEntryOffsets.size(); i++)
			nEntryOffsets[i] = mf.Read<uint32_t>();

	std::vector<T> records(nEntries);
	for (uint32_t i = 0; i < records.size(); i++) {
		if (nEntryOffsets[i])
			records[i] = T(mf, infoOffset+nEntryOffsets[i]);
		//verbose("INFO entry ID #%d: %d\n", i, records[i].fileId);
	}

	return records;
}

bool SDAT::ReadHeader(MemFile& mf, SdatFiles& files) {
	// SDAT header
	if (!Common::Assert(mf, 0x53444154, mf.ReadFixLen(4, false))) { return false; }
	if (!Common::Assert<uint16_t>(mf, 0xFEFF, mf.Read<uint16_t>())) { return false; }
	mf.Read<uint16_t>(); // Version
	if (!Common::Assert(mf, mf.GetSize(), mf.Read<uint32_t>())) { return false; }
	if (!Common::Assert<uint16_t>(mf, 0x40, mf.Read<uint16_t>())) { return false; }
	uint16_t numBlocks = mf.Read<uint16_t>();

	// Read block offsets
	uint32_t symbOffset = mf.Read<uint32_t>();
	uint32_t symbLength = mf.Read<uint32_t>();

	uint32_t infoOffset = mf.Read<uint32_t>();
	uint32_t infoLength = mf.Read<uint32_t>();

	uint32_t fatOffset = mf.Read<uint32_t>();
	uint32_t fatLength = mf.Read<uint32_t>();

	uint32_t fileOffset = mf.Read<uint32_t>();
	uint32_t fileLength = mf.Read<uint32_t>();

	std::vector<SdatSymb> symbs;
	bool haveSYMBs = symbOffset && symbLength;
	if (haveSYMBs) {
		std::vector<uint32_t> nRecOffset(8);

		mf.m_Offset = symbOffset;

		// SYMB block
		if (!Common::Assert(mf, 0x53594D42, mf.ReadFixLen(4, false))) { return false; }
		if (!Common::Assert(mf, symbLength, mf.Read<uint32_t>())); // Continue execution
		for (uint32_t i = 0; i < nRecOffset.size(); i++)
			nRecOffset[i] = mf.Read<uint32_t>();
		mf.m_Offset+=24;

		verbose("Reading SSEQ symbols.\n");
		symbs.push_back(SDATi_ProcessRecordSYMB(mf, symbOffset, nRecOffset[SDATR_SSEQ]));
		verbose("Reading SBNK symbols.\n");
		symbs.push_back(SDATi_ProcessRecordSYMB(mf, symbOffset, nRecOffset[SDATR_SBNK]));
		verbose("Reading SWAR symbols.\n");
		symbs.push_back(SDATi_ProcessRecordSYMB(mf, symbOffset, nRecOffset[SDATR_SWAR]));
		verbose("Reading STRM symbols.\n");
		symbs.push_back(SDATi_ProcessRecordSYMB(mf, symbOffset, nRecOffset[SDATR_STRM]));
	} else {
		warning("SYMB block not aviable, filenames will be set automatically.\n");
	}

	SdatInfo fInfo;
	if (infoOffset && infoLength) {
		std::vector<uint32_t> nRecOffset(8);

		mf.m_Offset = infoOffset;

		// INFO block
		if (!Common::Assert(mf, 0x494E464F, mf.ReadFixLen(4, false))) { return false; }
		if (!Common::Assert(mf, infoLength, mf.Read<uint32_t>())) { return false; }
		for (uint32_t i = 0; i < nRecOffset.size(); i++)
			nRecOffset[i] = mf.Read<uint32_t>();
		mf.m_Offset+=24;

		verbose("Reading SSEQ info.\n");
		fInfo.sseqinfo = SDATi_ProcessRecordINFO<SSEQInfo>(mf, infoOffset, nRecOffset[SDATR_SSEQ]);
		verbose("Reading SBNK info.\n");
		fInfo.sbnkinfo = SDATi_ProcessRecordINFO<SBNKInfo>(mf, infoOffset, nRecOffset[SDATR_SBNK]);
		verbose("Reading SWAR info.\n");
		fInfo.swarinfo = SDATi_ProcessRecordINFO<SWARInfo>(mf, infoOffset, nRecOffset[SDATR_SWAR]);
		verbose("Reading STRM info.\n");
		fInfo.strminfo = SDATi_ProcessRecordINFO<STRMInfo>(mf, infoOffset, nRecOffset[SDATR_STRM]);
	} else {
		error("No INFO block found. Cannot continue\n");
		return false;
	}

	std::vector<FTableEntry> fatEnts;
	if (fatOffset && fatLength) {
		mf.m_Offset = fatOffset;

		// FAT block
		if (!Common::Assert(mf, 0x46415420, mf.ReadFixLen(4, false))) { return false; }
		if (!Common::Assert(mf, fatLength, mf.Read<uint32_t>())) { return false; }

		verbose("Reading files.\n");
		uint32_t nEntries = mf.Read<uint32_t>();
		for (uint32_t i = 0; i < nEntries; i++) {
			FTableEntry entry;
			uint32_t offset = mf.Read<uint32_t>();

			entry.Offset = mf.GetRawPtr()+offset;
			entry.Length = mf.Read<uint32_t>();

			fatEnts.push_back(entry);
			mf.m_Offset+=8;
		}
	} else {
		error("No FAT block found. Cannot continue\n");
		return false;
	}

	for (uint32_t i = 0; i < fInfo.sseqinfo.size(); i++) {
		uint16_t id = fInfo.sseqinfo[i].fileId;

		if (id == (uint16_t)-1)
			continue;

		SdatSseq sseq;
		FTableEntry entry = fatEnts[id];
		sseq.Offset   = entry.Offset;
		sseq.Length   = entry.Length;
		if (!bUseFname || !haveSYMBs || symbs[SDATI_SSEQ][i].Name.empty())
			sseq.FileName = "SSEQ_"+std::to_string(i);
		else
			sseq.FileName = symbs[SDATI_SSEQ][i].Name;

		files.sseqs.push_back(sseq);
	}

	for (uint32_t i = 0; i < fInfo.sbnkinfo.size(); i++) {
		uint16_t id = fInfo.sbnkinfo[i].fileId;

		if (id == (uint16_t)-1)
			continue;

		SdatSbnk sbnk;
		FTableEntry entry = fatEnts[id];
		sbnk.Offset   = entry.Offset;
		sbnk.Length   = entry.Length;
		if (!bUseFname || !haveSYMBs || symbs[SDATI_SBNK][i].Name.empty())
			sbnk.FileName = "SBNK_"+std::to_string(i);
		else
			sbnk.FileName = symbs[SDATI_SBNK][i].Name;

		files.sbnks.push_back(sbnk);
	}

	for (uint32_t i = 0; i < fInfo.swarinfo.size(); i++) {
		uint16_t id = fInfo.swarinfo[i].fileId;

		if (id == (uint16_t)-1)
			continue;

		SdatSwar swar;
		FTableEntry entry = fatEnts[id];
		swar.Offset   = entry.Offset;
		swar.Length   = entry.Length;
		if (!bUseFname || !haveSYMBs || symbs[SDATI_SWAR][i].Name.empty())
			swar.FileName = "SWAR_"+std::to_string(i);
		else
			swar.FileName = symbs[SDATI_SWAR][i].Name;

		files.swars.push_back(swar);
	}

	for (uint32_t i = 0; i < fInfo.strminfo.size(); i++) {
		uint16_t id = fInfo.strminfo[i].fileId;

		if (id == (uint16_t)-1)
			continue;

		SdatStrm strm;
		FTableEntry entry = fatEnts[id];
		strm.Offset   = entry.Offset;
		strm.Length   = entry.Length;
		if (!bUseFname || !haveSYMBs || symbs[SDATI_STRM][i].Name.empty())
			strm.FileName = "STRM_"+std::to_string(i);
		else
			strm.FileName = symbs[SDATI_STRM][i].Name;

		files.strms.push_back(strm);
	}

	return true;
}
