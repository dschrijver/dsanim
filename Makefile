COMPILER = gcc
ARCHIVER = ar
OPT_FLAGS = -std=c11 -O3
DEBUG_FLAGS = -Wall -Wextra

all: raylib
	rm -f *.o
	rm -f *.a
	$(COMPILER) -c dsanim.c -o dsanim.o $(OPT_FLAGS) $(DEBUG_FLAGS)
	$(ARCHIVER) x raylib/src/libraylib.a
	$(ARCHIVER) rcs libdsanim.a *.o
	rm -f *.o

raylib:
	git clone https://github.com/raysan5/raylib.git
	cd raylib/src;\
	sed -i '/SUPPORT_CUSTOM_FRAME_CONTROL/s|^[[:space:]]*//[[:space:]]*||' config.h;\
	make PLATFORM=PLATFORM_DESKTOP;\
