#include "rv1126_vi_ai_manage.h"
#include "rv1126_task_function.h"
#include "rv1126_vi_ai_map.h"

int init_vi_ai_function()
{
    rv1126_system_init();

    S_VI_CONFIG tmp_vi_config;
    memset(&tmp_vi_config, 0, sizeof(tmp_vi_config));
    tmp_vi_config.id = 1;
    tmp_vi_config.attr.u32BufCnt = 3;
    tmp_vi_config.attr.u32Width = 1920;
    tmp_vi_config.attr.u32Height = 1080;
    tmp_vi_config.attr.enPixFmt = IMAGE_TYPE_NV12;
    tmp_vi_config.attr.enBufType = VI_CHN_BUF_TYPE_MMAP;
    tmp_vi_config.attr.enWorkMode = VI_WORK_MODE_NORMAL;
    int ret = rv1126_vi_init(&tmp_vi_config);
    if (ret != 0)
    {
        printf("vi init error\n");
    }
    else
    {
        printf("vi init success\n");
        S_VI_MAP vi_map = {0};
        vi_map.id = 0;
        vi_map.real_id = tmp_vi_config.id;
        set_vi_map(0, &vi_map);
    }

    S_AI_CONFIG tmp_ai_config = {0};
    memset(&tmp_ai_config, 0, sizeof(tmp_ai_config));
    tmp_ai_config.id = 0;
    tmp_ai_config.attr.enSampleFormat = RK_SAMPLE_FMT_S16;
    tmp_ai_config.attr.enAiLayout = AI_LAYOUT_NORMAL;
    tmp_ai_config.attr.u32NbSamples = AAC_NB_SAMPLES;
    tmp_ai_config.attr.u32SampleRate = 48000;
    tmp_ai_config.attr.u32Channels = 2;
    tmp_ai_config.enable = true;
    ret = rv1126_ai_init(&tmp_ai_config);
    if (ret != 0)
    {
        printf("ai init error, ret = %d\n", ret);
    }
    else
    {
        printf("ai init success ret = %d\n", ret);
        S_AI_MAP ai_map = {0};
        ai_map.id = 0;
        ai_map.real_id = tmp_ai_config.id;
        set_ai_map(0, &ai_map);
    }

    S_VENC_CONFIG venc_config = {0};
    memset(&venc_config, 0, sizeof(venc_config));
    venc_config.id = 0;
    venc_config.attr.stVencAttr.enType = RK_CODEC_TYPE_H264;
    venc_config.attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
    venc_config.attr.stRcAttr.stH264Cbr.u32Gop = 30;
    venc_config.attr.stRcAttr.stH264Cbr.u32BitRate = 1920 * 1080;
    venc_config.attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
    venc_config.attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 30;
    venc_config.attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
    venc_config.attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 30;
    venc_config.attr.stVencAttr.imageType = IMAGE_TYPE_NV12;
    venc_config.attr.stVencAttr.u32PicWidth = 1920;
    venc_config.attr.stVencAttr.u32PicHeight = 1080;
    venc_config.attr.stVencAttr.u32VirWidth = 1920;
    venc_config.attr.stVencAttr.u32VirHeight = 1080;
    venc_config.attr.stVencAttr.u32Profile = 77;
    venc_config.enable = true;
    ret = rv1126_venc_init(&venc_config);
    if (ret != 0)
    {
        printf("venc init error, ret = %d\n", ret);
    }
    else
    {
        S_VENC_MAP venc_map;
        venc_map.id = 0;
        venc_map.real_id = venc_config.id;
        set_venc_map(0, &venc_map);
        printf("venc init success ret = %d\n", ret);
    }

    S_AENC_CONFIG tmp_aenc_config;
    tmp_aenc_config.id = 0;
    tmp_aenc_config.attr.enCodecType = RK_CODEC_TYPE_AAC;
    tmp_aenc_config.attr.u32Bitrate = 153600;
    tmp_aenc_config.attr.u32Quality = 1;
    tmp_aenc_config.attr.stAencAAC.u32Channels = 2;
    tmp_aenc_config.attr.stAencAAC.u32SampleRate = 48000;
    ret = rv1126_aenc_init(&tmp_aenc_config);
    if (ret != 0)
    {
        printf("aenc init error, ret = %d\n", ret);
    }
    else
    {
        S_AENC_MAP aenc_map;
        aenc_map.id = 0;
        aenc_map.real_id = tmp_aenc_config.id;
        set_aenc_map(0, &aenc_map);
        printf("aenc init success, ret = %d\n", ret);
    }

    return 0;
}