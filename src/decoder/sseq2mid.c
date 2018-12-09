/**
 * sseq2mid.c: convert sseq into standard midi
 * presented by loveemu, feel free to redistribute
 * see sseq2midConvert to know conversion detail :)
 * Minor change to remove the main function an all other unnessary functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sseq2mid.h"


#ifndef countof
#define countof(a)  (sizeof(a) / sizeof(a[0]))
#endif

#define SSEQ2MID_NAME   "sseq2mid"
#define SSEQ2MID_VER    "20070314"

bool g_log = false;
bool g_modifyChOrder = false;
bool g_noReverb = false;
int g_loopCount = 1; 
int g_loopStyle = 0;

void dispatchLogMsg(const char* logMsg);


void sseq2midPutLog(Sseq2mid* sseq2mid, const char* logMessage);
void sseq2midPutLogLine(Sseq2mid* sseq2mid, size_t offset, size_t size, 
  const char* description, const char* comment);
int sseq2midSseqChToMidiCh(Sseq2mid* sseq2mid, int sseqChannel);

int getS1From(byte* data);
int getS2LitFrom(byte* data);
int getS3LitFrom(byte* data);
int getS4LitFrom(byte* data);
unsigned int getU1From(byte* data);
unsigned int getU2LitFrom(byte* data);
unsigned int getU3LitFrom(byte* data);
unsigned int getU4LitFrom(byte* data);


/* dispatch log message */
void dispatchLogMsg(const char* logMsg)
{
  printf(logMsg);   /* output to stdout */
}

/* call the fuction to put log message */
void sseq2midPutLog(Sseq2mid* sseq2mid, const char* logMessage)
{
  if(sseq2mid && sseq2mid->logProc)
  {
    sseq2mid->logProc(logMessage);
  }
}

#define SSEQ2MID_MAX_DUMP   5

/* put log message in prescribed form */
void sseq2midPutLogLine(Sseq2mid* sseq2mid, size_t offset, size_t size, 
  const char* description, const char* comment)
{
  if(sseq2mid)
  {
    char logMsg[96];
    char hexDump[24];
    char hexDumpPart[8];
    size_t sizeToTransfer;
    size_t transferedSize;
    byte* sseq = sseq2mid->sseq;
    size_t sseqSize = sseq2mid->sseqSize;

    sizeToTransfer = (offset + size <= sseqSize) ? size : sseqSize - offset;
    sizeToTransfer = (sizeToTransfer < SSEQ2MID_MAX_DUMP) ? sizeToTransfer : SSEQ2MID_MAX_DUMP;

    strcpy(hexDump, "");
    for(transferedSize = 0; transferedSize < sizeToTransfer; transferedSize++)
    {
      if(transferedSize != 0)
      {
        strcat(hexDump, " ");
      }
      sprintf(hexDumpPart, "%02X", sseq[offset + transferedSize]);
      strcat(hexDump, hexDumpPart);
    }

    sprintf(logMsg, "%08X: %-14s | %-20s | %s\n", offset, hexDump, 
      description ? description : "", comment ? comment : "");
    sseq2midPutLog(sseq2mid, logMsg);
  }
}

/* filter: sseq channel number to midi track number */
int sseq2midSseqChToMidiCh(Sseq2mid* sseq2mid, int sseqChannel)
{
  return ((sseqChannel >= 0) && (sseqChannel <= SSEQ_MAX_TRACK)) ? sseq2mid->chOrder[sseqChannel] : sseqChannel;
}

/* create sseq2mid object */
Sseq2mid* sseq2midCreate(const byte* sseq, size_t sseqSize, bool modifyChOrder)
{
  Sseq2mid* newSseq2mid = (Sseq2mid*) calloc(1, sizeof(Sseq2mid));

  if(newSseq2mid)
  {
    newSseq2mid->sseq = (byte*) malloc(sseqSize);
    if(newSseq2mid->sseq)
    {
      newSseq2mid->smf = smfCreate();
      if(newSseq2mid->smf)
      {
        memcpy(newSseq2mid->sseq, sseq, sseqSize);
        newSseq2mid->sseqSize = sseqSize;

        smfSetTimebase(newSseq2mid->smf, 48);
        newSseq2mid->loopCount = 1;
        newSseq2mid->noReverb = false;
        newSseq2mid->modifyChOrder = modifyChOrder;
      }
      else
      {
        free(newSseq2mid->sseq);
        free(newSseq2mid);
        newSseq2mid = NULL;
      }
    }
    else
    {
      free(newSseq2mid);
      newSseq2mid = NULL;
    }
  }
  return newSseq2mid;
}

