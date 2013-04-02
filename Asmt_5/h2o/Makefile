OS="LINUX"
#OS="WIN32"

ifeq ($(OS), "LINUX")
  CC=g++
  FLAGS=-g -DLINUX
  LIBS=-lpthread
  GFX_LIBS=-lglut -lGL -lGLU -lm
  OUT_EXT=
endif

ifeq ($(OS), "WIN32")
  ## cross compiling from linux
  MINGW_BIN=/usr/local/bin#/opt/xmingw/bin  
  MINGW_BASE=/usr/local/i386-mingw32#/opt/xmingw/i386-mingw32msvc
  CC=$(MINGW_BIN)/i386-mingw32-g++#${MINGW_BIN}/i386-mingw32msvc-g++
  FLAGS=-DWIN32 -g
  INCS=-I$(MINGW_BASE)/include
  LIBS=-L$(MINGW_BASE)/lib -lm -lgdi32 -lpthreadGC1 -lwinmm
  GFX_LIBS=-lglut32 -lopengl32 -lglu32
  OUT_EXT=.exe
endif

all: h2o_sem h2o_stats

h2o_cv: h2o_cv.o
	$(CC) $(FLAGS) h2o_cv.o $(LIBS) \
	-o h2o_cv$(OUT_EXT)

h2o_cv.o: h2o_cv.cpp
	$(CC) $(FLAGS) h2o_cv.cpp \
	-c -o h2o_cv.o

h2o_sem: h2o_sem.o
	$(CC) $(FLAGS) h2o_sem.o $(LIBS) \
	-o h2o_sem$(OUT_EXT)

h2o_sem.o: h2o_sem.cpp
	$(CC) $(FLAGS) h2o_sem.cpp \
	-c -o h2o_sem.o

h2o_stats: h2o_stats.cpp
	$(CC) $(FLAGS) h2o_stats.cpp \
	-o h2o_stats$(OUT_EXT)

clean:
	rm -f *.o *.exe h2o_sem h2o_cv h2o_stats
