/**
 * nsswav.h: nds swav functions
 * written by loveemu, feel free to redistribute
 */


#ifndef NSSWAV_H
#define NSSWAV_H


#include "cioutil.h"


typedef struct TagNSSwav
{
  int waveType;
  bool hasLoop;
  int rate;
  int time;
  int loopStart;
  int numSamp;
  int bps;
  int decodedSampSize;
  byte* data;
  size_t dataSize;
} NSSwav;

NSSwav* nsSwavCreate(const byte* swav, size_t size);
NSSwav* nsSwavCreateFromSamp(const byte* sampHeader, size_t size);
void nsSwavDelete(NSSwav* swav);
NSSwav* nsSwavReadFile(const char* path);
size_t nsSwavGetWaveSize(NSSwav* swav);
bool nsSwavWriteToWave(NSSwav* swav, byte* buf, size_t bufSize);
bool nsSwavWriteToWaveFile(NSSwav* swav, const char* path);


#endif /* !NSSWAV_H */