/* create sseq2mid object from file */
Sseq2mid* sseq2midCreateFromFile(const char* filename, bool modifyChOrder)
{
  Sseq2mid* newSseq2mid = NULL;
  FILE* sseqFile = fopen(filename, "rb");

  if(sseqFile)
  {
    size_t sseqSize;
    byte* sseq;

    fseek(sseqFile, 0, SEEK_END);
    sseqSize = (size_t) ftell(sseqFile);
    rewind(sseqFile);

    sseq = (byte*) malloc(sseqSize);
    if(sseq)
    {
      fread(sseq, sseqSize, 1, sseqFile);
      newSseq2mid = sseq2midCreate(sseq, sseqSize, modifyChOrder);
      free(sseq);
    }

    fclose(sseqFile);
  }
  return newSseq2mid;
}

/* delete sseq2mid object */
void sseq2midDelete(Sseq2mid* sseq2mid)
{
  if(sseq2mid)
  {
    smfDelete(sseq2mid->smf);
    free(sseq2mid->sseq);
    free(sseq2mid);
  }
}

/* copy sseq2mid object */
Sseq2mid* sseq2midCopy(Sseq2mid* sseq2mid)
{
  Sseq2mid* newSseq2mid = NULL;

  if(sseq2mid)
  {
    newSseq2mid = sseq2midCreate(sseq2mid->sseq, sseq2mid->sseqSize, sseq2mid->modifyChOrder);
    if(newSseq2mid)
    {
      smfDelete(newSseq2mid->smf);
      newSseq2mid->smf = smfCopy(sseq2mid->smf);
      if(newSseq2mid->smf)
      {
        sseq2midSetLogProc(newSseq2mid, sseq2mid->logProc);
        sseq2midSetLoopCount(newSseq2mid, sseq2mid->loopCount);
      }
      else
      {
        sseq2midDelete(newSseq2mid);
        newSseq2mid = NULL;
      }
    }
  }
  return newSseq2mid;
}

#define SSEQ_MIN_SIZE   0x1d

