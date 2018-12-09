/**
 * nssamp.h: nds sample decoder
 * written by loveemu, feel free to redistribute
 */


#ifndef NSSAMP_H
#define NSSAMP_H


#include "cioutil.h"


#define NSSAMP_WAVE_PCM8    0
#define NSSAMP_WAVE_PCM16   1
#define NSSAMP_WAVE_ADPCM   2

typedef struct TagNSSampADPCMInfo
{
  bool low;
  int samp;
  int stepIndex;
} NSSampADPCMInfo;

int nsSampGetBPSFromWaveType(int waveType);
bool nsSampDecodeBlock(byte* dest, const byte* blocks, size_t blockSize, int nSamples, int waveType, int channels);
size_t nsSampDecode(byte* dest, const byte* src, int waveType, NSSampADPCMInfo* adpcm);


#endif /* !NSSAMP_H */
