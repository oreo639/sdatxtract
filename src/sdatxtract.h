#ifndef __SDATXTRACT_H__
#define __SDATXTRACT_H__

#include "main.h"
#include "nds.h"
#include "sdat.h"

bool extractAudio(const char *file);
bool SDATxtract(const char* filepath,  const char* outputdir_part1, NDS *ndsdata);
void outputFiles(const char* outputdir_part1, SDAT* sdatfile);
#endif
