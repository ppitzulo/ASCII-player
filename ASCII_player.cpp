extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/avconfig.h>
}
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <time.h>
#include <list>

static const char *badapplemp3 = "bad_apple.mp3";

using namespace std;

void frame2Ascii(AVFrame *pFrame) {
    char greyscale_ramp[] = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
    const int WIDTH = 6;
    const int HEIGHT = 12;
    int offset = 0;
    string str = "";

    for (int x = 0; x < pFrame->height; x += HEIGHT) {
        str = "";
        for (int y = 0; y < pFrame->width; y += WIDTH) {
            offset = pFrame->data[0][y + x * pFrame->linesize[0]];
            if (offset < 16) {
                str += " ";
            } else if (offset >= 16 && offset <= 235) {
                str += greyscale_ramp[(offset - 16) * 23/73];
            } else {
                str += "$";
            }
        }
        cout << str << '\n';
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
    system("setterm -cursor off");
    const AVCodec *pCodec = NULL;
    AVCodecParameters *pCodecParameters = NULL;
    int video_stream_index = -1;

    // loop though all the streams and print its main information
    for (int i = 0; i < pFormatContext->nb_streams; i++) {
        AVCodecParameters *pLocalCodecParameters = NULL;
        pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
        const AVCodec *pLocalCodec = NULL;

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


    int result = 0;
    int flags = MIX_INIT_MP3;
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Failed to init SDL\n");
        exit(1);
    }
    if (flags != (result = Mix_Init(flags))) {
        printf("Could not initialize mixer (result: %d).\n", result);
        printf("Mix_Init: %s\n", Mix_GetError());
        exit(1);
    }
    struct timespec time, time2;
    time.tv_sec = 0;
    time.tv_nsec = 33333300;
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Music *music = Mix_LoadMUS(badapplemp3);
    int playing_music = 0;
    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        // if it's the video stream
        if (pPacket->stream_index == video_stream_index) {
            response = avcodec_send_packet(pCodecContext, pPacket);
            response = avcodec_receive_frame(pCodecContext, pFrame);
            if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                break;
            } else if (response < 0) {
                return response;
            }
            if (!playing_music) {
                Mix_PlayMusic(music, 1);
                playing_music = 1;
            }
            frame2Ascii(pFrame);
            usleep(32000);
        }
        av_packet_unref(pPacket);
    }

    avformat_close_input(&pFormatContext);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    avcodec_free_context(&pCodecContext);
    system("setterm -cursor on");
    return 0;
}
