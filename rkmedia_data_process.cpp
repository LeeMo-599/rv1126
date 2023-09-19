#include "rkmedia_data_process.h"
#include "ffmpeg_video_queue.h"
#include "ffmpeg_audio_queue.h"
#include "rkmedia_module.h"
#include "rkmedia_ffmpeg_config.h"
#include "rkmedia_data_process.h"

extern VIDEO_QUEUE *video_queue;
extern AUDIO_QUEUE *audio_queue;

// 从RV1126视频编码数据赋值到FFMPEG的Video AVPacket中
AVPacket *get_ffmpeg_video_avpacket(AVPacket *pkt)
{
    video_data_packet_t *video_data_packet = video_queue->getVideoPacketQueue(); // 从视频队列获取数据

    if (video_data_packet != NULL)
    {
        /*
     重新分配给定的缓冲区
   1.  如果入参的 AVBufferRef 为空，直接调用 av_realloc 分配一个新的缓存区，并调用 av_buffer_create 返回一个新的 AVBufferRef 结构；
   2.  如果入参的缓存区长度和入参 size 相等，直接返回 0；
   3.  如果对应的 AVBuffer 设置了 BUFFER_FLAG_REALLOCATABLE 标志，或者不可写，再或者 AVBufferRef data 字段指向的数据地址和 AVBuffer 的 data 地址不同，递归调用 av_buffer_realloc 分配一个新
的 buffer，并将 data 拷贝过去；
   4.  不满足上面的条件，直接调用 av_realloc 重新分配缓存区。
 */
        int ret = av_buffer_realloc(&pkt->buf, video_data_packet->video_frame_size + 70);
        if (ret < 0)
        {
            return NULL;
        }
        pkt->size = video_data_packet->video_frame_size;                                        // rv1126的视频长度赋值到AVPacket Size
        memcpy(pkt->buf->data, video_data_packet->buffer, video_data_packet->video_frame_size); // rv1126的视频数据赋值到AVPacket data
        pkt->data = pkt->buf->data;                                                             // 把pkt->buf->data赋值到pkt->data
        pkt->flags |= AV_PKT_FLAG_KEY;                                                          // 默认flags是AV_PKT_FLAG_KEY
        if (video_data_packet != NULL)
        {
            free(video_data_packet);
            video_data_packet = NULL;
        }

        return pkt;
    }
    else
    {
        return NULL;
    }
}

AVPacket *get_ffmpeg_audio_avpacket(AVPacket *pkt)
{
    audio_data_packet_t *audio_data_packet = audio_queue->getAudioPacketQueue();// 从音频队列获取数据

    if (audio_data_packet != NULL)
    {
        /*
  重新分配给定的缓冲区
1.  如果入参的 AVBufferRef 为空，直接调用 av_realloc 分配一个新的缓存区，并调用 av_buffer_create 返回一个新的 AVBufferRef 结构；
2.  如果入参的缓存区长度和入参 size 相等，直接返回 0；
3.  如果对应的 AVBuffer 设置了 BUFFER_FLAG_REALLOCATABLE 标志，或者不可写，再或者 AVBufferRef data 字段指向的数据地址和 AVBuffer 的 data 地址不同，递归调用 av_buffer_realloc 分配一个新
的 buffer，并将 data 拷贝过去；
4.  不满足上面的条件，直接调用 av_realloc 重新分配缓存区。
*/
        int ret = av_buffer_realloc(&pkt->buf, audio_data_packet->audio_frame_size + 70);
        if (ret < 0)
        {
            return NULL;
        }

        pkt->size = audio_data_packet->audio_frame_size; // rv1126的音频长度赋值到AVPacket Size
        memcpy(pkt->buf->data, audio_data_packet->buffer, audio_data_packet->audio_frame_size); //rv1126的音频数据赋值到AVPacket data
        pkt->data = pkt->buf->data; // 把pkt->buf->data赋值到pkt->data

        if (audio_data_packet != NULL)
        {
            free(audio_data_packet);
            audio_data_packet = NULL;
        }

        return pkt;
    }
    else
    {
        return NULL;
    }
}

int write_ffmpeg_avpacket(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    /*将输出数据包时间戳值从编解码器重新调整为流时基 */
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;

    return av_interleaved_write_frame(fmt_ctx, pkt);
}

