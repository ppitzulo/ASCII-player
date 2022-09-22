# ASCII-player
Program written in C++ that plays an mp4 in the terminal. Initally created for the Bad Apple music video, but should work for most low resolution videos.

## Dependencies
The program requires SDL2, SDL2-mixer and a few FFmpeg libraries (avcodec, avformat, swscale and avutil) to compile.

## Usage
This program requires a mp4 file of your chosen video along with an mp3 of the audio. In the future I might find a way to extract the mp3 from the mp4 container directly with FFmpeg, but for now this hacky work around will have to do.
```
./ASCII_player ${video_file.mp4}
```
