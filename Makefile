OBJS = bad_apple.cpp

OBJ_NAME = bad_apple

all : $(OBJS)
		g++ $(OBJS) -w -lavcodec -lavformat -lswscale  -lavutil  -lSDL2 -lSDL2_mixer 