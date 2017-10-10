#include "video_recording.h"
#include <QFile>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avresample.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
}

static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, QFile& outfile)
{
    int ret;
    /* send the frame to the encoder */
    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "error sending a frame for encoding\n");
        return;
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "error during encoding\n");
            return;
        }
        printf("encoded frame %3""ld"" (size=%5d)\n", pkt->pts, pkt->size);
        outfile.write((const char*)(pkt->data), pkt->size);
        av_packet_unref(pkt);
    }
}

VideoRecording::VideoRecording()
{
    const char *filename = "video1.avi";
    const AVCodec *codec;
    AVCodecContext *c = NULL;
    int i, ret, x, y;

    AVFrame *picture;
    AVPacket *pkt;
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };

    QFile file(filename);
    if(!file.open(QIODevice::ReadWrite))
    {
         return;
    }

    avcodec_register_all();
    /* find the mpeg1video encoder */
    codec = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        return;
    }
    c = avcodec_alloc_context3(codec);
    picture = av_frame_alloc();
    pkt = av_packet_alloc();
    if (!pkt)
        return;
    /* put sample parameters */
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = 352;
    c->height = 288;
    /* frames per second */
    c->time_base = { 1, 25 };
    c->framerate = { 25, 1 };
    c->gop_size = 10; /* emit one intra frame every ten frames */
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "could not open codec\n");
        return;
    }

    picture->format = c->pix_fmt;
    picture->width = c->width;
    picture->height = c->height;
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "could not alloc the frame data\n");
        return;
    }
    /* encode 1 second of video */
    for (i = 0; i < 25; i++) {
        fflush(stdout);
        /* make sure the frame data is writable */
        ret = av_frame_make_writable(picture);
        if (ret < 0)
            return;
        /* prepare a dummy image */
        /* Y */
        for (y = 0; y < c->height; y++) {
            for (x = 0; x < c->width; x++) {
                picture->data[0][y * picture->linesize[0] + x] = x + y + i * 3;
            }
        }
        /* Cb and Cr */
        for (y = 0; y < c->height / 2; y++) {
            for (x = 0; x < c->width / 2; x++) {
                picture->data[1][y * picture->linesize[1] + x] = 128 + y + i * 2;
                picture->data[2][y * picture->linesize[2] + x] = 64 + x + i * 5;
            }
        }
        picture->pts = i;
        /* encode the image */
        encode(c, picture, pkt, file);
    }
    /* flush the encoder */
    encode(c, NULL, pkt, file);
    /* add sequence end code to have a real MPEG file */
    file.write((const char*)endcode, sizeof(endcode) );
    file.close();
    avcodec_free_context(&c);
    av_frame_free(&picture);
    av_packet_free(&pkt);
}


VideoRecording::~VideoRecording()
{
}

