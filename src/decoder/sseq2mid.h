/**
 * sseq2mid.h: convert sseq into standard midi
 * presented by loveemu, feel free to redistribute
 */

#ifndef SSEQ2MID_H
#define SSEQ2MID_H


#include <stddef.h>
#include "libsmfc.h"
#include "libsmfcx.h"


#define SSEQ_INVALID_OFFSET     -1

typedef struct TagSseq2midTrackState
{
  int loopCount;
  int absTime;
  bool noteWait;
  size_t curOffset;
  size_t offsetToTop;
  size_t offsetToReturn;
  int offsetToAbsTime[ 262144 ]; // XXX
} Sseq2midTrackState;


#define SSEQ_MAX_TRACK          16

typedef void (Sseq2midLogProc)(const char*);

typedef struct TagSseq2mid
{
  byte* sseq;
  size_t sseqSize;
  Smf* smf;
  Sseq2midTrackState track[SSEQ_MAX_TRACK];
  Sseq2midLogProc* logProc;
  int chOrder[SSEQ_MAX_TRACK];
  bool modifyChOrder;
  bool noReverb;
  int loopCount;
} Sseq2mid;

Sseq2mid* sseq2midCreate(const byte* sseq, size_t sseqSize, bool modifyChOrder);
Sseq2mid* sseq2midCreateFromFile(const char* filename, bool modifyChOrder);
void sseq2midDelete(Sseq2mid* sseq2mid);
Sseq2mid* sseq2midCopy(Sseq2mid* sseq2mid);
bool sseq2midConvert(Sseq2mid* sseq2mid);
size_t sseq2midWriteMidi(Sseq2mid* sseq2mid, byte* buffer, size_t bufferSize);
size_t sseq2midWriteMidiFile(Sseq2mid* sseq2mid, const char* filename);
void sseq2midSetLogProc(Sseq2mid* sseq2mid, Sseq2midLogProc* logProc);
bool sseq2midNoReverb(Sseq2mid* sseq2mid, bool noReverb);
int sseq2midSetLoopCount(Sseq2mid* sseq2mid, int loopCount);


#endif /* !SSEQ2MID_H */