/* sseq2mid conversion main, enjoy my dirty code :P */
bool sseq2midConvert(Sseq2mid* sseq2mid)
{
  bool result = false;
  char strForLog[64];
  int loopStartCount;
  size_t loopStartOffset;
  bool loopPointUsed = false;
  bool loopStartPointUsed = false;
  bool loopEndPointUsed = false;

  if(sseq2mid)
  {
    byte* sseq = sseq2mid->sseq;
    size_t sseqSize = sseq2mid->sseqSize;
    Smf* smf = sseq2mid->smf;

    if((sseqSize >= SSEQ_MIN_SIZE) && 
        (sseq[0x00] == 'S') && (sseq[0x01] == 'S') && (sseq[0x02] == 'E') && (sseq[0x03] == 'Q') && 
        (sseq[0x10] == 'D') && (sseq[0x11] == 'A') && (sseq[0x12] == 'T') && (sseq[0x13] == 'A'))
    {
      int trackIndex;
      int midiCh;
      size_t sseqOffsetBase;
      int midiChOrder[SSEQ_MAX_TRACK] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 9 };

      /* put SSEQ header info */
      sseq2midPutLogLine(sseq2mid, 0x00, 4, "Signature", "SSEQ");
      sseq2midPutLogLine(sseq2mid, 0x04, 2, "", "Unknown");
      sseq2midPutLogLine(sseq2mid, 0x06, 2, "", "Unknown");
      sprintf(strForLog, "%u", getU4LitFrom(&sseq[0x08]));
      sseq2midPutLogLine(sseq2mid, 0x08, 4, "SSEQ file size", strForLog);
      sseq2midPutLogLine(sseq2mid, 0x0c, 2, "", "Unknown");
      sseq2midPutLogLine(sseq2mid, 0x0e, 2, "", "Unknown");
      sseq2midPutLog(sseq2mid, "\n");

      /* put DATA chunk header */
      sseq2midPutLogLine(sseq2mid, 0x10, 4, "Signature", "DATA");
      sprintf(strForLog, "%u", getU4LitFrom(&sseq[0x14]));
      sseq2midPutLogLine(sseq2mid, 0x14, 4, "DATA chunk size", strForLog);
      sseqOffsetBase = (size_t) getU4LitFrom(&sseq[0x18]);
      sprintf(strForLog, "%08X", sseqOffsetBase);
      sseq2midPutLogLine(sseq2mid, 0x18, 4, "Offset Base", strForLog);
      sseq2midPutLog(sseq2mid, "\n");

      /* initialize channel order */
      for(midiCh = 0; midiCh < SSEQ_MAX_TRACK; midiCh++)
      {
        sseq2mid->chOrder[midiCh] = sseq2mid->modifyChOrder ? midiChOrder[midiCh] : midiCh;
      }

      /* initialize track settings */
      sseq2mid->track[0].loopCount = sseq2mid->loopCount;
      sseq2mid->track[0].absTime = 0;
      sseq2mid->track[0].noteWait = false;
      sseq2mid->track[0].offsetToTop = 0x1c;
      sseq2mid->track[0].offsetToReturn = SSEQ_INVALID_OFFSET;
      sseq2mid->track[0].curOffset = sseq2mid->track[0].offsetToTop;
      for(trackIndex = 1; trackIndex < SSEQ_MAX_TRACK; trackIndex++)
      {
        sseq2mid->track[trackIndex].loopCount = 0;  /* inactive */
        sseq2mid->track[trackIndex].noteWait = false;
      }

      /* initialize midi */
#if 0
      smfInsertGM1SystemOn(smf, 0, 0, 0);
#endif

      /* convert each track */
      result = true;
      for(trackIndex = 0; trackIndex < SSEQ_MAX_TRACK; trackIndex++)
      {
        int loopCount = sseq2mid->track[trackIndex].loopCount;

        if(loopCount > 0)
        {
          do
          {
            int absTime = sseq2mid->track[trackIndex].absTime;
            size_t curOffset = sseq2mid->track[trackIndex].curOffset;
            size_t eventOffset = curOffset;
            char eventName[64];
            char eventDesc[64];
            bool eventException;
            size_t offsetToJump = SSEQ_INVALID_OFFSET;

            midiCh = sseq2midSseqChToMidiCh(sseq2mid, trackIndex);
            sprintf(eventName, "Access Violation");
            sprintf(eventDesc, "End of File at %08X", sseqSize);
            eventException = true;

            if(curOffset < sseqSize)
            {
              byte statusByte;
          
              sseq2mid->track[trackIndex].offsetToAbsTime[curOffset] = absTime;

              statusByte = getU1From(&sseq[curOffset]);
              curOffset++;

              sprintf(eventName, "Unknown Event %02X", statusByte);
              sprintf(eventDesc, "");
              eventException = false;

              if(statusByte < 0x80)
              {
                int velocity;
                int duration;
                const char* noteName[] = {
                  "C ", "C#", "D ", "D#", "E ", "F ", 
                  "F#", "G ", "G#", "A ", "A#", "B "
                };

                velocity = getU1From(&sseq[curOffset]);
                curOffset++;
                duration = smfReadVarLength(&sseq[curOffset], sseqSize - curOffset);
                curOffset += smfGetVarLengthSize(duration);

                smfInsertNote(smf, absTime, midiCh, midiCh, statusByte, velocity, duration);
                if(sseq2mid->track[trackIndex].noteWait)
                {
                  absTime += duration;
                }

                sprintf(eventName, "Note with Duration");
                sprintf(eventDesc, "%s %d [%d]  vel:%-3d dur:%-3d", noteName[statusByte % 12], 
                  (statusByte / 12) - 1, statusByte, velocity, duration);
              }
              else
              {
                switch(statusByte)
                {
                case 0x80:
                {
                  int tick;

                  tick = smfReadVarLength(&sseq[curOffset], sseqSize - curOffset);
                  curOffset += smfGetVarLengthSize(tick);

                  absTime += tick;

                  sprintf(eventName, "Rest");
                  sprintf(eventDesc, "%d", tick);
                  break;
                }

                case 0x81:
                {
                  int realProgram;
                  int bankMsb;
                  int bankLsb;
                  int program;

                  realProgram = smfReadVarLength(&sseq[curOffset], sseqSize - curOffset);
                  curOffset += smfGetVarLengthSize(realProgram);

                  program = realProgram % 128;
                  bankLsb = (realProgram / 128) % 128;
                  bankMsb = (realProgram / 128 / 128) % 128;
                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_BANKSELM, bankMsb);
                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_BANKSELL, bankLsb);
                  smfInsertProgram(smf, absTime, midiCh, midiCh, program);

                  sprintf(eventName, "Program Change");
                  sprintf(eventDesc, "%d", realProgram);
                  break;
                }

                case 0x93: /* ('A`) */
                {
                  int newTrackIndex;
                  int offset;

                  newTrackIndex = getU1From(&sseq[curOffset]);
                  curOffset += 1;
                  offset = getU3LitFrom(&sseq[curOffset]) + sseqOffsetBase;
                  curOffset += 3;

                  sseq2mid->track[newTrackIndex].loopCount = loopCount;
                  sseq2mid->track[newTrackIndex].absTime = absTime;
                  sseq2mid->track[newTrackIndex].offsetToTop = offset;
                  sseq2mid->track[newTrackIndex].offsetToReturn = SSEQ_INVALID_OFFSET;
                  sseq2mid->track[newTrackIndex].curOffset = offset;

                  sprintf(eventName, "Open Track");
                  sprintf(eventDesc, "Track %02d at %08Xh", newTrackIndex + 1, offset);
                  break;
                }

                case 0x94:
                {
                  int newOffset;

                  newOffset = getU3LitFrom(&sseq[curOffset]) + sseqOffsetBase;
                  curOffset += 3;

                  offsetToJump = newOffset;

                  if(offsetToJump >= sseq2mid->track[trackIndex].offsetToTop)
                  {
                    if(offsetToJump < curOffset)
                    {
                      switch(g_loopStyle)
                      {
                      case 0:
                        loopCount--;
                        break;
                      
                      case 1: 
                        if(!loopPointUsed)
                        {
                            smfInsertControl(smf, sseq2mid->track[trackIndex].offsetToAbsTime[offsetToJump], midiCh, midiCh, 0x74, 0);
                            smfInsertControl(smf, absTime, midiCh, midiCh, 0x75, 0);
                            loopPointUsed = true;
                        }
                        loopCount = 0;
                        break;

                      case 2:
                        if(!loopPointUsed)
                        {
                            smfInsertMetaEvent(smf, sseq2mid->track[trackIndex].offsetToAbsTime[offsetToJump], midiCh, 6, "loopStart", 9);
                            smfInsertMetaEvent(smf, absTime, midiCh, 6, "loopEnd", 7);
                            loopPointUsed = true;
                        }
                        loopCount = 0;
                        break;
                      }
                    }
                    else
                    {
                      /* jump to forward */
                    }
                  }
                  else
                  {
                    /* redirect */
                  }

                  sprintf(eventName, "Jump");
                  sprintf(eventDesc, "%08X", newOffset);
                  break;
                }

                case 0x95:
                {
                  int newOffset;

                  newOffset = getU3LitFrom(&sseq[curOffset]) + sseqOffsetBase;
                  curOffset += 3;

                  sseq2mid->track[trackIndex].offsetToReturn = curOffset;
                  offsetToJump = newOffset;

                  sprintf(eventName, "Call");
                  sprintf(eventDesc, "%08X", newOffset);
                  break;
                }

                case 0xa0: /* Hanjuku Hero DS: NSE_45, New Mario Bros: BGM_AMB_CHIKA, Slime Morimori Dragon Quest 2: SE_187, SE_210, Advance Wars */
                {
                  byte subStatusByte;
                  int randMin;
                  int randMax;

                  subStatusByte = getU1From(&sseq[curOffset]);
                  curOffset++;
                  randMin = getU2LitFrom(&sseq[curOffset]);
                  curOffset += 2;
                  randMax = getU2LitFrom(&sseq[curOffset]);
                  curOffset += 2;

                  /* TODO: implement */

                  sprintf(eventName, "Random (%02X)", subStatusByte);
                  sprintf(eventDesc, "Min:%d Max:%d", randMin, randMax);
                  break;
                }

                case 0xa1: /* New Mario Bros: BGM_AMB_SABAKU */
                {
                  byte subStatusByte;
                  int varNumber;

                  subStatusByte = getU1From(&sseq[curOffset]);
                  curOffset++;
                  if(subStatusByte >= 0xb0 && subStatusByte <= 0xbd) /* var */
                  {
                    /* loveemu is a lazy person :P */
                    curOffset++;
                    varNumber = getU1From(&sseq[curOffset]);
                    curOffset++;
                  }
                  else
                  {
                    varNumber = getU1From(&sseq[curOffset]);
                    curOffset++;
                  }

                  /* TODO: implement */

                  sprintf(eventName, "From Var (%02X)", subStatusByte);
                  sprintf(eventDesc, "var %d", varNumber);
                  break;
                }

                case 0xa2:
                {
                  /* TODO: implement */

                  sprintf(eventName, "If");
                  sprintf(eventDesc, "");
                  break;
                }

                case 0xb0: /* Children of Mana: SEQ_BGM001 */
                case 0xb1: /* Advance Wars - Dual Strike: SE_TAGPT_COUNT01 */
                case 0xb2:
                case 0xb3:
                case 0xb4:
                case 0xb5:
                case 0xb6: /* Mario Kart DS: 76th sequence */
                case 0xb8: /* Tottoko Hamutaro: MUS_ENDROOL, Nintendogs */
                case 0xb9:
                case 0xba:
                case 0xbb:
                case 0xbc:
                case 0xbd:
                {
                  int varNumber;
                  int var;
                  const char* varMethodName[] = {
                    "=", "+=", "-=", "*=", "/=", "[Shift]", "[Rand]", "", 
                    "==", ">=", ">", "<=", "<", "!="
                  };

                  varNumber = getU1From(&sseq[curOffset]);
                  curOffset++;
                  var = getU2LitFrom(&sseq[curOffset]);
                  curOffset += 2;

                  /* TODO: implement */

                  sprintf(eventName, "Variable %s", varMethodName[statusByte - 0xb0]);
                  sprintf(eventDesc, "var %d : %d", varNumber, var);
                  break;
                }

                case 0xc0:
                {
                  int pan;

                  pan = getU1From(&sseq[curOffset]);
                  curOffset++;

                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_PANPOT, pan);

                  sprintf(eventName, "Pan");
                  sprintf(eventDesc, "%d", pan - 64);
                  break;
                }

                case 0xc1:
                {
                  int vol;

                  vol = getU1From(&sseq[curOffset]);
                  curOffset++;

                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_VOLUME, vol);

                  sprintf(eventName, "Volume");
                  sprintf(eventDesc, "%d", vol);
                  break;
                }

                case 0xc2: /* Dawn of Sorrow: SDL_BGM_BOSS1_ */
                {
                  int vol;

                  vol = getU1From(&sseq[curOffset]);
                  curOffset++;

                  smfInsertMasterVolume(smf, absTime, 0, midiCh, vol);

                  sprintf(eventName, "Master Volume");
                  sprintf(eventDesc, "%d", vol);
                  break;
                }

                case 0xc3: /* Puyo Pop Fever 2: BGM00 */
                {
                  int transpose;

                  transpose = getS1From(&sseq[curOffset]);
                  curOffset++;

                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_RPNM, 0);
                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_RPNL, 2);
                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_DATAENTRYM, 64 + transpose);

                  sprintf(eventName, "Transpose");
                  sprintf(eventDesc, "%d", transpose);
                  break;
                }

                case 0xc4:
                {
                  int bend;

                  bend = getS1From(&sseq[curOffset]) * 64;
                  curOffset++;

                  smfInsertPitchBend(smf, absTime, midiCh, midiCh, bend);

                  sprintf(eventName, "Pitch Bend");
                  sprintf(eventDesc, "%d", bend);
                  break;
                }

                case 0xc5:
                {
                  int range;

                  range = getU1From(&sseq[curOffset]);
                  curOffset++;

                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_RPNM, 0);
                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_RPNL, 0);
                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_DATAENTRYM, range);

                  sprintf(eventName, "Pitch Bend Range");
                  sprintf(eventDesc, "%d", range);
                  break;
                }

                case 0xc6: /* Children of Mana: SEQ_BGM000 */
                {
                  int priority;

                  priority = getU1From(&sseq[curOffset]);
                  curOffset++;

                  sprintf(eventName, "Priority");
                  sprintf(eventDesc, "%d", priority);
                  break;
                }

                case 0xc7: /* Dawn of Sorrow: SDL_BGM_ARR1_ */
                {
                  int flg;

                  flg = getU1From(&sseq[curOffset]);
                  curOffset++;

                  smfInsertControl(smf, absTime, midiCh, midiCh, flg ? SMF_CONTROL_MONO : SMF_CONTROL_POLY, 0);
                  sseq2mid->track[trackIndex].noteWait = flg ? true : false;

                  sprintf(eventName, "Mono/Poly");
                  sprintf(eventDesc, "%s (%d)", flg ? "Mono" : "Poly", flg);
                  break;
                }

                case 0xc8: /* Hanjuku Hero DS: NSE_42 */
                {
                  int flg;

                  flg = getU1From(&sseq[curOffset]);
                  curOffset++;

                  /* TODO: implement */

                  sprintf(eventName, "Tie");
                  sprintf(eventDesc, "%s (%d)", flg ? "On" : "Off", flg);
                  break;
                }

                case 0xc9: /* Hanjuku Hero DS: NSE_50 */
                {
                  int key;

                  key = getU1From(&sseq[curOffset]);
                  curOffset++;

                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_PORTAMENTOCTRL, key);

                  sprintf(eventName, "Portamento Control");
                  sprintf(eventDesc, "%d", key);
                  break;
                }

                case 0xca: /* Dawn of Sorrow: SDL_BGM_ARR1_ */
                {
                  int amount;

                  amount = getU1From(&sseq[curOffset]);
                  curOffset++;

                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_MODULATION, amount);

                  sprintf(eventName, "Modulation Depth");
                  sprintf(eventDesc, "%d", amount);
                  break;
                }

                case 0xcb: /* Dawn of Sorrow: SDL_BGM_ARR1_ */
                {
                  int amount;

                  amount = getU1From(&sseq[curOffset]);
                  curOffset++;

                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_VIBRATORATE, 64 + amount / 2);

                  sprintf(eventName, "Modulation Speed");
                  sprintf(eventDesc, "%d", amount);
                  break;
                }

                case 0xcc: /* Children of Mana: SEQ_BGM001 */
                {
                  int type;
                  char* typeStr[] = { "Pitch", "Volume", "Pan" };

                  type = getU1From(&sseq[curOffset]);
                  curOffset++;

                  sprintf(eventName, "Modulation Type");
                  sprintf(eventDesc, "%s", typeStr[type]);
                  break;
                }

                case 0xcd: /* Phoenix Wright: BGM021 */
                {
                  int amount;

                  amount = getU1From(&sseq[curOffset]);
                  curOffset++;

                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_VIBRATODEPTH, 64 + amount / 2);

                  sprintf(eventName, "Modulation Range");
                  sprintf(eventDesc, "%d", amount);
                  break;
                }

                case 0xce: /* Dawn of Sorrow: SDL_BGM_ARR1_ */
                {
                  int flg;

                  flg = getU1From(&sseq[curOffset]);
                  curOffset++;

                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_PORTAMENTO, !flg ? 0 : 127);

                  sprintf(eventName, "Portamento");
                  sprintf(eventDesc, "%s (%d)", flg ? "On" : "Off", flg);
                  break;
                }

                case 0xcf: /* Bomberman: SEQ_AREA04 */
                {
                  int time;

                  time = getU1From(&sseq[curOffset]);
                  curOffset++;

                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_PORTAMENTOTIME, time);

                  sprintf(eventName, "Portamento Time");
                  sprintf(eventDesc, "%d", time);
                  break;
                }

                case 0xd0: /* Dawn of Sorrow: SDL_BGM_WIND_ */
                {
                  int amount;

                  amount = getU1From(&sseq[curOffset]);
                  curOffset++;
#if 0
                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_ATTACKTIME, 64 + amount / 2);
