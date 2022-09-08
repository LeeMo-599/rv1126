#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "rkmedia_api.h"

#define AAC_PROFILE_LOW 1

typedef struct AacFreqIdx_ {
  RK_S32 u32SmpRate;
  RK_U8 u8FreqIdx;
} AacFreqIdx;

AacFreqIdx AacFreqIdxTbl[13] = {{96000, 0}, {88200, 1}, {64000, 2},  {48000, 3},
                                {44100, 4}, {32000, 5}, {24000, 6},  {22050, 7},
                                {16000, 8}, {12000, 9}, {11025, 10}, {8000, 11},
                                {7350, 12}};

static void GetAdtsHeader(RK_U8 *pu8AdtsHdr, RK_S32 u32SmpRate, RK_U8 u8Channel,
                          RK_U32 u32DataLen) {
  RK_U8 u8FreqIdx = 0;
  for (int i = 0; i < 13; i++) {
    if (u32SmpRate == AacFreqIdxTbl[i].u32SmpRate) {
      u8FreqIdx = AacFreqIdxTbl[i].u8FreqIdx;
      break;
    }
  }

  RK_U32 u32PacketLen = u32DataLen + 7;
  pu8AdtsHdr[0] = 0xFF;
  pu8AdtsHdr[1] = 0xF1;
  pu8AdtsHdr[2] =
      ((AAC_PROFILE_LOW) << 6) + (u8FreqIdx << 2) + (u8Channel >> 2);
  pu8AdtsHdr[3] = (((u8Channel & 3) << 6) + (u32PacketLen >> 11));
  pu8AdtsHdr[4] = ((u32PacketLen & 0x7FF) >> 3);
  pu8AdtsHdr[5] = (((u32PacketLen & 7) << 5) + 0x1F);
  pu8AdtsHdr[6] = 0xFC;
}


void *Get_Audio_Thread(void *args)
{
    pthread_detach(pthread_self());
    FILE *save_file = fopen("output_4k.aac", "w");

    RK_U8 aac_header[7];

    MEDIA_BUFFER mb = NULL;
    while (1)
    {
        printf("Get HDMI Buffer\n");
        mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_AENC, 0, -1);
        if (!mb)
        {
            printf("RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
            break;
        }
        GetAdtsHeader(aac_header, 48000, 2, RK_MPI_MB_GetSize(mb));
        fwrite(aac_header, 1, 7, save_file);
        fwrite(RK_MPI_MB_GetPtr(mb), 1, RK_MPI_MB_GetSize(mb), save_file);
        //fwrite(RK_MPI_MB_GetPtr(mb), 1, RK_MPI_MB_GetSize(mb), save_file);
        //RK_MPI_MB_ReleaseBuffer(mb);
    }
}

int main()
{

    RK_U32 u32SampleRate = 48000;
    RK_U32 u32BitRate = 64000; // 64kbps
    RK_U32 u32ChnCnt = 2;
    RK_U32 u32FrameCnt = 1024; // always 1024 for aac
    SAMPLE_FORMAT_E enSampleFmt = RK_SAMPLE_FMT_FLTP;
    RK_CHAR *p_DeviceName = "default:CARD=rockchiprk809co";
    RK_CHAR *pOutPath = "/tmp/aenc.adts";
    int ret = 0;
    int c;

    printf("#Device: %s\n", p_DeviceName);
    printf("#SampleRate: %d\n", u32SampleRate);
    printf("#Channel Count: %d\n", u32ChnCnt);
    printf("#Frame Count: %d\n", u32FrameCnt);
    printf("#BitRate: %d\n", u32BitRate);
    printf("#SampleFmt: %s\n", (enSampleFmt == RK_SAMPLE_FMT_S16) ? "s16le" : "fltp");
    printf("#Output Path: %s\n", pOutPath);

    RK_MPI_SYS_Init();

    // 1. create AI
    AI_CHN_ATTR_S ai_attr;
    ai_attr.pcAudioNode = p_DeviceName;
    ai_attr.enSampleFormat = enSampleFmt;
    ai_attr.u32NbSamples = u32FrameCnt;
    ai_attr.u32SampleRate = u32SampleRate;
    ai_attr.u32Channels = u32ChnCnt;
    ai_attr.enAiLayout = AI_LAYOUT_NORMAL;
    ret = RK_MPI_AI_SetChnAttr(0, &ai_attr);
    ret |= RK_MPI_AI_EnableChn(0);
    if (ret)
    {
        printf("Create AI[0] failed! ret=%d\n", ret);
        //return -1;
    }

    AENC_CHN_ATTR_S aenc_hdmi_attr;
    aenc_hdmi_attr.enCodecType = RK_CODEC_TYPE_AAC;
    aenc_hdmi_attr.u32Bitrate = u32BitRate;
    aenc_hdmi_attr.u32Quality = 1;
    aenc_hdmi_attr.stAencAAC.u32Channels = u32ChnCnt;
    aenc_hdmi_attr.stAencAAC.u32SampleRate = u32SampleRate;
    ret = RK_MPI_AENC_CreateChn(0, &aenc_hdmi_attr);
    if (ret)
    {
        printf("Create AENC[0] failed! ret=%d\n", ret);
    }

    MPP_CHN_S mpp_chn_ai, mpp_chn_aenc;
    mpp_chn_ai.enModId = RK_ID_AI;
    mpp_chn_ai.s32ChnId = 0;
    mpp_chn_aenc.enModId = RK_ID_AENC;
    mpp_chn_aenc.s32ChnId = 0;
    ret = RK_MPI_SYS_Bind(&mpp_chn_ai, &mpp_chn_aenc);
    if (ret)
    {
        printf("Bind AI[0] to AENC[0] failed! ret=%d\n", ret);
    }

    pthread_t hdmi_audio_thread;
    pthread_create(&hdmi_audio_thread, NULL, Get_Audio_Thread, NULL);

    while (1)
    {
        sleep(50);
    }

    printf("AI Before\n");
    RK_MPI_AI_DisableChn(0);
    printf("AENC Before\n");
    RK_MPI_AENC_DestroyChn(0);

    return 0;
}