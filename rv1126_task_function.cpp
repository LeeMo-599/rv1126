#include "rv1126_task_function.h"

int rv1126_system_init()
{
    RK_MPI_SYS_Init();
    return 0;
}

int rv1126_vi_init(S_VI_CONFIG *vi_config)
{
#if 1
    printf("[VI] vi init id: %u, device: %s, width: %u, height: %u, pixfmt: %u, buf count: %u, work mode: %u, enable: %u\n",
           vi_config->id,
           vi_config->attr.pcVideoNode,
           vi_config->attr.u32Width,
           vi_config->attr.u32Height,
           vi_config->attr.enPixFmt,
           vi_config->attr.u32BufCnt,
           vi_config->attr.enWorkMode,
           vi_config->enable);
#endif

    int ret;
    VI_CHN_ATTR_S attr = vi_config->attr;
    unsigned int id = vi_config->id;
    attr.pcVideoNode = CAM_DEVICE_NAME;
    ret = RK_MPI_VI_SetChnAttr(S32CAMID, id, &attr);
    ret |= RK_MPI_VI_EnableChn(S32CAMID, id);
    if (ret != 0)
    {
        printf("[VI] create vi type: %u, error ret: %d", ret);
        return -1;
    }
    return 0;
}

int rv1126_ai_init(S_AI_CONFIG *ai_config)
{

#if 1
    printf("[AI] ai init id: %u,  device: %s, sample format: %u, channels: %u, sample rate: %u, nb samples: %u, enable: %u\n",
           ai_config->id,
           ai_config->attr.pcAudioNode,
           ai_config->attr.enSampleFormat,
           ai_config->attr.u32Channels,
           ai_config->attr.u32SampleRate,
           ai_config->attr.u32NbSamples,
           ai_config->enable);
#endif

    int ret;
    AI_CHN_ATTR_S attr = ai_config->attr;
    unsigned int id = ai_config->id;
    attr.pcAudioNode = AUDIO_ALSA_PATH;
    ret = RK_MPI_AI_SetChnAttr(id, &attr);
    ret = RK_MPI_AI_EnableChn(id);
    if (ret != 0)
    {
        printf("[AI] create ai type: %u, ai id: %u, error ret: %d", id, ret);
        return -1;
    }

    return 0;
}

int rv1126_venc_init(S_VENC_CONFIG *venc_config)
{
#if 1
    printf("[VENC] venc init id: %u, encodec type: %u, image type: %u, vir width: %u, vir height: %u, profile: %u, by frame: %u, pic width: %u, pic height: %u, rotation: %u, enable: %u\n",
           venc_config->id,
           venc_config->attr.stVencAttr.enType,
           venc_config->attr.stVencAttr.imageType,
           venc_config->attr.stVencAttr.u32VirWidth,
           venc_config->attr.stVencAttr.u32VirHeight,
           venc_config->attr.stVencAttr.u32Profile,
           venc_config->attr.stVencAttr.bByFrame,
           venc_config->attr.stVencAttr.u32PicWidth,
           venc_config->attr.stVencAttr.u32PicHeight,
           venc_config->attr.stVencAttr.enRotation,
           venc_config->enable);
#endif

    int ret;
    VENC_CHN_ATTR_S venc_chn_attr = venc_config->attr;
    unsigned int venc_id = venc_config->id;
    ret = RK_MPI_VENC_CreateChn(venc_config->id, &venc_chn_attr);
    if (ret != 0)
    {
        printf("[VENC] create venc id: %u, error ret: %d\n", venc_id, ret);
        return -1;
    }
    else
    {
        printf("[VENC] create venc id: %u, success\n");
    }

    return 0;
}

int rv1126_aenc_init(S_AENC_CONFIG *aenc_config)
{
#if 1
    printf("[AENC] aenc init id: %u, encodec type: %u, bit rate: %u, Quality: %u, enable: %u",
           aenc_config->id,
           aenc_config->attr.enCodecType,
           aenc_config->attr.u32Bitrate,
           aenc_config->attr.u32Quality,
           aenc_config->enable);
    if (aenc_config->attr.enCodecType == RK_CODEC_TYPE_AAC)
    {
        printf("[AENC] aenc init aac channels: %u, sample rate: %u\n",
               aenc_config->attr.stAencAAC.u32Channels,
               aenc_config->attr.stAencAAC.u32SampleRate);
    }

#endif

    int ret;
    AENC_CHN_ATTR_S aenc_chn_attr = aenc_config->attr;
    unsigned int aenc_id = aenc_config->id;
    ret = RK_MPI_AENC_CreateChn(aenc_id, &aenc_chn_attr);
    if (ret != 0)
    {
        printf("[AENC] create aenc id: %u, error ret: %d\n", aenc_id, ret);
        return -1;
    }

    return 0;
}