#endif
                  sprintf(eventName, "Attack Rate");
                  sprintf(eventDesc, "%d", amount);
                  break;
                }

                case 0xd1: /* Dawn of Sorrow: SDL_BGM_WIND_ */
                {
                  int amount;

                  amount = getU1From(&sseq[curOffset]);
                  curOffset++;
#if 0
                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_DECAYTIME, 64 + amount / 2);
#endif
                  sprintf(eventName, "Decay Rate");
                  sprintf(eventDesc, "%d", amount);
                  break;
                }

                case 0xd2: /* Dawn of Sorrow: SDL_BGM_WIND_ */
                {
                  int amount;

                  amount = getU1From(&sseq[curOffset]);
                  curOffset++;

                  sprintf(eventName, "Sustain Rate");
                  sprintf(eventDesc, "%d", amount);
                  break;
                }

                case 0xd3: /* Dawn of Sorrow: SDL_BGM_WIND_ */
                {
                  int amount;

                  amount = getU1From(&sseq[curOffset]);
                  curOffset++;
#if 0
                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_RELEASETIME, 64 + amount / 2);
#endif
                  sprintf(eventName, "Release Rate");
                  sprintf(eventDesc, "%d", amount);
                  break;
                }

                case 0xd4: /* Dawn of Sorrow: SDL_BGM_WIND_ */
                {

                  loopStartCount = getU1From(&sseq[curOffset]);
                  curOffset++;

                  loopStartOffset = curOffset;
                  if(loopStartCount == 0)
                  {
                      loopStartCount = -1;
                      if(!loopStartPointUsed)
                      {
                          switch(g_loopStyle)
                          {
                          case 1:
                              smfInsertControl(smf, absTime, midiCh, midiCh, 0x74, 0);
                              break;
                          case 2:
                              smfInsertMetaEvent(smf, absTime, midiCh, 6, "loopStart", 9);
                              break;
                          }
                          loopStartPointUsed = true;
                      }
                  }

                  sprintf(eventName, "Loop Start");
                  sprintf(eventDesc, "%d", loopStartCount);
                  break;
                }

                case 0xd5:
                {
                  int expression;

                  expression = getU1From(&sseq[curOffset]);
                  curOffset++;

                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_EXPRESSION, expression);

                  sprintf(eventName, "Expression");
                  sprintf(eventDesc, "%d", expression);
                  break;
                }

                case 0xd6:
                {
                  int varNumber;

                  varNumber = getU1From(&sseq[curOffset]);
                  curOffset++;

                  /* TODO: implement */

                  sprintf(eventName, "Print Variable");
                  sprintf(eventDesc, "%d", varNumber);
                  break;
                }

                case 0xe0: /* Children of Mana: SEQ_BGM001 */
                {
                  int amount;

                  amount = getU2LitFrom(&sseq[curOffset]);
                  curOffset += 2;

                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_VIBRATODELAY, 64 + amount / 2);

                  sprintf(eventName, "Modulation Delay");
                  sprintf(eventDesc, "%d", amount);
                  break;
                }

                case 0xe1:
                {
                  int bpm;

                  bpm = getU2LitFrom(&sseq[curOffset]);
                  curOffset += 2;

                  smfInsertTempoBPM(smf, absTime, midiCh, bpm);

                  sprintf(eventName, "Tempo");
                  sprintf(eventDesc, "%d", bpm);
                  break;
                }

                case 0xe3: /* Hippatte! Puzzle Bobble: SEQ_1pbgm03 */
                {
                  int amount;

                  amount = getS2LitFrom(&sseq[curOffset]);
                  curOffset += 2;

                  smfInsertControl(smf, absTime, midiCh, midiCh, SMF_CONTROL_VIBRATODELAY, amount);

                  sprintf(eventName, "Sweep Pitch");
                  sprintf(eventDesc, "%d", amount);
                  break;
                }

                case 0xfc: /* Dawn of Sorrow: SDL_BGM_WIND_ */
                {
                  if(loopStartCount > 0)
                  {
                      loopStartCount--;
                      curOffset = loopStartOffset;
                  }
                  if(loopStartCount == -1)
                  {
                      switch(g_loopStyle)
                      {
                      case 0:
                          loopCount--;
                          curOffset = loopStartOffset;
                          break;
                      case 1:
                          if(!loopEndPointUsed)
                          {
                              smfInsertControl(smf, absTime, midiCh, midiCh, 0x75, 0);
                              loopCount = 0;
                              loopEndPointUsed = true;
                          }
                          break;
                      case 2:
                          if(!loopEndPointUsed)
                          {
                              smfInsertMetaEvent(smf, absTime, midiCh, 6, "loopEnd", 7);
                              loopCount = 0;
                              loopEndPointUsed = true;
                          }
                          break;
                      }
                  }

                  sprintf(eventName, "Loop End");
                  sprintf(eventDesc, "");
                  break;
                }

                case 0xfd:
                {
                  offsetToJump = sseq2mid->track[trackIndex].offsetToReturn;
                  sseq2mid->track[trackIndex].offsetToReturn = SSEQ_INVALID_OFFSET; /* to avoid eternal loop */

                  if(offsetToJump == SSEQ_INVALID_OFFSET)
                  {
                    loopCount = 0;
                    eventException = true;
                    result = false;
                  }

                  sprintf(eventName, "Return");
                  sprintf(eventDesc, "%08X", offsetToJump);
                  break;
                }

                case 0xfe:
                {
                  int flag;
                  unsigned int bit;

                  flag = getU2LitFrom(&sseq[curOffset]);
                  curOffset += 2;

                  if(sseq2mid->modifyChOrder)
                  {
                    int sseqCh;
                    int midiCh = 0;

                    /* padding tracks, if necessary */
                    bit = 1;
                    for(sseqCh = 0; sseqCh < SSEQ_MAX_TRACK; sseqCh++)
                    {
                      if(flag & bit)
                      {
                        sseq2mid->chOrder[sseqCh] = midiChOrder[midiCh];
                        midiCh++;
                      }
                      bit = bit << 1;
                    }
                    bit = 1;
                    for(sseqCh = 0; sseqCh < SSEQ_MAX_TRACK; sseqCh++)
                    {
                      if(!(flag & bit))
                      {
                        sseq2mid->chOrder[sseqCh] = midiChOrder[midiCh];
                        midiCh++;
                      }
                      bit = bit << 1;
                    }
                  }

                  sprintf(eventName, "Signify Multi Track");
                  strcpy(eventDesc, "");
                  for(bit = 1; bit < 0x10000; bit = bit << 1)
                  {
                    strcat(eventDesc, (flag & bit) ? "*" : "-");
                  }
                  break;
                }

                case 0xff:
                {
                  loopCount = 0;
                  sprintf(eventName, "End of Track");
                  sprintf(eventDesc, "");
                  break;
                }

