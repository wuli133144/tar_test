cmake_minimum_required(VERSION 2.6)
PROJECT(sunlands_test)

AUX_SOURCE_DIRECTORY(./ SRC_LIST)

#SET(LIBRARY_OUTPUT_PATH ./)
SET(EXECUTABLE_OUTPUT_PATH  ./)
SET(PB_LIB_DIR ../base/pb/lib/linux)
SET(SRC_LSIT ./)

ADD_DEFINITIONS( -g -W -O2 -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
-DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
-Wno-literal-suffix -Wno-unused-variable -Wno-unused-function
-Wno-sign-compare -Wno-deprecated -Wno-unused-parameter
-DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11)

INCLUDE_DIRECTORIES(./ ../base ../base/pb/protocol ../base/jsoncpp/ ../base/pb/)

LINK_DIRECTORIES( /usr/local/lib /im/IMFrame/src/branch/Group/server/base   /im/IMFrame/src/branch/Group/server/base/slog/lib /im/IMFrame/src/branch/Group/server/base/pb/lib/linux   /im/IMFrame/src/branch/Group/server/pb/gen/cpp   /usr/lib64
)

#FIND_LIBRARY(LIB_PATH  NAME base slog protobuf-lite PATH ../base ../base/slog/lib ${PB_LIB_DIR} ../base/pb/protocol )
#ADD_XXXX必须在TARGET_LINK_LIBRARIES前面，否则会报错

#IF ( NOT LIB_PATH)
  #MESSAGE(FATAL_ERROR "NOT FIND LIBBASE OR SLOG")
#ENDIF (NOT LIB_PATH)

#ADD_LIBRARY(${PROJECTNAME} SHARED/STATIC ${SRC_LIST})

ADD_EXECUTABLE(sunlands_test ${SRC_LIST}) 

TARGET_LINK_LIBRARIES(sunlands_test base slog curl protobuf-lite pthread  crypto)
#include "EncDec.h"
