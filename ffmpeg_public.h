#ifndef _FFMPEG_PUBLIC_H 
#define _FFMPEG_PUBLIC_H

#define __STDC_CONSTANT_MACROS
extern "C"
{
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include "libavutil/time.h"
}

#define MAX_URL_NUM (50)
#define MAX_URL_ADDR_LENGTH (2000)

#define WIDTH 1920
#define HEIGHT 1080
#define GOPSIZE 5

#if 0
typedef enum
{
    URL_TYPE_NONE = 0,
    LOCAL = 1,
    RTMP = 2,
    SRT = 3 ,
    
} E_URL_TYPE;
#endif

#define RTMP 0
#define SRT 1
#define LOCAL 2


typedef struct
{
    AVStream *st;
    AVCodecContext *enc;
    int64_t next_pts;
    int samples_count;
    AVPacket *pkt;
} OutputStream;

typedef struct
{
    unsigned int config_id;
    unsigned int config_random_value;
    unsigned int task_id;
    unsigned int task_random_value;
    unsigned int url_id;
    //E_URL_TYPE url_type;
    int url_type;
    char url_addr[MAX_URL_ADDR_LENGTH];
    enum AVCodecID video_codec;
    enum AVCodecID audio_codec;
    OutputStream video_st;
    OutputStream audio_st;
    AVFormatContext *oc;
} S_FFMPEG_GROUP;

#endif