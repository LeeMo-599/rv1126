#include "rv1126_data_process.h"
#include "ffmpeg_video_queue.h"
#include "ffmpeg_audio_queue.h"
#include "ffmpeg_module.h"

extern VIDEO_QUEUE *video_queue;
extern AUDIO_QUEUE *audio_queue;

AVPacket *get_video_packet_from_queue(AVStream *st, AVPacket *pkt)
{
    video_data_packet_t *video_data_packet = video_queue->getVideoPacketQueue();

    if (video_data_packet != NULL)
    {
        int ret = av_buffer_realloc(&pkt->buf, video_data_packet->video_frame_size + AV_INPUT_BUFFER_PADDING_SIZE);
        if (ret < 0)
        {
            return NULL;
        }
#if 1
        pkt->size = video_data_packet->video_frame_size;
        printf("video_pkt_size = %d\n", pkt->size);
        memcpy(pkt->buf->data, video_data_packet->buffer, video_data_packet->video_frame_size);
        pkt->data = pkt->buf->data;

        printf("frame_flag = %d\n", video_data_packet->frame_flag);
        pkt->flags |= AV_PKT_FLAG_KEY;

        if ((video_data_packet->frame_flag == VENC_NALU_IDRSLICE) || (video_data_packet->frame_flag == VENC_NALU_ISLICE))
        {
            printf("DETECT IDR Frame\n");
            //pkt->flags |= AV_PKT_FLAG_KEY;
        }
        
#endif
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

AVPacket *get_audio_packet_from_queue(AVPacket *pkt)
{
    audio_data_packet_t *audio_data_packet = audio_queue->getAudioPacketQueue();

    if (audio_data_packet != NULL)
    {
        int ret = av_buffer_realloc(&pkt->buf, audio_data_packet->audio_frame_size + AV_INPUT_BUFFER_PADDING_SIZE);
        if (ret < 0)
        {
            return NULL;
        }

        pkt->size = audio_data_packet->audio_frame_size;
        printf("audio_pkt_size = %d\n", pkt->size);
        memcpy(pkt->buf->data, audio_data_packet->buffer, audio_data_packet->audio_frame_size);
        pkt->data = pkt->buf->data;

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

int write_video_packet(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    /*将输出数据包时间戳值从编解码器重新调整为流时基 */
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;

    return av_interleaved_write_frame(fmt_ctx, pkt);
}

int write_audio_packet(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;

    return av_interleaved_write_frame(fmt_ctx, pkt);
}

int handle_video_packet(AVFormatContext *oc, OutputStream *ost)
{
    int ret;

    AVPacket *pkt = get_video_packet_from_queue(ost->st, ost->pkt);
    if (pkt != NULL)
    {
        pkt->pts = ost->next_pts++;
    }

    ret = write_video_packet(oc, &(ost->enc->time_base), ost->st, pkt);
    if (ret != 0)
    {
        printf("[FFMPEG] write video frame error");
        return -1;
    }
    else
    {
        //zlog_info(get_category(), "[FFMPEG] write video frame ok");
    }

    return 0;
}

int handle_audio_packet(AVFormatContext *oc, OutputStream *ost)
{
    int ret;

    AVCodecContext *c = ost->enc;
    AVPacket *pkt = get_audio_packet_from_queue(ost->pkt);
    if (pkt != NULL)
    {
        pkt->pts = av_rescale_q(ost->samples_count, (AVRational){1, c->sample_rate}, c->time_base);
        ost->samples_count += 1024;
        ost->next_pts = ost->samples_count;
    }
    else
    {
        //zlog_error(get_category(), "[FFMPEG] get audio packet from queue error");
        //return NULL;
    }

    ret = write_audio_packet(oc, &(ost->enc->time_base), ost->st, pkt);
    if (ret != 0)
    {
        printf("[FFMPEG] write video frame error");
        return -1;
    }
    else
    {
        
    }

    return 0;
}

void *video_venc_thread(void *args)
{
    pthread_detach(pthread_self());
    MEDIA_BUFFER mb = NULL;
    static RK_S32 packet_cnt = 0;

    S_VENC_PROC_ARG venc_arg = *(S_VENC_PROC_ARG *)args;
    free(args);

    while (1)
    {
        mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_VENC, venc_arg.venc_id, -1);
        if (!mb)
        {
            printf("[VENC][PROC] task id: %u, venc id: %u, get media buffer error\n", venc_arg.task_id, venc_arg.venc_id);
            break;
        }

        int naluType = RK_MPI_MB_GetFlag(mb);
        video_data_packet_t *video_data_packet = (video_data_packet_t *)malloc(sizeof(video_data_packet_t));
        memset(video_data_packet->buffer, 0, MAX_PACK_BUFFER_SIZE);
        memcpy(video_data_packet->buffer, RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetSize(mb));
        video_data_packet->video_frame_size = RK_MPI_MB_GetSize(mb);
        video_data_packet->frame_flag = naluType;
        video_queue->putVideoPacketQueue(video_data_packet);
        printf("#naluType = %d \n", naluType);
        RK_MPI_MB_ReleaseBuffer(mb);
    }

    MPP_CHN_S vi_chn;
    MPP_CHN_S venc_chn;

    vi_chn.enModId = RK_ID_VI;
    vi_chn.s32ChnId = 1;

    venc_chn.enModId = RK_ID_VENC;
    venc_chn.s32ChnId = venc_arg.venc_id;

    int ret;
    ret = RK_MPI_SYS_UnBind(&vi_chn, &venc_chn);
    if (ret != 0)
    {
        printf("[VI] vi id: %d bind venc id: %d, ret: %d error\n", vi_chn.s32ChnId, venc_chn.s32ChnId, ret);
    }
    else
    {
        printf("[VI] vi id: %d bind venc id: %d success\n", vi_chn.s32ChnId, venc_chn.s32ChnId);
    }

    ret = RK_MPI_VENC_DestroyChn(0);
    if (ret)
    {
        printf("ERROR: Destroy VENC[0] error! ret=%d\n", ret);
        return 0;
    }
    // destroy vi
    ret = RK_MPI_VI_DisableChn(S32CAMID, 0);
    if (ret)
    {
        printf("ERROR: Destroy VI[0] error! ret=%d\n", ret);
        return 0;
    }

    return NULL;
}

void *audio_aenc_thread(void *args)
{
    pthread_detach(pthread_self());
    MEDIA_BUFFER mb = NULL;
    static RK_S32 packet_cnt = 0;

    S_AENC_PROC_ARG aenc_arg = *(S_AENC_PROC_ARG *)args;
    free(args);

    while (1)
    {
        mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_AENC, aenc_arg.aenc_id, -1);
        if (!mb)
        {
            printf("[VENC][PROC] task id: %u, venc id: %u, get media buffer error\n", aenc_arg.task_id, aenc_arg.aenc_id);
            break;
        }

        audio_data_packet_t *audio_data_packet = (audio_data_packet_t *)malloc(sizeof(audio_data_packet_t));
        memset(audio_data_packet->buffer, 0, MAX_PACK_BUFFER_SIZE);
        memcpy(audio_data_packet->buffer, RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetSize(mb));
        audio_data_packet->audio_frame_size = RK_MPI_MB_GetSize(mb);
        audio_queue->putAudioPacketQueue(audio_data_packet);
        RK_MPI_MB_ReleaseBuffer(mb);
    }

    MPP_CHN_S ai_chn;
    MPP_CHN_S aenc_chn;

    ai_chn.enModId = RK_ID_AI;
    ai_chn.s32ChnId = 0;

    aenc_chn.enModId = RK_ID_AENC;
    aenc_chn.s32ChnId = aenc_arg.aenc_id;

    int ret;
    ret = RK_MPI_SYS_UnBind(&ai_chn, &aenc_chn);
    if (ret != 0)
    {
        printf("[VI] vi id: %d bind venc id: %d, ret: %d error\n", ai_chn.s32ChnId, aenc_chn.s32ChnId, ret);
    }
    else
    {
        printf("[VI] vi id: %d bind venc id: %d success\n", ai_chn.s32ChnId, aenc_chn.s32ChnId);
    }

    ret = RK_MPI_AENC_DestroyChn(0);
    if (ret)
    {
        printf("ERROR: Destroy VENC[0] error! ret=%d\n", ret);
        return 0;
    }
    // destroy vi
    ret = RK_MPI_AI_DisableChn(0);
    if (ret)
    {
        printf("ERROR: Destroy VI[0] error! ret=%d\n", ret);
        return 0;
    }

    return NULL;
}

void *push_server_thread(void *args)
{
    pthread_detach(pthread_self());
    S_FFMPEG_GROUP old_arg = *(S_FFMPEG_GROUP *)args;
    free(args);

    AVOutputFormat *fmt = NULL;

    int ret;

    while (1)
    {
        ret = av_compare_ts(old_arg.video_st.next_pts,
                            old_arg.video_st.enc->time_base,
                            old_arg.audio_st.next_pts,
                            old_arg.audio_st.enc->time_base);

        if (ret <= 0)
        {
            printf("handle_video\n");
            ret = handle_video_packet(old_arg.oc, &old_arg.video_st);
            if (ret == -1)
            {
                printf("handle video packet error break");
                break;
            }
        }
        else
        {
            printf("handle_audio\n");
            ret = handle_audio_packet(old_arg.oc, &old_arg.audio_st);
            if (ret == -1)
            {
                printf("handle audio packet error break");
                break;
            }
        }
    }
    av_write_trailer(old_arg.oc);

#if 1
    fmt = old_arg.oc->oformat;
    if (fmt->video_codec != AV_CODEC_ID_NONE)
    {
        close_stream(old_arg.oc, &old_arg.video_st);
    }

    if (fmt->audio_codec != AV_CODEC_ID_NONE)
    {
        close_stream(old_arg.oc, &old_arg.audio_st);
    }
#endif

    if (!(fmt->flags & AVFMT_NOFILE))
    {
        avio_closep(&old_arg.oc->pb);
    }

    avformat_free_context(old_arg.oc);
}