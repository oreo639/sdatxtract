#ifndef __SX_SDAT_H__
#define __SX_SDAT_H__

#include "nds.h"

typedef struct {
	//filename
	char **name;
	//Max number of filenames.
	uint32_t fnamemax;
	//file ID's, may be nessary later.
	uint16_t *ident;
	//this is specificly for sdat.
	uint16_t banks;
	
	uint32_t numFiles;
} SDAT_META;

typedef struct {
	uint8_t *file;
	uint32_t fileoffset;
	uint32_t filesize;
} SDAT_FILE;

typedef struct {
	//SYMB
	SDAT_META sseqName;
	SDAT_META swarName;
	SDAT_META sbnkName;
	SDAT_META strmName;

	//FAT
	SDAT_FILE* sseqfile;
	SDAT_FILE* swarfile;
	SDAT_FILE* sbnkfile;
	SDAT_FILE* strmfile;
	uint32_t files;
} SDAT;

bool SDAT_isSDAT(const char* filepath);
bool SDAT_getUniqueId(const char* filepath, char *buf);
bool SDAT_fakeNds(const char* filepath, NDS *ndsdata);
bool SDAT_getFiles(const char* filepath, NDSfile_t *ndsfile, SDAT* sdatfile);
void SDAT_outputFiles(const char* filepath, const char* outputdir_part1, SDAT* sdatfile);
void SDAT_close(SDAT *sdatfile);
#endif
