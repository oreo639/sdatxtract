#pragma once

#include "memfile.hpp"

enum sdatrecord {
	SDATR_SSEQ = 0, // Sequence
	SDATR_SSEA,     // Sequence Archive
	SDATR_SBNK,     // Bank
	SDATR_SWAR,     // Wave Archive
	SDATR_SPLR,     // Player
	SDATR_SGRP,     // Group
	SDATR_SSPL,     // Sound Player
	SDATR_STRM,     // Stream
};

enum sdatIlist {
	SDATI_SSEQ = 0, // Sequence
	SDATI_SBNK,     // Bank
	SDATI_SWAR,     // Wave Archive
	SDATI_STRM,     // Stream
};

struct SSEQInfo {
	uint16_t fileId;
	uint16_t unk0;
	uint16_t bank;       // Associated BANK
	uint8_t  volume;     // Volume
	uint8_t  chnPrio;    // Channel Priority
	uint8_t  plrPrio;    // Player Priority
	uint8_t  players;
	uint8_t  unk1[2];

	SSEQInfo(MemFile& mf, uint32_t baseOffset) {
		mf.m_Offset = baseOffset;
		fileId  = mf.Read<uint16_t>();
		unk0    = mf.Read<uint16_t>();
		bank    = mf.Read<uint16_t>();
		volume  = mf.Read<uint8_t>();
		chnPrio = mf.Read<uint8_t>();
		plrPrio = mf.Read<uint8_t>();
		players = mf.Read<uint8_t>();
		mf.ReadArray(unk1, sizeof(unk1));
	}

	SSEQInfo() {
		fileId  = (uint16_t)-1;
		unk0    = 0;
		bank    = 0;
		volume  = 0;
		chnPrio = 0;
		plrPrio = 0;
		players = 0;
	}

	~SSEQInfo() {}
};

struct SBNKInfo {
	uint16_t fileId;
	uint16_t unk0;
	uint16_t wars[4];      // Associated WAVEARC. 0xffff if not in use

	SBNKInfo(MemFile& mf, uint32_t baseOffset) {
		mf.m_Offset = baseOffset;
		fileId  = mf.Read<uint16_t>();
		unk0    = mf.Read<uint16_t>();
		mf.ReadArray(wars, sizeof(wars));
	}

	SBNKInfo() {
		fileId  = (uint16_t)-1;
		unk0    = 0;
	}

	~SBNKInfo() {}
};

struct SWARInfo {
	uint16_t fileId;
	uint16_t unk0;

	SWARInfo(MemFile& mf, uint32_t baseOffset) {
		mf.m_Offset = baseOffset;
		fileId  = mf.Read<uint16_t>();
		unk0    = mf.Read<uint16_t>();
	}

	SWARInfo() {
		fileId  = (uint16_t)-1;
		unk0    = 0;
	}

	~SWARInfo() {}
};

struct STRMInfo {
	uint16_t fileId;
	uint16_t unk0;
	uint8_t  volume;
	uint8_t  prio;         // Priority
	uint8_t  players;
	uint8_t  reserved[5];

	STRMInfo(MemFile& mf, uint32_t baseOffset) {
		mf.m_Offset = baseOffset;
		fileId  = mf.Read<uint16_t>();
		unk0    = mf.Read<uint16_t>();
		volume  = mf.Read<uint8_t>();
		prio    = mf.Read<uint8_t>();
		players = mf.Read<uint8_t>();
		mf.ReadArray(reserved, sizeof(reserved));
	}

	STRMInfo() {
		fileId  = (uint16_t)-1;
		unk0    = 0;
		volume  = 0;
		prio    = 0;
		players = 0;
	}

	~STRMInfo() {}
};

struct SdatInfo
{
	std::vector<SSEQInfo> sseqinfo;
	std::vector<SBNKInfo> sbnkinfo;
	std::vector<SWARInfo> swarinfo;
	std::vector<STRMInfo> strminfo;
};

struct FTableEntry
{
	uint8_t* Offset;
	uint32_t Length;
};

struct SymbRecord
{
	uint8_t* Offset;

	std::string Name;
};

typedef std::vector<SymbRecord> SdatSymb;

struct SdatSseq
{
	uint8_t* Offset;
	uint32_t Length;

	std::string FileName;
};

struct SdatSbnk
{
	uint8_t* Offset;
	uint32_t Length;

	std::string FileName;
};

struct SdatSwar
{
	uint8_t* Offset;
	uint32_t Length;

	std::string FileName;
};

struct SdatStrm
{
	uint8_t* Offset;
	uint32_t Length;

	std::string FileName;
};

struct SdatFiles
{
	std::vector<SdatSseq> sseqs;
	std::vector<SdatSbnk> sbnks;
	std::vector<SdatSwar> swars;
	std::vector<SdatStrm> strms;
};

namespace SDAT {
	bool Verify(const std::string& filename);
	bool ReadHeader(MemFile& mf, SdatFiles& files);
}
