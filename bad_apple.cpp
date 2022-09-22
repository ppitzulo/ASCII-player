extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using namespace std;
// using namespace cv;
void logging(const char *string) {
    int x = 1;
}

int convertToGreyscale(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame) {
    const char greyscale_ramp[] = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
    const int WIDTH = 1;
    const int HEIGHT = 1;
    int value = 0;
     int response = avcodec_send_packet(pCodecContext, pPacket);
  int test = 0;
  int offset = 0;
  while (response >= 0)
  {
    response = avcodec_receive_frame(pCodecContext, pFrame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    } else if (response < 0) {
      // logging("Error while receiving a frame from the decoder: %s", av_err2str(response));
      return response;
    }
    for (int x = 0; x < pFrame->height; x += HEIGHT) {
        for (int y = 0; y < pFrame->width/3; y += WIDTH) {
            // Vec3b & bgrPixel = frame->at<Vec3b>(x, y);

            // const int test = .21 * bgrPixel.val[2] + .7152 * bgrPixel.val[1]  + .0722 * bgrPixel[0];
            // int channel = 0;
            // bgrPixel.val[channel] = test;
            // value = (bgrPixel.val[channel]);
            offset = (y * 3 + x * pFrame->linesize[0]);
            value = .21 * pFrame->data[0][offset] + .7152 * pFrame->data[0][offset + 1] + .0722 * pFrame->data[0][offset + 2];
            // value = (value/3) - 5;
            // if (value == 73) {
            //   value = value - 2;
            // }
//            cout << value << endl;
            // if (value > 50) {
            // cout << value << endl;
            // }
            cout << greyscale_ramp[value * 14/17]; // 14/17 i think
        }
        cout << "\n";
   }
}
}


static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame)
{
  int response = avcodec_send_packet(pCodecContext, pPacket);
  int test = 0;
  while (response >= 0)
  {
    response = avcodec_receive_frame(pCodecContext, pFrame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    } else if (response < 0) {
      // logging("Error while receiving a frame from the decoder: %s", av_err2str(response));
      return response;
    }
      // wait until you plug this into the ascii function
    for (int x = 0; x < pFrame->height; x++) { //used to be width
      for (int y = 0; y < pFrame->width; y++) { //used to be height
        // printf("%d\n", pFrame->data[0][y * pFrame->linesize[0] + x]);
        test = pFrame->data[0][x * pFrame->linesize[0] + x];
        if (test != 16 && test != 17) {
          printf("%d\n", test);
        }
      }
    }
  }
  return 0;
}


int main(int argc, const char *argv[])
{
  if (argc < 2) {
    printf("You need to specify a media file.\n");
    return -1;
  }
  
  logging("initializing all the containers, codecs and protocols.");

  AVFormatContext *pFormatContext = avformat_alloc_context();
  if (!pFormatContext) {
    logging("ERROR could not allocate memory for Format Context");
    return -1;
  }

  if (avformat_open_input(&pFormatContext, argv[1], NULL, NULL) != 0) {
    logging("ERROR could not open the file");
    return -1;
  }

  if (avformat_find_stream_info(pFormatContext,  NULL) < 0) {
    logging("ERROR could not get the stream info");
    return -1;
  }

  AVCodec *pCodec = NULL;
  AVCodecParameters *pCodecParameters =  NULL;
  int video_stream_index = -1;

  // loop though all the streams and print its main information
  for (int i = 0; i < pFormatContext->nb_streams; i++)
  {
    AVCodecParameters *pLocalCodecParameters =  NULL;
    pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
    AVCodec *pLocalCodec = NULL;

    // finds the registered decoder for a codec ID
    pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

    if (pLocalCodec==NULL) {
      logging("ERROR unsupported codec!");
      // In this example if the codec is not found we just skip it
      continue;
    }

    // when the stream is a video we store its index, codec parameters and codec
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
  if (!pCodecContext)
  {
    logging("failed to allocated memory for AVCodecContext");
    return -1;
  }

  // Fill the codec context based on the values from the supplied codec parameters
  // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
  if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0)
  {
    logging("failed to copy codec params to codec context");
    return -1;
  }

  // Initialize the AVCodecContext to use the given AVCodec.
  // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
  if (avcodec_open2(pCodecContext, pCodec, NULL) < 0)
  {
    logging("failed to open codec through avcodec_open2");
    return -1;
  }

  AVFrame *pFrame = av_frame_alloc();
  AVPacket *pPacket = av_packet_alloc();
  

  int response = 0;
  int how_many_packets_to_process = 8;

  while (av_read_frame(pFormatContext, pPacket) >= 0)
  {
    // if it's the video stream
    if (pPacket->stream_index == video_stream_index) {
      response = convertToGreyscale(pPacket, pCodecContext, pFrame);
      
      usleep(30000);  
      // stop it, otherwise we'll be saving hundreds of frames
      // if (--how_many_packets_to_process <= 0) break;
    }
    av_packet_unref(pPacket);
  }

  logging("releasing all the resources");

  avformat_close_input(&pFormatContext);
  av_packet_free(&pPacket);
  av_frame_free(&pFrame);
  avcodec_free_context(&pCodecContext);
  return 0;
}

// int main(int argc, char** argv)
// {
//     if (argc == 1) {
//         cout << "Please input a video file" << endl;
//         exit(EXIT_FAILURE);
//     }
//     VideoCapture cap(argv[1]);


//      // if not success, exit program
//      if (cap.isOpened() == false)
//      {
//       cout << "Cannot open the video file" << endl;
//       cin.get(); //wait for any key press
//       return -1;
//      }

//      //Uncomment the following line if you want to start the video in the middle
//      //cap.set(CAP_PROP_POS_MSEC, 300);

//      //get the frames rate of the video
//      double fps = cap.get(CAP_PROP_FPS);
//      cout << "Frames per seconds : " << fps << endl;

//      String window_name = "Bad Apple";

// //     namedWindow(window_name, WINDOW_NORMAL); //create a window

//      while (true)
//      {
//       Mat frame;
//       bool bSuccess = cap.read(frame); // read a new frame from video
//       //Breaking the while loop at the end of the video
//       if (bSuccess == false)
//       {
//        cout << "Found the end of the video" << endl;
//        break;
//       }

//       convertToGreyscale(&frame);
//       //show the frame in the created window
// //      imshow(window_name, frame);

//       //wait for for 10 ms until any key is pressed.
//       //If the 'Esc' key is pressed, break the while loop.
//       //If the any other key is pressed, continue the loop
//       //If any key is not pressed withing 10 ms, continue the loop
//       usleep(30000);
// //      if (waitKey(30) == 27)
// //      {
// //       cout << "Esc key is pressed by user. Stoppig the video" << endl;
// //       break;
// //      }
// //      return 0;
// }
//      return 0;
// }
