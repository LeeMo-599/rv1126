#include "ffmpeg_module.h"

int add_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, unsigned int task_id)
{
    AVCodecContext *c = NULL;

    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec))
    {
        printf("Can't not find any encoder");
    }
    else
    {
        printf("Success find encoder");
    }

    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st)
    {
        printf("[FFMPEG][ADD STREAM] task id: %u, Could not allocate stream\n", task_id);
        return -2;
    }

    ost->st->id = oc->nb_streams - 1;
    c = avcodec_alloc_context3(*codec);
    if (!c)
    {
        printf("[FFMPEG][ADD STREAM] task id: %u, Could not alloc an encoding context\n", task_id);
        return -3;
    }

    ost->enc = c;

    /*if ((*codec)->type == AVMEDIA_TYPE_VIDEO)
    {
    }

    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
    {
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }*/

    switch ((*codec)->type)
    {
    case AVMEDIA_TYPE_AUDIO:

        c->sample_fmt = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        c->bit_rate = 153600;
        c->sample_rate = 48000;
        c->channel_layout = AV_CH_LAYOUT_STEREO;
        c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
        ost->st->time_base = (AVRational){1, c->sample_rate};
        break;

    case AVMEDIA_TYPE_VIDEO:

        c->codec_id = codec_id;
        c->bit_rate = 40000; //SRT RTMP
        //分辨率必须是2的倍数
        c->width = WIDTH;
        c->height = HEIGHT;

        ost->st->r_frame_rate.den = 1;
        ost->st->r_frame_rate.num = 30;
        ost->st->time_base = (AVRational){1, 30};

        c->time_base = ost->st->time_base;
        c->gop_size = 5;
        c->pix_fmt = AV_PIX_FMT_YUV420P;
        c->max_b_frames = 0; //不要B帧

        c->me_range = 12; //srt:12 rtmp:15
        c->max_qdiff = 10;
        c->qmin = 10;
        c->qmax = 51;
        c->qcompress = 0.5;

        break;

    default:
        break;
    }

    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
    {
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    return 0;
}

int open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    AVCodecContext *c = ost->enc;

    avcodec_open2(c, codec, NULL);
    ost->pkt = av_packet_alloc();

    /* 将流参数复制到多路复用器 */
    avcodec_parameters_from_context(ost->st->codecpar, c);
    return 0;
}

int open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    AVCodecContext *c = NULL;
    //int nb_samples = 1024;

    c = ost->enc;
    avcodec_open2(c, codec, NULL);
    ost->pkt = av_packet_alloc();

    avcodec_parameters_from_context(ost->st->codecpar, c);

    
    return 0;
}

void close_stream(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_close(ost->enc);
    avcodec_free_context(&ost->enc);

    av_buffer_unref(&(ost->pkt->buf));

    av_packet_unref(ost->pkt);
    av_packet_free(&ost->pkt);
}

int init_stream_format_context(S_FFMPEG_GROUP *group)
{
    AVOutputFormat *fmt = NULL;
    AVCodec *audio_codec = NULL;
    AVCodec *video_codec = NULL;
    int ret = 0;

    //av_register_all();
    //avformat_network_init();

    if (group->url_type == RTMP)
    {
        ret = avformat_alloc_output_context2(&group->oc, NULL, "flv", group->url_addr); //RTMP
        if (ret < 0)
        {
            printf("[FFMPEG][INIT] task id: %u, url id: %u, RTMP protocol type: %d alloc output context2 error", group->task_id, group->url_id, group->url_type);
            return -1;
        }
    }
    else if (group->url_type == SRT)
    {
        ret = avformat_alloc_output_context2(&group->oc, NULL, "mpegts", group->url_addr); //SRT
        if (ret < 0)
        {
            printf("[FFMPEG][INIT] task id: %u, url id: %u, SRT protocol type: %d alloc output context2 error", group->task_id, group->url_id, group->url_type);
            return -1;
        }
    }
    else if (group->url_type == LOCAL)
    {
        ret = avformat_alloc_output_context2(&group->oc, NULL, NULL, group->url_addr); //LOCAL
        if (ret < 0)
        {
            printf("[FFMPEG][INIT] task id: %u, url id: %u, LOCAL protocol type: %d alloc output context2 error", group->task_id, group->url_id, group->url_type);
            return -1;
        }
    }

    fmt = group->oc->oformat;
    /*指定编码器*/
    fmt->video_codec = group->video_codec;
    fmt->audio_codec = group->audio_codec;

    if (fmt->video_codec != AV_CODEC_ID_NONE)
    {
        ret = add_stream(&group->video_st, group->oc, &video_codec, fmt->video_codec, group->task_id);
        if (ret < 0)
        {
            avcodec_free_context(&group->video_st.enc);
            close_stream(group->oc, &group->video_st);
            avformat_free_context(group->oc);
            //zlog_error(get_category(), "[FFMPEG][INIT] task id: %u, url id: %u, add video stream error", group->task_id, group->url_id);
            return -1;
        }

        ret = open_video(group->oc, video_codec, &group->video_st, NULL);
        if (ret < 0)
        {
            avformat_free_context(group->oc);
        }
    }

    if (fmt->audio_codec != AV_CODEC_ID_NONE)
    {
        ret = add_stream(&group->audio_st, group->oc, &audio_codec, fmt->audio_codec, group->task_id);
        if (ret < 0)
        {
            avcodec_free_context(&group->audio_st.enc);
            close_stream(group->oc, &group->audio_st);
            avformat_free_context(group->oc);
            printf("[FFMPEG][INIT] task id: %u, url id: %u, add audio stream error\n", group->task_id, group->url_id);
            return -1;
        }

        ret = open_audio(group->oc, audio_codec, &group->audio_st, NULL);
        if (ret < 0)
        {
            avformat_free_context(group->oc);
        }
    }

    av_dump_format(group->oc, 0, group->url_addr, 1);

    if (!(fmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&group->oc->pb, group->url_addr, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            //printf("[FFMPEG][INIT] task id: %u, url id: %u, open output file: '%s': %s error |__|", group->task_id, group->url_id, group->url_addr, av_err2str(ret));
            close_stream(group->oc, &group->video_st);
            close_stream(group->oc, &group->audio_st);
            avformat_free_context(group->oc);
            return -1;
        }
    }

    avformat_write_header(group->oc, NULL);
    return 0;
}