int deal_video_avpacket(AVFormatContext *oc, OutputStream *ost)
{
    int ret;
    AVCodecContext *c = ost->enc;
    AVPacket *video_packet = get_ffmpeg_video_avpacket(ost->packet); // 从RV1126视频编码数据赋值到FFMPEG的Video AVPacket中
    if (video_packet != NULL)
    {
        video_packet->pts = ost->next_timestamp++; // VIDEO_PTS按照帧率进行累加
    }

    ret = write_ffmpeg_avpacket(oc, &c->time_base, ost->stream, video_packet); // 向复合流写入视频数据
    if (ret != 0)
    {
        printf("write video avpacket error");
        return -1;
    }

    return 0;
}

int deal_audio_avpacket(AVFormatContext *oc, OutputStream *ost)
{
    int ret;
    AVCodecContext *c = ost->enc;
    AVPacket *audio_packet = get_ffmpeg_audio_avpacket(ost->packet); // 从RV1126视频编码数据赋值到FFMPEG的Audio AVPacket中
    if (audio_packet != NULL)
    {
        audio_packet->pts = ost->samples_count;
        ost->samples_count += 1024;
        ost->next_timestamp = ost->samples_count; // AUDIO_PTS按照帧率进行累加1024
    }

    ret = write_ffmpeg_avpacket(oc, &c->time_base, ost->stream, audio_packet); // 向复合流写入音频数据
    if (ret != 0)
    {
        printf(" write audio avpacket error");
        return -1;
    }

    return 0;
}

void *camera_venc_thread(void *args)
{
    pthread_detach(pthread_self());
    MEDIA_BUFFER mb = NULL;

    VENC_PROC_PARAM venc_arg = *(VENC_PROC_PARAM *)args;
    free(args);

    printf("video_venc_thread...\n");

    while (1)
    {
        // 从指定通道中获取VENC数据
        mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_VENC, venc_arg.vencId, -1);
        if (!mb)
        {
            printf("get venc media buffer error\n");
            break;
        }

        // int naluType = RK_MPI_MB_GetFlag(mb);
        // 分配video_data_packet_t结构体
        video_data_packet_t *video_data_packet = (video_data_packet_t *)malloc(sizeof(video_data_packet_t));
        // 把VENC视频缓冲区数据传输到video_data_packet的buffer中
        memcpy(video_data_packet->buffer, RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetSize(mb));
        // 把VENC的长度赋值给video_data_packet的video_frame_size中
        video_data_packet->video_frame_size = RK_MPI_MB_GetSize(mb);
        // video_data_packet->frame_flag = naluType;
        // 入到视频压缩队列
        video_queue->putVideoPacketQueue(video_data_packet);
        // printf("#naluType = %d \n", naluType);
        // 释放VENC资源
        RK_MPI_MB_ReleaseBuffer(mb);
    }

    MPP_CHN_S vi_channel;
    MPP_CHN_S venc_channel;

    vi_channel.enModId = RK_ID_VI;
    vi_channel.s32ChnId = 0;

    venc_channel.enModId = RK_ID_VENC;
    venc_channel.s32ChnId = venc_arg.vencId;

    int ret;
    ret = RK_MPI_SYS_UnBind(&vi_channel, &venc_channel);
    if (ret != 0)
    {
        printf("VI UnBind failed \n");
    }
    else
    {
        printf("Vi UnBind success\n");
    }

    ret = RK_MPI_VENC_DestroyChn(0);
    if (ret)
    {
        printf("Destroy Venc error! ret=%d\n", ret);
        return 0;
    }
    // destroy vi
    ret = RK_MPI_VI_DisableChn(0, 0);
    if (ret)
    {
        printf("DisableChn Venc error! ret=%d\n", ret);
        return 0;
    }

    return NULL;
}

