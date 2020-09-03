#ifndef __SX_NDS_H__
#define __SX_NDS_H__

typedef struct {
	uint8_t *sdatImage;
	uint32_t sdatsize;
} sdatfile_t;

typedef struct {
	sdatfile_t *sdatfile;
	uint32_t sdatnum;
} nds_t;

bool NDS_isNds(const char* filepath);
bool NDS_getGameTitle(const char* filepath, char *buf);
bool NDS_getGameCode(const char* filepath, char *buf);
bool NDS_getSDAToffset(const char* filepath, nds_t *ndsdata);
bool NDS_dumpSDAT(const char *filepath, const char* fileout, sdatfile_t *ndsfile);
#endif
