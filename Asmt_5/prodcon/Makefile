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

prodcon_sem: prodcon_sem.cpp
	$(CC) $(FLAGS) prodcon_sem.cpp $(LIBS) \
	-o prodcon_sem$(OUT_EXT)

prodcon_cv: prodcon_cv.cpp
	$(CC) $(FLAGS) prodcon_cv.cpp $(LIBS) \
	-o prodcon_cv$(OUT_EXT)

clean:
	rm -f *.o *.exe prodcon_sem prodcon_cv
