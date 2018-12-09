/**
 * nssseq.h: nds sseq functions
 * written by loveemu, feel free to redistribute
 */


#ifndef NSSSEQ_H
#define NSSSEQ_H


#include "cioutil.h"


typedef struct TagNSSseq
{
  int waveType;
  bool hasLoop;
  int channels;
  int rate;
  int time;
  int loopStart;
  int numSamp;
  int numBlocks;
  size_t lenBlock;
  int sampPerBlock;
  size_t lenLastBlock;
  int sampPerLastBlock;
  int bps;
  int decodedSampSize;
  byte* data;
  size_t dataSize;
} NSSseq;

NSStrm* nsSseqCreate(const byte* strm, size_t size);
void nsSseqDelete(NSStrm* strm);
NSStrm* nsSseqReadFile(const char* path);
size_t nsStrmGetWaveSize(NSStrm* strm);
bool nsSseqWriteToWave(NSStrm* strm, byte* buf, size_t bufSize);
bool nsSseqWriteToWaveFile(NSStrm* strm, const char* path);


#endif /* !NSSSEQ_H */
