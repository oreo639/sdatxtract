/**
 * nsstrm.c: nds strm functions
 * written by loveemu, feel free to redistribute
 */


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "cioutil.h"
#include "nssamp.h"
#include "nsswav.h"


#define WAVE_HEADER_SIZE    0x2c

NSSwav* nsSwavCreate(const byte* swav, size_t size)
{
  NSSwav* newSwav = NULL;

  if(size >= 0x18 
      && mget4l(&swav[0x00]) == 0x56415753 /* SWAV */
      && mget2l(&swav[0x04]) == 0xfeff
      && mget2l(&swav[0x0c]) == 0x0010
      && mget4l(&swav[0x10]) == 0x41544144 /* DATA */
  )
  {
    newSwav = nsSwavCreateFromSamp(&swav[0x18], size - 0x18);
  }
  return newSwav;
}

NSSwav* nsSwavCreateFromSamp(const byte* sampHeader, size_t size)
{
  NSSwav* newSwav = NULL;

  if(sampHeader && size >= 0x0c)
  {
    int waveType = mget1(&sampHeader[0x00]);
    bool hasLoop = (bool) mget1(&sampHeader[0x01]);
    int bps = nsSampGetBPSFromWaveType(waveType);
    int firstSampOfs = (waveType == NSSAMP_WAVE_ADPCM) ? 4 : 0;
    int byteToSampMul = (waveType == NSSAMP_WAVE_ADPCM) ? 2 : 1;
    int byteToSampDiv = (waveType == NSSAMP_WAVE_PCM16) ? 2 : 1;
    int loopStartInBytes = mget2l(&sampHeader[0x06]) * 4;
    int loopLenInBytes = mget4l(&sampHeader[0x08]) * 4;
    int loopStart = (loopStartInBytes - firstSampOfs) * byteToSampMul / byteToSampDiv;
    int loopLen = loopLenInBytes * byteToSampMul / byteToSampDiv;
    int numSamp = loopStart + loopLen;
    size_t dataOffset = 0x0c;
    size_t dataSize = loopStartInBytes + loopLenInBytes;
    byte* data;

    if(dataOffset + dataSize <= size)
    {
      data = (byte*) malloc(dataSize);
      if(data)
      {
        newSwav = (NSSwav*) calloc(1, sizeof(NSSwav));
        if(newSwav)
        {
          memcpy(data, &sampHeader[dataOffset], dataSize);

          newSwav->waveType = waveType;
          newSwav->hasLoop = hasLoop;
          newSwav->rate = mget2l(&sampHeader[0x02]);
          newSwav->time = mget2l(&sampHeader[0x04]);
          newSwav->loopStart = hasLoop ? loopStart : 0;
          newSwav->numSamp = numSamp;
          newSwav->data = data;
          newSwav->dataSize = dataSize;

          newSwav->bps = bps;
          newSwav->decodedSampSize = numSamp * (bps/8);
        }
        else
        {
          free(data);
        }
      }
    }
  }
  return newSwav;
}

void nsSwavDelete(NSSwav* swav)
{
  if(swav)
  {
    free(swav->data);
    free(swav);
  }
}

NSSwav* nsSwavReadFile(const char* path)
{
  NSSwav* newSwav = NULL;
  FILE* swavFile = fopen(path, "rb");

  if(swavFile)
  {
    size_t swavFileSize;
    byte* swavBuf;

    fseek(swavFile, 0, SEEK_END);
    swavFileSize = (size_t) ftell(swavFile);
    rewind(swavFile);

    swavBuf = (byte*) malloc(swavFileSize);
    if(swavBuf)
    {
      size_t readSize;

      readSize = fread(swavBuf, 1, swavFileSize, swavFile);
      if(readSize == swavFileSize)
      {
        newSwav = nsSwavCreate(swavBuf, swavFileSize);
      }
      free(swavBuf);
    }
    fclose(swavFile);
  }
  return newSwav;
}

size_t nsSwavGetWaveSize(NSSwav* swav)
{
  size_t waveSize = WAVE_HEADER_SIZE;

  if(swav)
  {
    waveSize += swav->decodedSampSize;
  }
  return waveSize;
}

bool nsSwavWriteToWave(NSSwav* swav, byte* buf, size_t bufSize)
{
  bool result = true;
  size_t waveSize = nsSwavGetWaveSize(swav);

  if(swav && bufSize >= waveSize)
  {
    int channels = 1;
    int rate = swav->rate;
    int bps = swav->bps;
    int decodedSampSize = swav->decodedSampSize;
/*
    int waveType = swav->waveType;
    int sampPerBlock = swav->numSamp;
    size_t lenBlock = sampPerBlock;
    int numBlocks = swav->numBlocks;
    size_t lenLastBlock = strm->lenLastBlock;
    int sampPerLastBlock = strm->sampPerLastBlock;
    size_t destOfs;
    size_t srcOfs;
    int blockId;
    const byte* data = swav->data;
*/

    mput4l(0x46464952, &buf[0x00]); /* RIFF */
    mput4l((int) (waveSize - 8), &buf[0x04]);
    mput4l(0x45564157, &buf[0x08]); /* WAVE */
    mput4l(0x20746d66, &buf[0x0c]); /* fmt  */
    mput4l(16, &buf[0x10]);
    mput2l(1, &buf[0x14]);
    mput2l(channels, &buf[0x16]);
    mput4l(rate, &buf[0x18]);
    mput4l(rate * channels * (bps/8), &buf[0x1c]);
    mput2l(channels * bps/8, &buf[0x20]);
    mput2l(bps, &buf[0x22]);
    mput4l(0x61746164, &buf[0x24]); /* data */
    mput4l(decodedSampSize, &buf[0x28]);

    nsSampDecodeBlock(&buf[WAVE_HEADER_SIZE], swav->data, swav->dataSize, 
        swav->numSamp, swav->waveType, channels);
/*
    destOfs = WAVE_HEADER_SIZE;
    srcOfs = 0;
    for(blockId = 0; blockId < (numBlocks-1); blockId++)
    {
      nsSampDecodeBlock(&buf[destOfs], &data[srcOfs], lenBlock, sampPerBlock, waveType, channels);
      destOfs += sampPerBlock * (bps/8) * channels;
      srcOfs += lenBlock * channels;
    }
    nsSampDecodeBlock(&buf[destOfs], &data[srcOfs], lenLastBlock, sampPerLastBlock, waveType, channels);
    destOfs += sampPerLastBlock * (bps/8) * channels;
    srcOfs += lenLastBlock * channels;
*/
  }
  return result;
}

bool nsSwavWriteToWaveFile(NSSwav* swav, const char* path)
{
  bool result = false;
  size_t waveSize;
  byte* wave;

  if(swav)
  {
    waveSize = nsSwavGetWaveSize(swav);
    wave = (byte*) malloc(waveSize);
    if(wave)
    {
      if(nsSwavWriteToWave(swav, wave, waveSize))
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
