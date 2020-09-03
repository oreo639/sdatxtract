#ifndef __SDATXTRACT_MAIN_H__
#define __SDATXTRACT_MAIN_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include "file.h" //File i/o wraper.
#include "config.h" //Compile settings.

#include "macrotools.h"

#define MAX_PATH 260

#define DIR_SSEQ "sequence"
#define DIR_SWAR "wave"
#define DIR_SBNK "bank"
#define DIR_STRM "stream"

// Verbose messages.
#define verbose(...) if(bVerboseMessages){printf(__VA_ARGS__);}
#define processIndicator() printf(".");
#define DEBUG(...) printf("[" __FILE__ "] " STRINGIFY(__LINE__) ": " __VA_ARGS__)

static inline uint32_t getUint(uint8_t* data) {
	uint32_t a;
	memcpy(&a, data, sizeof(uint32_t));
	return a;
}

static inline uint16_t getShort(uint8_t* data) {
	uint16_t a;
	memcpy(&a, data, sizeof(uint16_t));
	return a;
}

static inline uint8_t getByte(uint8_t* data) {
	uint8_t a;
	memcpy(&a, data, sizeof(uint8_t));
	return a;
}

// Global variables.
extern bool bDecodeFile;
extern bool bVerboseMessages;
extern bool bExtractSdat;
extern bool bUseFname;
extern bool bGetSwav;
#endif
