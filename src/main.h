#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/stat.h>

#include "file.h" //File i/o wraper.
#include "config.h" //Compile settings.

#define MAX_PATH 260

#define DIR_SSEQ "sequence"
#define DIR_SWAR "wave"
#define DIR_SBNK "bank"
#define DIR_STRM "stream"

// Verbose messages.
#define verbose(...) if(bVerboseMessages){printf(__VA_ARGS__);}

// Global variables.
extern bool bDecodeFile;
extern bool bVerboseMessages;
extern bool bExtractSdat;
