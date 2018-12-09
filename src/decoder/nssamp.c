/**
 * nssamp.c: nds sample decoder
 * written by loveemu, feel free to redistribute
 */


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "cioutil.h"
#include "nssamp.h"


#define WAVE_HEADER_SIZE    0x2c


typedef struct TagNSSampDecBlockInfo
{
  size_t transferedSize;
  NSSampADPCMInfo adpcm;
} NSSampDecBlockInfo;


void clamp_step_index(int* stepIndex);
void clamp_sample(int* decompSample);
void process_nibble(unsigned char code, int* stepIndex, int* decompSample);


int nsSampGetBPSFromWaveType(int waveType)
{
  return (waveType == NSSAMP_WAVE_PCM8) ? 8 : 16;
}

bool nsSampDecodeBlock(byte* dest, const byte* blocks, size_t blockSize, int nSamples, int waveType, int channels)
{
  bool result = false;
  NSSampDecBlockInfo* info;

  info = (NSSampDecBlockInfo*) calloc(channels, sizeof(NSSampDecBlockInfo));
  if(info)
  {
    int ch;
    int sampId;
    size_t blockHead;
    size_t transferedSize;
    size_t destOfs = 0;
    int bps = nsSampGetBPSFromWaveType(waveType);

    blockHead = 0;
    for(ch = 0; ch < channels; ch++)
    {
      switch(waveType)
      {
      case NSSAMP_WAVE_ADPCM:
        info[ch].transferedSize = 4;
        info[ch].adpcm.low = true;
        info[ch].adpcm.samp = utos2(mget2l(&blocks[blockHead]));
        info[ch].adpcm.stepIndex = mget1(&blocks[blockHead + 2]);
        break;
      }
      blockHead += blockSize;
    }

    for(sampId = 0; sampId < nSamples; sampId++)
    {
      blockHead = 0;
      for(ch = 0; ch < channels; ch++)
      {
        transferedSize = info[ch].transferedSize;

        transferedSize += nsSampDecode(&dest[destOfs], 
          &blocks[blockHead + transferedSize], waveType, &info[ch].adpcm);
        destOfs += bps/8;

        info[ch].transferedSize = transferedSize;
        blockHead += blockSize;
      }
    }

    result = true;
    free(info);
  }
  return result;
}

size_t nsSampDecode(byte* dest, const byte* src, int waveType, NSSampADPCMInfo* adpcm)
{
  size_t transferedSize = 0;

  switch(waveType)
  {
    case NSSAMP_WAVE_PCM8:
    dest[0] = src[0] ^ 0x80;
    transferedSize++;
    break;

    case NSSAMP_WAVE_PCM16:
      memcpy(dest, src, 2);
      transferedSize += 2;
      break;

    case NSSAMP_WAVE_ADPCM:
    {
      bool low = adpcm->low;
      byte code = src[0];

      if(!low)
      {
        code = code >> 4;
        transferedSize++;
      }
      process_nibble(code, &adpcm->stepIndex, &adpcm->samp);
      mput2l(adpcm->samp, dest);
      adpcm->low = !low;
      break;
    }
  }

  return transferedSize;
}


void clamp_step_index(int* stepIndex)
{
  if (*stepIndex < 0 ) *stepIndex = 0;
  if (*stepIndex > 88) *stepIndex = 88;
}

void clamp_sample(int* decompSample)
{
  if (*decompSample < -32768) *decompSample = -32768;
  if (*decompSample >  32767) *decompSample =  32767;
}

void process_nibble(unsigned char code, int* stepIndex, int* decompSample)
{
  const unsigned ADPCMTable[89] = 
  {
    7, 8, 9, 10, 11, 12, 13, 14,
    16, 17, 19, 21, 23, 25, 28, 31,
    34, 37, 41, 45, 50, 55, 60, 66,
    73, 80, 88, 97, 107, 118, 130, 143,
    157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658,
    724, 796, 876, 963, 1060, 1166, 1282, 1411,
    1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
    3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
    7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
    32767
  };

  const int IMA_IndexTable[16] = 
  {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8 
  };

  unsigned step;
  int diff;

  code &= 0x0F;

  step = ADPCMTable[*stepIndex];
  diff = step >> 3;
  if (code & 1) diff += step >> 2;
  if (code & 2) diff += step >> 1;
  if (code & 4) diff += step;
  // Windows:
  //if (code & 8) *decompSample -= diff;
  //else          *decompSample += diff;
  //if (*decompSample < -32768) *decompSample = -32768;
  //if (*decompSample >  32767) *decompSample = 32767;
  // Nitro: (has minor clipping-error, see GBATEK for details)
  if (code & 8) {
    *decompSample -= diff;
    if (*decompSample < -32767)
      *decompSample = -32767;
  }
  else {
    *decompSample += diff;
    if (*decompSample > 32767)
      *decompSample = 32767;
  }
  (*stepIndex) += IMA_IndexTable[code];
  clamp_step_index(stepIndex);
}
