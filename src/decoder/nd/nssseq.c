/**
 * nssseq.c: nds sseq functions
 * written by loveemu, feel free to redistribute
 */


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "cioutil.h"
#include "nssseq.h"


#define WAVE_HEADER_SIZE    0x2c

NSStrm* nsStrmCreate(const byte* strm, size_t size)
{
  NSStrm* newStrm = NULL;

  if(size >= 0x68 
      && mget4l(&strm[0x00]) == 0x4d525453 /* STRM */
      && mget2l(&strm[0x04]) == 0xfeff
      && mget2l(&strm[0x0c]) == 0x0010
      && mget4l(&strm[0x10]) == 0x44414548 /* HEAD */
  )
  {
    int channels = mget1(&strm[0x1a]);
    int numBlocks = mget4l(&strm[0x2c]);

    if(channels > 0 && numBlocks > 0)
    {
      size_t dataOffset = mget4l(&strm[0x28]);
      size_t lenBlockPerChan = mget4l(&strm[0x30]);
      size_t lenLastBlockPerChan = mget4l(&strm[0x38]);
      size_t lenBlock = lenBlockPerChan * channels;
      size_t lenLastBlock = lenLastBlockPerChan * channels;
      size_t dataSize = lenBlock * (numBlocks-1) + lenLastBlock;
      byte* data;

      if(dataOffset + dataSize <= size)
      {
        data = (byte*) malloc(dataSize);
        if(data)
        {
          newStrm = (NSStrm*) calloc(1, sizeof(NSStrm));
          if(newStrm)
          {
            int waveType = mget1(&strm[0x18]);
            bool hasLoop = (bool) mget1(&strm[0x19]);
            int numSamp = mget4l(&strm[0x24]);
            int bps = nsSampGetBPSFromWaveType(waveType);

            memcpy(data, &strm[dataOffset], dataSize);

            newStrm->waveType = waveType;
            newStrm->hasLoop = hasLoop;
            newStrm->channels = channels;
            newStrm->rate = mget2l(&strm[0x1c]);
            newStrm->time = mget2l(&strm[0x1e]);
            newStrm->loopStart = hasLoop ? mget4l(&strm[0x20]) : 0;
            newStrm->numSamp = numSamp;
            newStrm->numBlocks = numBlocks;
            newStrm->lenBlock = lenBlockPerChan;
            newStrm->sampPerBlock = mget4l(&strm[0x34]);
            newStrm->lenLastBlock = lenLastBlockPerChan;
            newStrm->sampPerLastBlock = mget4l(&strm[0x3c]);
            newStrm->data = data;
            newStrm->dataSize = dataSize;

            newStrm->bps = bps;
            newStrm->decodedSampSize = numSamp * (bps/8) * channels;
          }
          else
          {
            free(data);
          }
        }
      }
    }
  }
  return newStrm;
}

void nsStrmDelete(NSStrm* strm)
{
  if(strm)
  {
    free(strm->data);
    free(strm);
  }
}

NSStrm* nsStrmReadFile(const char* path)
{
  NSStrm* newStrm = NULL;
  FILE* strmFile = fopen(path, "rb");

  if(strmFile)
  {
    size_t strmFileSize;
    byte* strmBuf;

    fseek(strmFile, 0, SEEK_END);
    strmFileSize = (size_t) ftell(strmFile);
    rewind(strmFile);

    strmBuf = (byte*) malloc(strmFileSize);
    if(strmBuf)
    {
      size_t readSize;

      readSize = fread(strmBuf, 1, strmFileSize, strmFile);
      if(readSize == strmFileSize)
      {
        newStrm = nsStrmCreate(strmBuf, strmFileSize);
      }
      free(strmBuf);
    }
    fclose(strmFile);
  }
  return newStrm;
}

size_t nsStrmGetWaveSize(NSStrm* strm)
{
  size_t waveSize = WAVE_HEADER_SIZE;

  if(strm)
  {
    waveSize += strm->decodedSampSize;
  }
  return waveSize;
}

bool nsStrmWriteToWave(NSStrm* strm, byte* buf, size_t bufSize)
{
  bool result = true;
  size_t waveSize = nsStrmGetWaveSize(strm);

  if(strm && bufSize >= waveSize)
  {
    int waveType = strm->waveType;
    int channels = strm->channels;
    int rate = strm->rate;
    int bps = strm->bps;
    int decodedSampSize = strm->decodedSampSize;
    int numBlocks = strm->numBlocks;
    size_t lenBlock = strm->lenBlock;
    int sampPerBlock = strm->sampPerBlock;
    size_t lenLastBlock = strm->lenLastBlock;
    int sampPerLastBlock = strm->sampPerLastBlock;
    const byte* data = strm->data;
    size_t destOfs;
    size_t srcOfs;
    int blockId;

    mput4l(0x46464952, &buf[0x00]); /* RIFF */
    mput4l((int) (waveSize - 8), &buf[0x04]);
    mput4l(0x45564157, &buf[0x08]); /* WAVE */
    mput4l(0x20746d66, &buf[0x0c]); /* fmt  */
    mput4l(16, &buf[0x10]);
    mput2l(1, &buf[0x14]);
    mput2l(channels, &buf[0x16]);
    mput4l(rate, &buf[0x18]);
    mput4l(rate * channels * (bps/8), &buf[0x1c]);
    mput2l(bps/8, &buf[0x20]);
    mput2l(bps, &buf[0x22]);
    mput4l(0x61746164, &buf[0x24]); /* data */
    mput4l(decodedSampSize, &buf[0x28]);

    destOfs = WAVE_HEADER_SIZE;
    srcOfs = 0;
    for(blockId = 0; blockId < (numBlocks-1); blockId++)
    {
      nsSampDecodeBlock(&buf[destOfs], &data[srcOfs], lenBlock, sampPerBlock, waveType, channels);
      destOfs += sampPerBlock * (bps/8) * channels;
      srcOfs += lenBlock * channels;
    }
    nsSampDecodeBlock(&buf[destOfs], &data[srcOfs], lenLastBlock, sampPerLastBlock, waveType, channels);
#if 0
    destOfs += sampPerLastBlock * (bps/8) * channels;
    srcOfs += lenLastBlock * channels;
#endif
  }
  return result;
}

bool nsStrmWriteToWaveFile(NSStrm* strm, const char* path)
{
  bool result = false;
  size_t waveSize;
  byte* wave;

  if(strm)
  {
    waveSize = nsStrmGetWaveSize(strm);
    wave = (byte*) malloc(waveSize);
    if(wave)
    {
      if(nsStrmWriteToWave(strm, wave, waveSize))
      {
        FILE* waveFile = fopen(path, "wb");

        if(waveFile)
        {
          size_t writtenSize;

          writtenSize = fwrite(wave, 1, waveSize, waveFile);
          if(writtenSize == waveSize)
          {
            result = true;
          }

          fclose(waveFile);
        }
      }
      free(wave);
    }
  }
  return result;
}
