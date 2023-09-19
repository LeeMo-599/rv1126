#include "rkmedia_module_function.h"
#include "rkmedia_assignment_manage.h"
#include "rkmedia_config_public.h"
#include "rkmedia_module.h"
#include "rkmedia_container.h"

int init_rkmedia_module_function()
{
    rkmedia_function_init();

    RV1126_VI_CONFIG rkmedia_vi_config;
    memset(&rkmedia_vi_config, 0, sizeof(rkmedia_vi_config));
    rkmedia_vi_config.id = 0;
    rkmedia_vi_config.attr.pcVideoNode = CMOS_DEVICE_NAME;//VIDEO视频节点路径,
    rkmedia_vi_config.attr.u32BufCnt = 3;//VI捕获视频缓冲区计数，默认是3。分担缓冲区压力，生产设消费者模型
    rkmedia_vi_config.attr.u32Width = 1920;//视频输入的宽度，一般和CMOS摄像头或者外设的宽度一致
    rkmedia_vi_config.attr.u32Height = 1080;//视频输入的高度，一般和CMOS摄像头或者外设的高度一致
    rkmedia_vi_config.attr.enPixFmt = IMAGE_TYPE_NV12;//视频输入的图像格式，默认是NV12(IMAGE_TYPE_NV12)
    rkmedia_vi_config.attr.enBufType = VI_CHN_BUF_TYPE_MMAP;//VI捕捉视频的类型
    rkmedia_vi_config.attr.enWorkMode = VI_WORK_MODE_NORMAL;//VI的工作模式，默认是NORMAL(VI_WORK_MODE_NORMAL)
    int ret = rkmedia_vi_init(&rkmedia_vi_config); //初始化VI工作
    if (ret != 0)
    {
        printf("vi init error\n");
    }
    else
    {
        printf("vi init success\n");
        RV1126_VI_CONTAINTER vi_container;
        vi_container.id = 0;
        vi_container.vi_id = rkmedia_vi_config.id;
        set_vi_container(0, &vi_container);  //设置VI容器
    }

    RV1126_AI_CONFIG rkmedia_ai_config = {0}; 
    memset(&rkmedia_ai_config, 0, sizeof(rkmedia_ai_config));
    rkmedia_ai_config.id = 0;
    rkmedia_ai_config.attr.pcAudioNode = AUDIO_PATH;  //音频alsa路径
    rkmedia_ai_config.attr.enSampleFormat = RK_SAMPLE_FMT_S16; //音频采样格式
    rkmedia_ai_config.attr.enAiLayout = AI_LAYOUT_NORMAL; //音频布局
    rkmedia_ai_config.attr.u32NbSamples = NB_SAMPLES; //采样个数，AAC默认1024
    rkmedia_ai_config.attr.u32SampleRate = 48000; //音频采样率
    rkmedia_ai_config.attr.u32Channels = 2; //音频通道
    ret = rkmedia_ai_init(&rkmedia_ai_config);
    if (ret != 0)
    {
        printf("ai init error\n");
    }
    else
    {
        printf("ai init success\n");
        RV1126_AI_CONTAINTER ai_container;
        ai_container.id = 0;
        ai_container.ai_id = rkmedia_ai_config.id;
        set_ai_container(0, &ai_container);  //设置AI容器
    }

    RV1126_VENC_CONFIG rkmedia_venc_config = {0};
    memset(&rkmedia_venc_config, 0, sizeof(rkmedia_venc_config));
    rkmedia_venc_config.id = 0;
    rkmedia_venc_config.attr.stVencAttr.enType = RK_CODEC_TYPE_H264;  //编码器协议类型
    rkmedia_venc_config.attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR; //编码器码率控制模式
    rkmedia_venc_config.attr.stRcAttr.stH264Cbr.u32Gop = 30;            //GOPSIZE:关键帧间隔
    rkmedia_venc_config.attr.stRcAttr.stH264Cbr.u32BitRate = 1920 * 1080 * 3;//码率
    rkmedia_venc_config.attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;  //目的帧率分子:填的是1固定
    rkmedia_venc_config.attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 25;//目的帧率分母:填的是25固定
    rkmedia_venc_config.attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1; //源头帧率分子:填的是1固定
    rkmedia_venc_config.attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 25; //源头帧率分母:填的是25固定
    rkmedia_venc_config.attr.stVencAttr.imageType = IMAGE_TYPE_NV12; //输入图像类型
    rkmedia_venc_config.attr.stVencAttr.u32PicWidth = 1920; //编码图像宽度
    rkmedia_venc_config.attr.stVencAttr.u32PicHeight = 1080;//编码图像高度
    rkmedia_venc_config.attr.stVencAttr.u32VirWidth = 1920;//编码图像虚宽度，一般来说u32VirWidth和u32PicWidth是一致的
    rkmedia_venc_config.attr.stVencAttr.u32VirHeight = 1080;//编码图像虚高度，一般来说u32VirHeight和u32PicHeight是一致的
    rkmedia_venc_config.attr.stVencAttr.u32Profile = 66;//编码等级H.264: 66: Baseline; 77:Main Profile; 100:High Profile; H.265: default:Main; Jpege/MJpege: default:Baseline(编码等级的作用主要是改变画面质量，66的画面质量最差利于网络传输，100的质量最好)
    ret = rkmedia_venc_init(&rkmedia_venc_config); //VENC模块的初始化
    if(ret != 0)
    {
        printf("venc init error\n");
    }
    else
    {
        RV1126_VENC_CONTAINER venc_container;
        venc_container.id = 0;
        venc_container.venc_id = rkmedia_venc_config.id;
        set_venc_container(0, &venc_container);
        printf("venc init success\n");
    }

    RV1126_AENC_CONFIG rkmedia_aenc_config; 
    rkmedia_aenc_config.id = 0;
    rkmedia_aenc_config.attr.enCodecType = RK_CODEC_TYPE_AAC; //AENC的编码类型
    rkmedia_aenc_config.attr.u32Bitrate = 153600; //AENC的音频码率
    rkmedia_aenc_config.attr.u32Quality = 1; //AENC编码质量
    rkmedia_aenc_config.attr.stAencAAC.u32Channels = 2; //AENC编码通道
    rkmedia_aenc_config.attr.stAencAAC.u32SampleRate = 48000; //AENC编码采样率
    ret = rkmedia_aenc_init(&rkmedia_aenc_config);
    if (ret != 0)
    {
        printf("aenc init error\n");
    }
    else
    {
        RV1126_AENC_CONTAINER aenc_container;
        aenc_container.id = 0;
        aenc_container.aenc_id = rkmedia_aenc_config.id;
        set_aenc_container(0, &aenc_container);  //设置AENC容器
        printf("aenc init success\n");
    }

    return 0;
}
