#ifndef __SX_SDAT_H__
#define __SX_SDAT_H__

#include "nds.h"

typedef struct {
	char     magic[5];
	uint16_t endianness;
        uint16_t version;
        uint32_t filesize;
        uint16_t headersize; // 16
        uint16_t numblocks;
} sheader_t;

typedef struct {
	uint32_t numEntries;
	uint32_t *entryOffsets;
	char **names;
} symbindex_t;

typedef struct {
	char magic[5];
	uint32_t blocksize;
	uint32_t indexoffsets[8];
	uint8_t padding[24];

	symbindex_t sseq;
	symbindex_t swar;
	symbindex_t sbnk;
	symbindex_t strm;
} symb_t;

typedef struct {
	uint32_t numEntries;
	uint32_t *entryOffsets;
} infoindex_t;

typedef struct {
	uint16_t fileId;
	uint16_t unk0;
	uint16_t bank;       // Associated BANK
	uint8_t  volume;     // Volume
	uint8_t  chnnPrio;   // Channel Priority
	uint8_t  playerPrio; // Player Priority
	uint8_t  players;
	uint8_t  unk1[2];
} sseqinfo_t;

typedef struct {
	uint16_t fileId;
	uint16_t unknown;
	uint16_t wars[4];      // Associated WAVEARC. 0xffff if not in use
} sbnkinfo_t;

typedef struct {
	uint16_t fileId;
	uint16_t unknown;
} swarinfo_t;

typedef struct {
	uint16_t fileId;
	uint16_t unknown;
	uint8_t  volume;
	uint8_t  priority;
	uint8_t  players;
	uint8_t  reserved[5];
} strminfo_t;

typedef struct {
	infoindex_t index;
	sseqinfo_t* entries;
} sseqinfoindex_t;

typedef struct {
	infoindex_t index;
	swarinfo_t* entries;
} swarinfoindex_t;

typedef struct {
	infoindex_t index;
	sbnkinfo_t* entries;
} sbnkinfoindex_t;

typedef struct {
	infoindex_t index;
	strminfo_t* entries;
} strminfoindex_t;

typedef struct {
	char magic[5];
	uint32_t blocksize;
	uint32_t infoOffsets[8];
	uint8_t padding[24];

	sseqinfoindex_t sseq;
	swarinfoindex_t swar;
	sbnkinfoindex_t sbnk;
	strminfoindex_t strm;
} info_t;

typedef struct {
	uint32_t offset;
	uint32_t size;
	uint8_t  padding[8];

	uint8_t* file;
} fatentry_t;

typedef struct {
	char magic[5];
	uint32_t blocksize;
	uint32_t numEntries;

	fatentry_t* entries;
} fat_t;

typedef struct {
	symb_t symb;
	info_t info;
	fat_t  fat;
} sdat_t;

enum sdatindex {
	SDATI_SSEQ = 0, // Sequence
	SDATI_SSEA,     // Sequence Archive
	SDATI_SBNK,     // Bank
	SDATI_SWAR,     // Wave Archive
	SDATI_SPLR,     // Player
	SDATI_SGRP,     // Group
	SDATI_SSPL,     // Sound Player
	SDATI_STRM,     // Stream
};

bool SDAT_isSDAT(const char* filepath);
bool SDAT_getUniqueId(const char* filepath, char *buf);
bool SDAT_fakeNds(const char* filepath, nds_t *ndsdata);
bool SDAT_getFiles(const char* filepath, sdatfile_t *ndsfile, sdat_t* sdatfile);
void SDAT_close(sdat_t *sdatfile);
#endif
