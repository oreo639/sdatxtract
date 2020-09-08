#ifndef __SDATXTRACT_MAIN_H__
#define __SDATXTRACT_MAIN_H__

#include <string>
#include <vector>
#include <filesystem>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "config.h" //Compile settings.
#include "common.hpp"

#include "htools/macrotools.h"

#define MAX_PATH 260

#define DIR_SSEQ "sequence"
#define DIR_SWAR "wave"
#define DIR_SBNK "bank"
#define DIR_STRM "stream"

// Verbose messages.
#define verbose(...) if(bVerboseMessages){printf(__VA_ARGS__);}
#define processIndicator() printf(".");
#define DEBUG(...) printf("[" __FILE__ "] " STRINGIFY(__LINE__) ": " __VA_ARGS__)

#define error(...) printf("[Err]: " __VA_ARGS__)
#define warning(...) printf("[Warn]: " __VA_ARGS__)

namespace File {
	static inline bool Exists(const std::string &filename) {
		FILE *fp = fopen(filename.c_str(), "rb");

		if(!fp) {
			return false;
		}

		fclose(fp);

		return true;
	}
}

namespace fs = std::filesystem;

// Global variables.
extern bool bDecodeFile;
extern bool bVerboseMessages;
extern bool bExtractSdat;
extern bool bUseFname;
extern bool bGetSwav;
#endif
