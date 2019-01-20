#ifndef __SX_NDS_H__
#define __SX_NDS_H__

typedef struct {
	uint8_t *sdatImage;
	uint32_t sdatsize;
} NDSfile_t;

typedef struct {
	NDSfile_t *ndsfile;
	uint32_t sdatnum;
} NDS;

bool NDS_isNds(const char* filepath);
bool NDS_getGameTitle(const char* filepath, char *buf);
bool NDS_getGameCode(const char* filepath, char *buf);
bool NDS_getSDAToffset(const char* filepath, NDS *ndsdata);
bool NDS_dumpSDAT(const char *filepath, const char* fileout, NDSfile_t *ndsfile);
#endif