void *audio_aenc_thread(void *args)
{
    pthread_detach(pthread_self());
    MEDIA_BUFFER mb = NULL;

    AENC_PROC_PARAM aenc_arg = *(AENC_PROC_PARAM *)args;
    free(args);

    while (1)
    {
        // 从指定通道中获取AENC数据
        mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_AENC, aenc_arg.aencId, -1);
        if (!mb)
        {
            printf("get aenc buffer error\n");
            break;
        }

        // 分配audio_data_packet_t结构体
        audio_data_packet_t *audio_data_packet = (audio_data_packet_t *)malloc(sizeof(audio_data_packet_t));
        // 把AENC视频缓冲区数据传输到audio_data_packet的buffer中
        memcpy(audio_data_packet->buffer, RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetSize(mb));
        // 把AENC的长度赋值给audio_data_packet的audio_frame_size中
        audio_data_packet->audio_frame_size = RK_MPI_MB_GetSize(mb);
        // 入到音频压缩队列
        audio_queue->putAudioPacketQueue(audio_data_packet);
        // 释放AENC资源
        RK_MPI_MB_ReleaseBuffer(mb);
    }

    MPP_CHN_S ai_channel;
    MPP_CHN_S aenc_channel;

    ai_channel.enModId = RK_ID_AI;
    ai_channel.s32ChnId = 0;

    aenc_channel.enModId = RK_ID_AENC;
    aenc_channel.s32ChnId = aenc_arg.aencId;

    int ret;
    ret = RK_MPI_SYS_UnBind(&ai_channel, &aenc_channel);
    if (ret != 0)
    {
        printf("AI Bind UnBind error\n");
    }
    else
    {
        printf("Ai UnBind success\n");
    }

    ret = RK_MPI_AENC_DestroyChn(0);
    if (ret)
    {
        printf("Destroy AENC CHN\n");
        return 0;
    }
    // destroy vi
    ret = RK_MPI_AI_DisableChn(0);
    if (ret)
    {
        printf("Disable AENC CHN\n");
        return 0;
    }

    return NULL;
}

// 音视频合成推流线程
void *push_server_thread(void *args)
{
    pthread_detach(pthread_self());
    RKMEDIA_FFMPEG_CONFIG ffmpeg_config = *(RKMEDIA_FFMPEG_CONFIG *)args;
    free(args);
    AVOutputFormat *fmt = NULL;
    int ret;

    while (1)
    {
        /*
     我们以转换到同一时基下的时间戳为例，假设上一时刻音、视频帧的保存时间戳都是0。
     当前任意保存一种视频帧，例如保存视频的时间戳为video_t1。接着比较时间戳，发现音频时间戳为0 < video_t1，保存一帧音频，时间戳为audio_t1。
     继续比较时间戳，发现audio_t1 < video_t1，选择保存一帧音频，时间戳为audio_t2。
     再一次比较时间戳video_t1 < audio_t2，选择保存一帧视频，时间戳为video_t2。
     int av_compare_ts(int64_t ts_a, AVRational_tb_b,int64_t ts_b, AVRational tb_b)
     {
         int64_t a = tb_a.num * (int64_t)tb_b.den;
         int64_t b = tb_b.num * (int64_t)tb_a.den;
         if ((FFABS64U(ts_a)|a|FFABS64U(ts_b)|b) <= INT_MAX)
             return (ts_a*a > ts_b*b) - (ts_a*a < ts_b*b);
         if (av_rescale_rnd(ts_a, a, b, AV_ROUND_DOWN) < ts_b)
             return -1;
          if (av_rescale_rnd(ts_b, b, a, AV_ROUND_DOWN) < ts_a)
             return -1;
         return 0;
     }
     */
        ret = av_compare_ts(ffmpeg_config.video_stream.next_timestamp,
                            ffmpeg_config.video_stream.enc->time_base,
                            ffmpeg_config.audio_stream.next_timestamp,
                            ffmpeg_config.audio_stream.enc->time_base);

        if (ret <= 0)
        {
            ret = deal_video_avpacket(ffmpeg_config.oc, &ffmpeg_config.video_stream); // 处理FFMPEG视频数据
            if (ret == -1)
            {
                printf("deal_video_avpacket error\n");
                break;
            }
        }
        else
        {
            ret = deal_audio_avpacket(ffmpeg_config.oc, &ffmpeg_config.audio_stream); // 处理FFMPEG音频数据
            if (ret == -1)
            {
                printf("deal_video_avpacket error\n");
                break;
            }
        }
    }

    av_write_trailer(ffmpeg_config.oc);                         // 写入AVFormatContext的尾巴
    free_stream(ffmpeg_config.oc, &ffmpeg_config.video_stream); // 释放VIDEO_STREAM的资源
    free_stream(ffmpeg_config.oc, &ffmpeg_config.audio_stream); // 释放AUDIO_STREAM的资源
    avio_closep(&ffmpeg_config.oc->pb);                         // 释放AVIO资源
    avformat_free_context(ffmpeg_config.oc);                    // 释放AVFormatContext资源
    return NULL;
}
