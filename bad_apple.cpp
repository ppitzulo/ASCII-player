extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avconfig.h>
}
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using namespace std;
// using namespace cv;


int frame2Ascii(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame) {
    const char greyscale_ramp[] = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
    const int WIDTH = 2;
    const int HEIGHT = 4;
    int offset = 0;

    for (int x = 0; x < pFrame->height; x += HEIGHT) {
        for (int y = 0; y < pFrame->width; y += WIDTH) {
            offset = pFrame->data[0][y + x * pFrame->linesize[0]];
            if (offset < 16) {
                cout << " ";
            } else if (offset >= 16 && offset <= 235) {
                cout << greyscale_ramp[(offset - 16) * 69 / 219];
            } else {
                cout << "$";
            }
        }
        cout << "\n";
    }
}


int main(int argc, const char *argv[])
{
    if (argc < 2) {
        printf("You need to specify a media file.\n");
        return -1;
    }


    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (!pFormatContext) {
        cout << "ERROR could not allocate memory for Format Context" << endl;
        return -1;
    }

    if (avformat_open_input(&pFormatContext, argv[1], NULL, NULL) != 0) {
        cout << "ERROR could not open the file" << endl;
        return -1;
    }

    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
        cout << "ERROR could not get the stream info" << endl;
        return -1;
    }

    AVCodec *pCodec = NULL;
    AVCodecParameters *pCodecParameters = NULL;
    int video_stream_index = -1;

    // loop though all the streams and print its main information
    for (int i = 0; i < pFormatContext->nb_streams; i++) {
        AVCodecParameters *pLocalCodecParameters = NULL;
        pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
        AVCodec *pLocalCodec = NULL;

        // finds the registered decoder for a codec ID
        pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

        if (pLocalCodec == NULL) {
            cout << "ERROR unsupported codec!" << endl;
            continue;
        }

        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video_stream_index == -1) {
                video_stream_index = i;
                pCodec = pLocalCodec;
                pCodecParameters = pLocalCodecParameters;
            }

        }
        // else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {

        // }
    }


    // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
    AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
    if (!pCodecContext) {
        cout << "failed to allocated memory for AVCodecContext" << endl;
        return -1;
    }

    // Fill the codec context based on the values from the supplied codec parameters
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
        cout << "failed to copy codec params to codec context" << endl;
        return -1;
    }

    // Initialize the AVCodecContext to use the given AVCodec.
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
    if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
        cout << "failed to open codec through avcodec_open2" << endl;
        return -1;
    }

    AVFrame *pFrame = av_frame_alloc();
    AVPacket *pPacket = av_packet_alloc();


    int response = 0;
    int how_many_packets_to_process = 8;

    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        // if it's the video stream
        if (pPacket->stream_index == video_stream_index) {
            response = avcodec_send_packet(pCodecContext, pPacket);
            response = avcodec_receive_frame(pCodecContext, pFrame);
            if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                cout << "test";
                break;
            } else if (response < 0) {
//                cout << "Error while receiving frame from decoder: " << av_err2str(response) << endl;
                return response;
            }
            response = frame2Ascii(pPacket, pCodecContext, pFrame);
            usleep(30000);
        }
        av_packet_unref(pPacket);
    }

    avformat_close_input(&pFormatContext);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    avcodec_free_context(&pCodecContext);
    return 0;
}