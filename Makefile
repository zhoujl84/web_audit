#****************************************************************************
#
# Makefile for TinyXml test.
# Lee Thomason
# www.grinninglizard.com
#
# This is a GNU make (gmake) makefile
#****************************************************************************

# DEBUG can be set to YES to include debugging info, or NO otherwise
DEBUG          := NO

# PROFILE can be set to YES to include profiling info, or NO otherwise
PROFILE        := NO

# TINYXML_USE_STL can be used to turn on STL support. NO, then STL
# will not be used. YES will include the STL files.
TINYXML_USE_STL := NO

#****************************************************************************

CC     := gcc
CXX    := g++
LD     := g++
AR     := ar rc
RANLIB := ranlib

DEBUG_CFLAGS     := -Wall -Wno-format -g -DDEBUG
RELEASE_CFLAGS   := -Wall -Wno-unknown-pragmas -Wno-format -O3

LIBS		 := -lpthread -L third_party/libcurl/curl-7.30.0/lib/.libs/ -lcurl

DEBUG_CXXFLAGS   := ${DEBUG_CFLAGS} 
RELEASE_CXXFLAGS := ${RELEASE_CFLAGS}

DEBUG_LDFLAGS    := -g
RELEASE_LDFLAGS  :=

ifeq (YES, ${DEBUG})
   CFLAGS       := ${DEBUG_CFLAGS}
   CXXFLAGS     := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CFLAGS       := ${RELEASE_CFLAGS}
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

ifeq (YES, ${PROFILE})
   CFLAGS   := ${CFLAGS} -pg -O3
   CXXFLAGS := ${CXXFLAGS} -pg -O3
   LDFLAGS  := ${LDFLAGS} -pg
endif

#****************************************************************************
# Preprocessor directives
#****************************************************************************

ifeq (YES, ${TINYXML_USE_STL})
  DEFS := -DTIXML_USE_STL
else
  DEFS :=
endif

#****************************************************************************
# Include paths
#****************************************************************************

#INCS := -I/usr/include/g++-2 -I/usr/local/include
INCS := -I include  -I third_party/tinyxml/include  -I third_party/libcurl/curl-7.30.0/include


#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS}   ${DEFS}
CXXFLAGS := ${CXXFLAGS} ${DEFS}

#****************************************************************************
# Targets of the build
#****************************************************************************

OUTPUT := a.out

all: dependency_libcurl init ${OUTPUT}


#****************************************************************************
# Source files
#****************************************************************************
SRCDIR = third_party/tinyxml/src src
OBJ_PATH = objs
PREFIX_BIN = /usr/local/bin/

#C_SOURCES = $(wildcard *.c)
C_SRCDIR = $(SRCDIR)
C_SOURCES = $(foreach d,$(C_SRCDIR),$(wildcard $(d)/*.c) )
C_OBJS = $(patsubst %.c, $(OBJ_PATH)/%.o, $(C_SOURCES))

#CPP_SOURCES = $(wildcard *.cpp)
CXX_SRCDIR = $(SRCDIR)
CXX_SOURCES = $(foreach d,$(CXX_SRCDIR),$(wildcard $(d)/*.cpp) )
CXX_OBJS = $(patsubst %.cpp, $(OBJ_PATH)/%.o, $(CXX_SOURCES))


$(C_OBJS):$(OBJ_PATH)/%.o:%.c
	$(CC) -c $(CFLAGS) $(INCS) $< -o $@

$(CXX_OBJS):$(OBJ_PATH)/%.o:%.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) $< -o $@


#****************************************************************************
# Output
#****************************************************************************

${OUTPUT}: ${C_OBJS} ${CXX_OBJS}
	${LD} -o $@ ${LDFLAGS} ${C_OBJS} ${CXX_OBJS} ${LIBS} ${EXTRA_LIBS}



#****************************************************************************
# Targets of the build
#****************************************************************************

default: dependency_libcurl init compile

dependency_libcurl:
	cd ./third_party/libcurl/curl-7.30.0/  &&  make all
	cp -f third_party/libcurl/curl-7.30.0/lib/.libs/libcurl.so* ./

init:
	$(foreach d,$(SRCDIR), [[ -d $(OBJ_PATH)/$(d) ]] || mkdir -p $(OBJ_PATH)/$(d);)

test:
	@echo "C_SOURCES: $(C_SOURCES)"
	@echo "C_OBJS: $(C_OBJS)"
	@echo "CXX_SOURCES: $(CXX_SOURCES)"
	@echo "CXX_OBJS: $(CXX_OBJS)"

compile:$(C_OBJS) $(CXX_OBJS)
	$(CXX)  $^ -o $(OUTPUT)  $(LINKFLAGS) $(LIBS)

clean:
	rm -rf $(OBJ_PATH)
	rm -f $(OUTPUT)
	rm -f libcurl.so*

install: dependency_libcurl init $(OUTPUT)
	cp -f $(OUTPUT) $(PREFIX_BIN)

uninstall:
	rm -f $(PREFIX_BIN)/$(OUTPUT)

rebuild: clean dependency_libcurl init compile
