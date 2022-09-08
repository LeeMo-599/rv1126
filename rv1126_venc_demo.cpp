#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "sample_common.h"
#include "rkmedia_api.h"
#include "rkmedia_venc.h"

void *h264_venc_thread(void *args)
{
  pthread_detach(pthread_self());
  FILE *save_file = fopen("output_4k.h264", "w");

  MEDIA_BUFFER mb = NULL;
  while (1)
  {
    printf("Get HDMI Buffer\n");
    mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_VENC, 0, -1);
    if (!mb)
    {
      printf("RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
      break;
    }

    fwrite(RK_MPI_MB_GetPtr(mb), 1, RK_MPI_MB_GetSize(mb), save_file);
    RK_MPI_MB_ReleaseBuffer(mb);
  }
}

int main()
{
  RK_MPI_SYS_Init();
  RK_U32 u32Width = 1920;
  RK_U32 u32Height = 1080;
  RK_CHAR *pDeviceName = "rkispp_scale0";
  RK_CHAR *pDeviceName_02 = "rkispp_scale1";
  RK_CHAR *pOutPath = "test_output.h264";
  RK_CHAR *pIqfilesPath = NULL;
  CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;
  RK_CHAR *pCodecName = "H264";
  RK_S32 s32CamId = 0;
  RK_U32 u32BufCnt = 3;
  int ret;

  printf("#Device: %s\n", pDeviceName);
  printf("#CodecName:%s\n", pCodecName);
  printf("#Resolution: %dx%d\n", u32Width, u32Height);
  //printf("#Frame Count to save: %d\n", g_s32FrameCnt);
  printf("#Output Path: %s\n", pOutPath);
  printf("#CameraIdx: %d\n\n", s32CamId);

#if 1
  rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
  SAMPLE_COMM_ISP_Init(hdr_mode, RK_FALSE);
  SAMPLE_COMM_ISP_Run();
  SAMPLE_COMM_ISP_SetFrameRate(30);
#endif

  VI_CHN_ATTR_S vi_chn_attr;
  vi_chn_attr.pcVideoNode = pDeviceName;
  vi_chn_attr.u32BufCnt = u32BufCnt;
  vi_chn_attr.u32Width = u32Width;
  vi_chn_attr.u32Height = u32Height;
  vi_chn_attr.enPixFmt = IMAGE_TYPE_NV12;
  vi_chn_attr.enBufType = VI_CHN_BUF_TYPE_MMAP;
  vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;
  ret = RK_MPI_VI_SetChnAttr(s32CamId, 1, &vi_chn_attr);
  ret |= RK_MPI_VI_EnableChn(s32CamId, 1);
  if (ret)
  {
    printf("ERROR: create rkisp0 VI[0] error! ret=%d\n", ret);
    return 0;
  }

  VENC_CHN_ATTR_S venc_chn_attr;
  memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));
  venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_H264;
  venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
  venc_chn_attr.stRcAttr.stH264Cbr.u32Gop = 30;
  venc_chn_attr.stRcAttr.stH264Cbr.u32BitRate = u32Width * u32Height;
  // frame rate: in 30/1, out 30/1.
  venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
  venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 30;
  venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
  venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 30;
  venc_chn_attr.stVencAttr.imageType = IMAGE_TYPE_NV12;
  venc_chn_attr.stVencAttr.u32PicWidth = u32Width;
  venc_chn_attr.stVencAttr.u32PicHeight = u32Height;
  venc_chn_attr.stVencAttr.u32VirWidth = u32Width;
  venc_chn_attr.stVencAttr.u32VirHeight = u32Height;
  venc_chn_attr.stVencAttr.u32Profile = 77;
  ret = RK_MPI_VENC_CreateChn(0, &venc_chn_attr);
  if (ret)
  {
    printf("ERROR: create VENC[0] error! ret=%d\n", ret);
    return 0;
  }

  MPP_CHN_S stSrcChn;
  stSrcChn.enModId = RK_ID_VI;
  stSrcChn.s32DevId = 0;
  stSrcChn.s32ChnId = 1;

  MPP_CHN_S stDestChn;
  stDestChn.enModId = RK_ID_VENC;
  stDestChn.s32DevId = 0;
  stDestChn.s32ChnId = 0;

  ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
  if (ret)
  {
    printf("ERROR: Bind VI[0] and VENC[0] error! ret=%d\n", ret);
  }

  pthread_t pid;
  ret = pthread_create(&pid, NULL, h264_venc_thread, NULL);

  while (1)
  {
    usleep(500000);
  }

  ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
  if (ret) {
    printf("ERROR: UnBind VI[0] and VENC[0] error! ret=%d\n", ret);
    return 0;
  }
  // destroy venc before vi
  ret = RK_MPI_VENC_DestroyChn(0);
  if (ret) {
    printf("ERROR: Destroy VENC[0] error! ret=%d\n", ret);
    return 0;
  }
  // destroy vi
  ret = RK_MPI_VI_DisableChn(s32CamId, 0);
  if (ret) {
    printf("ERROR: Destroy VI[0] error! ret=%d\n", ret);
    return 0;
  }


  return 0;
}