#if 0
                case 0xfa: /* WarioWare Touched! */
#endif

                default:
                  loopCount = 0;
                  eventException = true;
                  result = false;
                  break;
                }
              }
            }
            else
            {
              loopCount = 0;
            }

            if(eventException)
            {
              fprintf(stderr, "warning: exception [%s - %s]\n", eventName, eventDesc);
              strcat(eventDesc, " (!)");
            }

            sseq2midPutLogLine(sseq2mid, eventOffset, curOffset - eventOffset, eventName, eventDesc);
            if(offsetToJump != SSEQ_INVALID_OFFSET)
            {
              curOffset = offsetToJump;
            }
            sseq2mid->track[trackIndex].absTime = absTime;
            sseq2mid->track[trackIndex].curOffset = curOffset;
            sseq2mid->track[trackIndex].loopCount = loopCount;
          } while(loopCount > 0);

          if(sseq2mid->noReverb)
          {
            smfInsertControl(smf, 0, midiCh, midiCh, SMF_CONTROL_REVERB, 0);
          }
          smfSetEndTimingOfTrack(smf, midiCh, sseq2mid->track[midiCh].absTime);
          sseq2midPutLog(sseq2mid, "\n");
        }
      }
    }
  }
  else
  {
    sseq2midPutLog(sseq2mid, "is not valid SSEQ\n");
  }
  return result;
}

