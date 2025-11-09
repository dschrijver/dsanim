COMPILER = gcc
OPT_FLAGS = -std=c11 -O3
DEBUG_FLAGS = -Wall -Wextra
LIBS_FLAGS = -lm
DSANIM_FLAGS = -I../ -L../ -l:../libdsanim.a
TEST_CASES = $(wildcard */)

all: .PHONY

.PHONY: $(TEST_CASES:/=)

.SILENT:
$(TEST_CASES:/=): 
	cd ../;\
	make
	cd $@;\
	rm -f test.out *.h5 *.dat;\
	$(COMPILER) main.c -o turbulence.out $(OPT_FLAGS) $(DSANIM_FLAGS) $(LIBS_FLAGS);\
	./turbulence.out