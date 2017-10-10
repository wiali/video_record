#include "audio_recording.h"
#include <math.h>
#define  _USE_MATH_DEFINES

#include <QFile>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/frame.h"
#include "libavutil/samplefmt.h"

#pragma comment(lib, "C:/A_video/video_record/video_record/libav/lib/avcodec.lib")
#pragma comment(lib, "C:/A_video/video_record/video_record/libav/lib/avdevice.lib")
#pragma comment(lib, "C:/A_video/video_record/video_record/libav/lib/avfilter.lib")
#pragma comment(lib, "C:/A_video/video_record/video_record/libav/lib/avformat.lib")
#pragma comment(lib, "C:/A_video/video_record/video_record/libav/lib/avresample.lib")
#pragma comment(lib, "C:/A_video/video_record/video_record/libav/lib/avutil.lib")
#pragma comment(lib, "C:/A_video/video_record/video_record/libav/lib/swscale.lib")
}

/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(const AVCodec *codec, enum AVSampleFormat sample_fmt)
{
    const enum AVSampleFormat *p = codec->sample_fmts;
    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sample_fmt)
            return 1;
        p++;
    }
    return 0;
}
/* just pick the highest supported samplerate */
static int select_sample_rate(const AVCodec *codec)
{
    const int *p;
    int best_samplerate = 0;
    if (!codec->supported_samplerates)
        return 44100;
    p = codec->supported_samplerates;
    while (*p) {
        best_samplerate = FFMAX(*p, best_samplerate);
        p++;
    }
    return best_samplerate;
}
/* select layout with the highest channel count */
static int select_channel_layout(const AVCodec *codec)
{
    const uint64_t *p;
    uint64_t best_ch_layout = 0;
    int best_nb_channels = 0;
    if (!codec->channel_layouts)
        return AV_CH_LAYOUT_STEREO;
    p = codec->channel_layouts;
    while (*p) {
        int nb_channels = av_get_channel_layout_nb_channels(*p);
        if (nb_channels > best_nb_channels) {
            best_ch_layout = *p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return best_ch_layout;
}
static void encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt,
    QFile& output)
{
    int ret;
    /* send the frame for encoding */
    ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "error sending the frame to the encoder\n");
        return;
    }
    /* read all the available output packets (in general there may be any
    * number of them */
    while (ret >= 0) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "error encoding audio frame\n");
            return;
        }
        output.write((const char*)pkt->data, pkt->size);
        av_packet_unref(pkt);
    }
}

AudioRecording::AudioRecording()
{
    const char *filename;
    const AVCodec *codec;
    AVCodecContext *c = NULL;
    AVFrame *frame;
    AVPacket *pkt;
    int i, j, k, ret;
    uint16_t *samples;
    float t, tincr;

    filename = "audio1.mp3";

    /* register all the codecs */
    avcodec_register_all();
    /* find the MP2 encoder */
    codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        return;
    }
    c = avcodec_alloc_context3(codec);
    /* put sample parameters */
    c->bit_rate = 64000;
    /* check that the encoder supports s16 pcm input */
    c->sample_fmt = AV_SAMPLE_FMT_S16;
    if (!check_sample_fmt(codec, c->sample_fmt)) {
        fprintf(stderr, "encoder does not support %s",
            av_get_sample_fmt_name(c->sample_fmt));
        return;
    }
    /* select other audio parameters supported by the encoder */
    c->sample_rate = select_sample_rate(codec);
    c->channel_layout = select_channel_layout(codec);
    c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "could not open codec\n");
        return;
    }

    QFile file(filename);
    if(!file.open(QIODevice::ReadWrite))
    {
         return;
    }

    /* packet for holding encoded output */
    pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "could not allocate the packet\n");
        return;
    }
    /* frame containing input raw audio */
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "could not allocate audio frame\n");
        return;
    }
    frame->nb_samples = c->frame_size;
    frame->format = c->sample_fmt;
    frame->channel_layout = c->channel_layout;
    /* allocate the data buffers */
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        fprintf(stderr, "could not allocate audio data buffers\n");
        return;
    }
    /* encode a single tone sound */
    t = 0;
    tincr = 2 * M_PI * 440.0 / c->sample_rate;
    for (i = 0; i<200; i++) {
        /* make sure the frame is writable -- makes a copy if the encoder
        * kept a reference internally */
        ret = av_frame_make_writable(frame);
        if (ret < 0)
            return;
        samples = (uint16_t*)frame->data[0];
        for (j = 0; j < c->frame_size; j++) {
            samples[2 * j] = (int)(sin(t) * 10000);
            for (k = 1; k < c->channels; k++)
                samples[2 * j + k] = samples[2 * j];
            t += tincr;
        }
        encode(c, frame, pkt, file);
    }
    /* flush the encoder */
    encode(c, NULL, pkt, file);
    file.close();
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&c);
}


AudioRecording::~AudioRecording()
{
}