/* output standard midi to memory from sseq2mid object */
size_t sseq2midWriteMidi(Sseq2mid* sseq2mid, byte* buffer, size_t bufferSize)
{
  return smfWrite(sseq2mid->smf, buffer, bufferSize);
}

/* output standard midi file from sseq2mid object */
size_t sseq2midWriteMidiFile(Sseq2mid* sseq2mid, const char* filename)
{
  return smfWriteFile(sseq2mid->smf, filename);
}

/* set log message procedure */
void sseq2midSetLogProc(Sseq2mid* sseq2mid, Sseq2midLogProc* logProc)
{
  if(sseq2mid && logProc)
  {
    sseq2mid->logProc = logProc;
  }
}

/* set reverb mode */
bool sseq2midNoReverb(Sseq2mid* sseq2mid, bool noReverb)
{
  bool oldNoReverb = false;

  if(sseq2mid)
  {
    oldNoReverb = sseq2mid->noReverb;
    sseq2mid->noReverb = noReverb;
  }
  return oldNoReverb;
}

/* set sequence loop count */
int sseq2midSetLoopCount(Sseq2mid* sseq2mid, int loopCount)
{
  int oldLoopCount = 1;

  if(sseq2mid)
  {
    oldLoopCount = sseq2mid->loopCount;
    if(loopCount >= 0)
    {
      sseq2mid->loopCount = (loopCount != 0) ? loopCount : 1;
    }
  }
  return oldLoopCount;
}


/* get signed byte */
int getS1From(byte* data)
{
  int val = data[0];
  return (val & 0x80) ? -(signed) (0xFF-val+1) : val;
}

/* get signed 2 bytes as little endian */
int getS2LitFrom(byte* data)
{
  int val = data[0] | (data[1] << 8);
  return (val & 0x8000) ? -(signed) (0xFFFF-val+1) : val;
}

/* get signed 3 bytes as little endian */
int getS3LitFrom(byte* data)
{
  int val = data[0] | (data[1] << 8) | (data[2] << 16);
  return (val & 0x800000) ? -(signed) (0xFFFFFF-val+1) : val;
}

/* get signed 4 bytes as little endian */
int getS4LitFrom(byte* data)
{
  int val = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
  return (val & 0x80000000) ? -(signed) (0xFFFFFFFF-val+1) : val;
}

/* get unsigned byte */
unsigned int getU1From(byte* data)
{
  return (unsigned int) data[0];
}

/* get unsigned 2 bytes as little endian */
unsigned int getU2LitFrom(byte* data)
{
  return (unsigned int) (data[0] | (data[1] << 8));
}

/* get unsigned 3 bytes as little endian */
unsigned int getU3LitFrom(byte* data)
{
  return (unsigned int) (data[0] | (data[1] << 8) | (data[2] << 16));
}

/* get unsigned 4 bytes as little endian */
unsigned int getU4LitFrom(byte* data)
{
  return (unsigned int) (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}
