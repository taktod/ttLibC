cmake_minimum_required(VERSION 2.8)

project(ttLibC)

set(PACKAGE ttlibc)
set(TTLIBC_DESCRIPTION ttLibC library)
set(VERSION 1.1.0)

set(TT_DEFAULT ON)
if(WIN32)
  set(TT_DEFAULT OFF)
endif()

set(TT_LICENSE "GPL" CACHE STRING "")
option(TT_LICENSE    "target license"     "GPL")
option(TT_DEBUG      "Use debug output"   OFF)
option(TT_TEST       "compile test code"  OFF)

option(TT_FILE       "use file feature"   ${TT_DEFAULT})
option(TT_SOCKET     "use socket feature" ${TT_DEFAULT})

option(TT_FAAC       "enable faac"        ${TT_DEFAULT})
option(TT_FDKAAC     "enable fdkaac"      ${TT_DEFAULT})
option(TT_FFMPEG     "enable ffmpeg"      ${TT_DEFAULT})
option(TT_JPEG       "enable jpeg"        ${TT_DEFAULT})
option(TT_LIBPNG     "enable libpng"      ${TT_DEFAULT})
option(TT_LIBYUV     "enable libyuv"      ${TT_DEFAULT})
option(TT_MP3LAME    "enable mp3lame"     ${TT_DEFAULT})
option(TT_OPENAL     "enable openal"      ${TT_DEFAULT})
option(TT_OPENCV     "enable opencv"      ${TT_DEFAULT})
option(TT_OPENH264   "enable openh264"    ${TT_DEFAULT})
option(TT_OPUS       "enable opus"        ${TT_DEFAULT})
option(TT_SOUNDTOUCH "enable soundtouch"  ${TT_DEFAULT})
option(TT_SPEEX      "enable speex"       ${TT_DEFAULT})
option(TT_SPEEXDSP   "enable speexdsp"    ${TT_DEFAULT})
option(TT_THEORA     "enable theora"      ${TT_DEFAULT})
option(TT_VORBIS     "enable vorbis"      ${TT_DEFAULT})
option(TT_X264       "enable x264"        ${TT_DEFAULT})
option(TT_X265       "enable x265"        ${TT_DEFAULT})

file(WRITE ${CMAKE_BINARY_DIR}/ttLibC_def.h "")
if(WIN32)
  file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_WIN32__\n")
  add_definitions(-D__ENABLE_WIN32__)
  set(TTLIBC_ENCODER_HEADERS
    ${TTLIBC_ENCODER_HEADERS}
    ttLibC/encoder/msAacEncoder.h
    ttLibC/encoder/msH264Encoders.h)
  set(TTLIBC_RESAMPLER_HEADERS
    ${TTLIBC_RESAMPLER_HEADERS}
    ttLibC/resampler/msImageResampler.h
    ttLibC/resampler/msImageResampler.hpp)
  set(TTLIBC_UTIL_HEADERS
    ${TTLIBC_UTIL_HEADERS}
    ttLibC/util/mmAudioLoopbackUtil.h
    ttLibC/util/msAudioCapturerUtil.h
    ttLibC/util/msGlobalUtil.h
    ttLibC/util/msVideoCapturerUtil.h)
else()
  set(CMAKE_CXX_FLAGS "-std=c++0x")
  if(APPLE)
    # for rpath
    set(CMAKE_MACOSX_RPATH 1)
  endif()
  # use pkgconfig
  find_package(PkgConfig QUIET)
  # check header file for library
  function(checkLibraryHeader TARGET RESULT)
    FILE(WRITE "check.c" "#include <${TARGET}> \nint main(){return 0;}")
    execute_process(
      COMMAND gcc -E check.c
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
      OUTPUT_VARIABLE TT_RESULT
      ERROR_QUIET)
    if(TT_RESULT MATCHES [a-zA-z\\/]+${TARGET})
      string(REGEX MATCH [a-zA-z\\/]+${TARGET} TT_RESULT ${TT_RESULT})
      string(REPLACE ${TARGET} "" TT_RESULT ${TT_RESULT})
      set(${RESULT} ${TT_RESULT} PARENT_SCOPE)
    else()
      set(${RESULT} 0 PARENT_SCOPE)
    endif()
    FILE(REMOVE "check.c")
  endfunction()
endif()

message("")
# headers
set(TTLIBC_HEADERS
  ttLibC/allocator.h
  ttLibC/log.h
  ttLibC/ttLibC.h)
set(TTLIBC_CONTAINER_HEADERS
  ttLibC/container/container.h
  ttLibC/container/flv.h
  ttLibC/container/mkv.h
  ttLibC/container/mp3.h
  ttLibC/container/mp4.h
  ttLibC/container/mpegts.h)
set(TTLIBC_FRAME_AUDIO_HEADERS
  ttLibC/frame/audio/aac.h
  ttLibC/frame/audio/adpcmImaWav.h
  ttLibC/frame/audio/audio.h
  ttLibC/frame/audio/mp3.h
  ttLibC/frame/audio/nellymoser.h
  ttLibC/frame/audio/opus.h
  ttLibC/frame/audio/pcmAlaw.h
  ttLibC/frame/audio/pcmf32.h
  ttLibC/frame/audio/pcmMulaw.h
  ttLibC/frame/audio/pcms16.h
  ttLibC/frame/audio/speex.h
  ttLibC/frame/audio/vorbis.h)
set(TTLIBC_FRAME_VIDEO_HEADERS
  ttLibC/frame/video/bgr.h
  ttLibC/frame/video/flv1.h
  ttLibC/frame/video/h264.h
  ttLibC/frame/video/h265.h
  ttLibC/frame/video/jpeg.h
  ttLibC/frame/video/png.h
  ttLibC/frame/video/theora.h
  ttLibC/frame/video/video.h
  ttLibC/frame/video/vp6.h
  ttLibC/frame/video/vp8.h
  ttLibC/frame/video/vp9.h
  ttLibC/frame/video/wmv1.h
  ttLibC/frame/video/wmv2.h
  ttLibC/frame/video/yuv420.h)
set(TTLIBC_FRAME_HEADERS
  ttLibC/frame/frame.h)
set(TTLIBC_RESAMPLER_HEADERS
  ttLibC/resampler/audioResampler.h
  ttLibC/resampler/imageResampler.h
  ttLibC/resampler/imageResizer.h)
set(TTLIBC_UTIL_HEADERS
  ${TTLIBC_UTIL_HEADERS}
  ttLibC/util/amfUtil.h
  ttLibC/util/beepUtil.h
  ttLibC/util/byteUtil.h
  ttLibC/util/crc32Util.h
  ttLibC/util/dynamicBufferUtil.h
  ttLibC/util/hexUtil.h
  ttLibC/util/ioUtil.h
  ttLibC/util/stlListUtil.h
  ttLibC/util/stlMapUtil.h
  ttLibC/util/tetty2.h)

# License (gpl lgpl bsd)
message("Result ...")
if(TT_LICENSE STREQUAL "GPL")
  configure_file("gpl-3.0.txt" "LICENSE" COPYONLY)
  message("license:    GPLv3")
  file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_GPL__\n")
  add_definitions(-D__ENABLE_GPL__)
elseif(TT_LICENSE STREQUAL "LGPL")
  configure_file("lgpl-3.0.txt" "LICENSE" COPYONLY)
  file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_LGPL__\n")
  message("license:    LGPLv3")
  add_definitions(-D__ENABLE_LGPL__)
else()
  configure_file("3causeBSD.txt" "LICENSE" COPYONLY)
  message("license:    The 3-clause BSD")
endif()

# debug any
if(TT_DEBUG OR TT_TEST)
  message("debug:      on")
  add_definitions(-D__DEBUG_FLAG__)
  set(debug_flag 1)
else()
  message("debug:      off")
  set(debug_flag 0)
endif()
configure_file(ttLibC/log.h.in ${CMAKE_CURRENT_SOURCE_DIR}/ttLibC/log.h @ONLY)

# test(i wanna use gtest...) gpl -> any
if(TT_TEST)
  message("test:       on")
  include_directories(".")
else()
  message("test:       off")
endif()

# file any
if(TT_FILE)
  message("file:       on")
  file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_FILE__\n")
  add_definitions(-D__ENABLE_FILE__)
  set(TTLIBC_UTIL_HEADERS
    ${TTLIBC_UTIL_HEADERS}
      ttLibC/util/forkUtil.h)
else()
  message("file:       off")
endif()
# socket any
if(TT_SOCKET)
  message("socket:     on")
  file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_SOCKET__\n")
  add_definitions(-D__ENABLE_SOCKET__)
  set(TTLIBC_NET_HEADERS
    ${TTLIBC_NET_HEADERS}
    ttLibC/net/net.h
    ttLibC/net/tcp.h
    ttLibC/net/tetty.h
    ttLibC/net/udp.h)
  set(TTLIBC_NET_CLIENT_HEADERS
    ${TTLIBC_NET_CLIENT_HEADERS}
    ttLibC/net/client/rtmp.h
    ttLibC/net/client/websocket.h)
  set(TTLIBC_NET_TETTY2_HEADERS
    ${TTLIBC_NET_TETTY2_HEADERS}
    ttLibC/net/tetty2/tcpBootstrap.h)
else()
  message("socket:     off")
endif()

if(TT_FILE AND TT_SOCKET)
  set(TTLIBC_UTIL_HEADERS
    ${TTLIBC_UTIL_HEADERS}
      ttLibC/util/httpUtil.h)
endif()

# apple any for osx or ios
if(APPLE)
  find_library(TT_AUDIOTOOLBOX   AudioToolbox)
  find_library(TT_VIDEOTOOLBOX   VideoToolbox)
  find_library(TT_COREFOUNDATION CoreFoundation)
  find_library(TT_AUDIOUNIT      AudioUnit)
  find_library(TT_COREMEDIA      CoreMedia)
  find_library(TT_COREVIDEO      CoreVideo)
  find_library(TT_COREAUDIO      CoreAudio)
  if(TT_AUDIOTOOLBOX AND TT_VIDEOTOOLBOX AND TT_COREFOUNDATION AND TT_AUDIOUNIT AND TT_COREMEDIA AND TT_COREVIDEO AND TT_COREAUDIO)
    message("apple:      on")
    set(TT_APPLE_LIBRARIES "-framework AudioToolbox -framework VideoToolbox -framework CoreFoundation -framework AudioUnit -framework CoreMedia -framework CoreVideo -framework CoreAudio")
    file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_APPLE__\n")
    add_definitions(-D__ENABLE_APPLE__)
    set(TTLIBC_DECODER_HEADERS
      ${TTLIBC_DECODER_HEADERS}
      ttLibC/decoder/audioConverterDecoder.h
      ttLibC/decoder/vtDecompressSessionDecoder.h)
    set(TTLIBC_ENCODER_HEADERS
      ${TTLIBC_ENCODER_HEADERS}
      ttLibC/encoder/audioConverterEncoder.h
      ttLibC/encoder/vtCompressSessionEncoder.h)
    set(TTLIBC_UTIL_HEADERS
      ${TTLIBC_UTIL_HEADERS}
      ttLibC/util/audioUnitRecorderUtil.h
      ttLibC/util/audioUnitUtil.h)
  else()
    message("apple:      off")
  endif()
endif()

# faac LGPL
if(TT_FAAC)
  if(TT_LICENSE STREQUAL "GPL" OR TT_LICENSE STREQUAL "LGPL")
    if(WIN32)
      message("faac:       on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_FAAC__\n")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_FAAC_ENCODE__\n")
      add_definitions(-D__ENABLE_FAAC__ -D__ENABLE_FAAC_ENCODE__)
    else()
      checkLibraryHeader(faac.h TT_RESULT)
      if(TT_RESULT)
        message("faac:       on")
        file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_FAAC__\n")
        file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_FAAC_ENCODE__\n")
        add_definitions(-D__ENABLE_FAAC__ -D__ENABLE_FAAC_ENCODE__)
        include_directories(${TT_RESULT})
        link_directories(/usr/local/lib)
        set(TT_FAAC_LIBRARIES "faac")
        set(TTLIBC_ENCODER_HEADERS
          ${TTLIBC_ENCODER_HEADERS}
          ttLibC/encoder/faacEncoder.h)
      else()
        message("faac:       off(library is missing)")
      endif()
    endif()
  else()
    message("faac:       off(under lgpl only)")
  endif()
else()
  message("faac:       off")
endif()

if(TT_FDKAAC)
  if(TT_LICENSE STREQUAL "GPL" OR TT_LICENSE STREQUAL "LGPL")
    if(WIN32)
      message("fdkaac:     on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_FDKAAC__\n")
      add_definitions(-D__ENABLE_FDKAAC__)
    else()
      pkg_check_modules(TT_FDKAAC fdk-aac QUIET)
      if(TT_FDKAAC_LIBRARIES)
        message("fdkaac:     on")
        file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_FDKAAC__\n")
        add_definitions(-D__ENABLE_FDKAAC__)
        include_directories(${TT_FDKAAC_INCLUDE_DIRS})
        set(TTLIBC_ENCODER_HEADERS
          ${TTLIBC_ENCODER_HEADERS}
          ttLibC/encoder/fdkaacEncoder.h)
      else()
        message("fdkaac:     off(library is missing)")
      endif()
    endif()
  else()
    message("fdkaac:     off(under lgpl only)")
  endif()
else()
  message("fdkaac:     off")
endif()

# ffmpeg lgpl
if(TT_FFMPEG)
  if(TT_LICENSE STREQUAL "GPL" OR TT_LICENSE STREQUAL "LGPL")
    if(WIN32)
      message("ffmpeg:     on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_AVCODEC__\n")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_SWSCALE__\n")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_SWRESAMPLE__\n")
      add_definitions(-D__ENABLE_AVCODEC__ -D__ENABLE_SWSCALE__ -D__ENABLE_SWRESAMPLE__)
    else()
      pkg_check_modules(TT_FFMPEG libavcodec libswscale libswresample QUIET)
      if(TT_FFMPEG_LIBRARIES)
        message("ffmpeg:     on")
        include_directories(${TT_FFMPEG_INCLUDE_DIRS})
        file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_AVCODEC__\n")
        file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_SWSCALE__\n")
        file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_SWRESAMPLE__\n")
        add_definitions(-D__ENABLE_AVCODEC__ -D__ENABLE_SWSCALE__ -D__ENABLE_SWRESAMPLE__)
        set(TTLIBC_DECODER_HEADERS
          ${TTLIBC_DECODER_HEADERS}
          ttLibC/decoder/avcodecDecoder.h)
        set(TTLIBC_ENCODER_HEADERS
          ${TTLIBC_ENCODER_HEADERS}
          ttLibC/encoder/avcodecEncoder.h)
        set(TTLIBC_RESAMPLER_HEADERS
          ${TTLIBC_RESAMPLER_HEADERS}
          ttLibC/resampler/swscaleResampler.h
          ttLibC/resampler/swresampleResampler.h)
      else()
        message("ffmpeg:     off(library is missing)")
      endif()
    endif()
  else()
    message("ffmpeg:     off(under lgpl only)")
  endif()
else()
  message("ffmpeg:     off")
endif()

# jpeg any
if(TT_JPEG)
  if(WIN32)
    message("jpeg:       on")
    file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_JPEG__\n")
    add_definitions(-D__ENABLE_JPEG__)
  else()
    checkLibraryHeader(jpeglib.h TT_RESULT)
    if(TT_RESULT)
      message("jpeg:       on")
      include_directories(${TT_RESULT})
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_JPEG__\n")
      add_definitions(-D__ENABLE_JPEG__)
      link_directories(/usr/local/lib)
      set(TT_JPEG_LIBRARIES "jpeg")
      set(TTLIBC_ENCODER_HEADERS
        ${TTLIBC_ENCODER_HEADERS}
        ttLibC/encoder/jpegEncoder.h)
      set(TTLIBC_DECODER_HEADERS
        ${TTLIBC_DECODER_HEADERS}
        ttLibC/decoder/jpegDecoder.h)
    else()
      message("jpeg:       off(library is missing)")
    endif()
  endif()
else()
  message("jpeg:       off")
endif()

# libpng any
if(TT_LIBPNG)
  if(WIN32)
    message("libpng:     on")
    file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_LIBPNG__\n")
    add_definitions(-D__ENABLE_LIBPNG__)
  else()
    pkg_check_modules(TT_LIBPNG libpng QUIET)
    if(TT_LIBPNG_LIBRARIES)
      message("libpng:     on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_LIBPNG__\n")
      add_definitions(-D__ENABLE_LIBPNG__)
      include_directories(${TT_LIBPNG_INCLUDE_DIRS})
      set(TTLIBC_DECODER_HEADERS
        ${TTLIBC_DECODER_HEADERS}
        ttLibC/decoder/pngDecoder.h)
    else()
      message("libpng:     off(library is missing)")
    endif()
  endif()
else()
  message("libpng:     off")
endif()

# libyuv any
if(TT_LIBYUV)
  if(WIN32)
    message("libyuv:     on")
    file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_LIBYUV__\n")
    add_definitions(-D__ENABLE_LIBYUV__)
  else()
    checkLibraryHeader(libyuv.h TT_RESULT)
    if(TT_RESULT)
      message("libyuv:     on")
      include_directories(${TT_RESULT})
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_LIBYUV__\n")
      add_definitions(-D__ENABLE_LIBYUV__)
      link_directories(/usr/local/lib)
      set(TT_LIBYUV_LIBRARIES "yuv")
      set(TTLIBC_RESAMPLER_HEADERS
        ${TTLIBC_RESAMPLER_HEADERS}
        ttLibC/resampler/libyuvResampler.h)
    else()
      message("libyuv:     off(library is missing)")
    endif()
  endif()
else()
  message("libyuv:     off")
endif()

# mp3lame GPL
if(TT_MP3LAME)
  if(TT_LICENSE STREQUAL "GPL" OR TT_LICENSE STREQUAL "LGPL")
    if(WIN32)
      message("mp3lame:    on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_MP3LAME__\n")
      add_definitions(-D__ENABLE_MP3LAME__)
      if(TT_LICENSE STREQUAL "GPL")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_MP3LAME_DECODE__\n")
      add_definitions(-D__ENABLE_MP3LAME_DECODE__)
      endif()
    else()
      checkLibraryHeader(lame/lame.h TT_RESULT)
      if(TT_RESULT)
        message("mp3lame:    on")
        file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_MP3LAME__\n")
        add_definitions(-D__ENABLE_MP3LAME__)
        set(TTLIBC_ENCODER_HEADERS
          ${TTLIBC_ENCODER_HEADERS}
          ttLibC/encoder/mp3lameEncoder.h)
        if(TT_LICENSE STREQUAL "GPL")
          file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_MP3LAME_DECODE__\n")
          add_definitions(-D__ENABLE_MP3LAME_DECODE__)
          set(TTLIBC_DECODER_HEADERS
            ${TTLIBC_DECODER_HEADERS}
            ttLibC/decoder/mp3lameDecoder.h)
        endif()
        include_directories(${TT_RESULT})
        link_directories(/usr/local/lib)
        set(TT_MP3LAME_LIBRARIES "mp3lame")
      else()
        message("mp3lame:    off(library is missing)")
      endif()
    endif()
  else()
    message("mp3lame:    off(under lgpl only)")
  endif()
else()
  message("mp3lame:    off")
endif()

# openal GPL
if(TT_OPENAL)
  if(TT_LICENSE STREQUAL "GPL")
    if(APPLE)
      find_library(TT_OPENAL OpenAL)
      if(TT_OPENAL)
        message("openal:     on")
        set(TT_OPENAL_LIBRARIES "-framework OpenAL")
        file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_OPENAL__\n")
        add_definitions(-D__ENABLE_OPENAL__)
        set(TTLIBC_UTIL_HEADERS
          ${TTLIBC_UTIL_HEADERS}
          ttLibC/util/openalUtil.h)
      else()
        message("openal:     off(library is missing)")
      endif()
    elseif(WIN32)
      message("openal:     on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_OPENAL__\n")
      add_definitions(-D__ENABLE_OPENAL__)
    else()
      pkg_check_modules(TT_OPENAL openal QUIET)
      if(TT_OPENAL_LIBRARIES)
        message("openal:     on")
        file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_OPENAL__\n")
        add_definitions(-D__ENABLE_OPENAL__)
        include_directories(${TT_OPENAL_INCLUDE_DIRS})
        set(TTLIBC_UTIL_HEADERS
          ${TTLIBC_UTIL_HEADERS}
          ttLibC/util/openalUtil.h)
      else()
        message("openal:     off(library is missing)")
      endif()
    endif()
  else()
    message("openal:     off(under gpl only)")
  endif()
else()
  message("openal:     off")
endif()

# opencv any
if(TT_OPENCV)
  if(WIN32)
    message("opencv:     on")
    file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_OPENCV__\n")
    add_definitions(-D__ENABLE_OPENCV__)
  else()
    pkg_check_modules(TT_OPENCV opencv QUIET)
    if(TT_OPENCV_LIBRARIES)
      message("opencv:     on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_OPENCV__\n")
      add_definitions(-D__ENABLE_OPENCV__)
      include_directories(${TT_OPENCV_INCLUDE_DIRS})
      set(TTLIBC_UTIL_HEADERS
        ${TTLIBC_UTIL_HEADERS}
        ttLibC/util/opencvUtil.h)
    else()
      pkg_check_modules(TT_OPENCV opencv4 QUIET)
      if(TT_OPENCV_LIBRARIES)
        message("opencv:     on")
        file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_OPENCV__\n")
        add_definitions(-D__ENABLE_OPENCV__)
        include_directories(${TT_OPENCV_INCLUDE_DIRS})
        set(TTLIBC_UTIL_HEADERS
          ${TTLIBC_UTIL_HEADERS}
          ttLibC/util/opencvUtil.h)
      else()
        message("opencv:     off(library is missing)")
      endif()
    endif()
  endif()
else()
  message("opencv:     off")
endif()

# openh264 any
if(TT_OPENH264)
  if(WIN32)
    message("openh264:   on")
    file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_OPENH264__\n")
    add_definitions(-D__ENABLE_OPENH264__)
  else()
    pkg_check_modules(TT_OPENH264 openh264 QUIET)
    if(TT_OPENH264_LIBRARIES)
      message("openh264:   on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_OPENH264__\n")
      add_definitions(-D__ENABLE_OPENH264__)
      include_directories(${TT_OPENH264_INCLUDE_DIRS})
      set(TTLIBC_ENCODER_HEADERS
        ${TTLIBC_ENCODER_HEADERS}
        ttLibC/encoder/openh264Encoder.h)
      set(TTLIBC_DECODER_HEADERS
        ${TTLIBC_DECODER_HEADERS}
        ttLibC/decoder/openh264Decoder.h)
    else()
      message("openh264:   off(library is missing)")
    endif()
  endif()
else()
  message("openh264:   off")
endif()

# opus any
if(TT_OPUS)
  if(WIN32)
    message("opus:       on")
    file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_OPUS__\n")
    add_definitions(-D__ENABLE_OPUS__)
  else()
    pkg_check_modules(TT_OPUS opus QUIET)
    if(TT_OPUS_LIBRARIES)
      message("opus:       on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_OPUS__\n")
      add_definitions(-D__ENABLE_OPUS__)
      include_directories(${TT_OPUS_INCLUDE_DIRS})
      set(TTLIBC_ENCODER_HEADERS
        ${TTLIBC_ENCODER_HEADERS}
        ttLibC/encoder/opusEncoder.h)
      set(TTLIBC_DECODER_HEADERS
        ${TTLIBC_DECODER_HEADERS}
        ttLibC/decoder/opusDecoder.h)
    else()
      message("opus:       off(library is missing)")
    endif()
  endif()
else()
  message("opus:       off")
endif()

# soundtouch lgpl
if(TT_SOUNDTOUCH)
  if(TT_LICENSE STREQUAL "GPL" OR TT_LICENSE STREQUAL "LGPL")
    if(WIN32)
      message("soundtouch: on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_SOUNDTOUCH__\n")
      add_definitions(-D__ENABLE_SOUNDTOUCH__)
    else()
      pkg_check_modules(TT_SOUNDTOUCH soundtouch QUIET)
      if(TT_SOUNDTOUCH_LIBRARIES)
        message("soundtouch: on")
        file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_SOUNDTOUCH__\n")
        add_definitions(-D__ENABLE_SOUNDTOUCH__)
        include_directories(${TT_SOUNDTOUCH_INCLUDE_DIRS})
        set(TTLIBC_RESAMPLER_HEADERS
          ${TTLIBC_RESAMPLER_HEADERS}
          ttLibC/resampler/soundtouchResampler.h)
      else()
        message("soundtouch: off(library is missing)")
      endif()
    endif()
  else()
    message("soundtouch: off(under lgpl only)")
  endif()
else()
  message("soundtouch: off")
endif()

# speex any
if(TT_SPEEX)
  if(WIN32)
    message("speex:      on")
    file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_SPEEX__\n")
    add_definitions(-D__ENABLE_SPEEX__)
  else()
    pkg_check_modules(TT_SPEEX speex QUIET)
    if(TT_SPEEX_LIBRARIES)
      message("speex:      on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_SPEEX__\n")
      add_definitions(-D__ENABLE_SPEEX__)
      include_directories(${TT_SPEEX_INCLUDE_DIRS})
      set(TTLIBC_ENCODER_HEADERS
        ${TTLIBC_ENCODER_HEADERS}
        ttLibC/encoder/speexEncoder.h)
      set(TTLIBC_DECODER_HEADERS
        ${TTLIBC_DECODER_HEADERS}
        ttLibC/decoder/speexDecoder.h)
    else()
      message("speex:      off(library is missing)")
    endif()
  endif()
else()
  message("speex:      off")
endif()

# speexdsp any
if(TT_SPEEXDSP)
  if(WIN32)
    message("speexdsp:   on")
    file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_SPEEXDSP__\n")
    add_definitions(-D__ENABLE_SPEEXDSP__)
  else()
    pkg_check_modules(TT_SPEEXDSP speexdsp QUIET)
    if(TT_SPEEXDSP_LIBRARIES)
      message("speexdsp:   on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_SPEEXDSP__\n")
      add_definitions(-D__ENABLE_SPEEXDSP__)
      include_directories(${TT_SPEEXDSP_INCLUDE_DIRS})
      set(TTLIBC_RESAMPLER_HEADERS
        ${TTLIBC_RESAMPLER_HEADERS}
        ttLibC/resampler/speexdspResampler.h)
    else()
      message("speexdsp:   off(library is missing)")
    endif()
  endif()
else()
  message("speexdsp:   off")
endif()

# theora any
if(TT_THEORA)
  if(WIN32)
    message("theora:     on")
    file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_THEORA__\n")
    add_definitions(-D__ENABLE_THEORA__)
  else()
    pkg_check_modules(TT_THEORA theora QUIET)
    if(TT_THEORA_LIBRARIES)
      message("theora:     on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_THEORA__\n")
      add_definitions(-D__ENABLE_THEORA__)
      include_directories(${TT_THEORA_INCLUDE_DIRS})
      set(TTLIBC_ENCODER_HEADERS
        ${TTLIBC_ENCODER_HEADERS}
        ttLibC/encoder/theoraEncoder.h)
      set(TTLIBC_DECODER_HEADERS
        ${TTLIBC_DECODER_HEADERS}
        ttLibC/decoder/theoraDecoder.h)
    else()
      message("theora:     off(library is missing)")
    endif()
  endif()
else()
  message("theora:     off")
endif()

# vorbis any
if(TT_VORBIS)
  if(WIN32)
    message("vorbis:     on")
    file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_VORBIS_DECODE__\n")
    file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_VORBIS_ENCODE__\n")
    add_definitions(-D__ENABLE_VORBIS_DECODE__ -D__ENABLE_VORBIS_ENCODE__)
  else()
    pkg_check_modules(TT_VORBIS vorbis vorbisenc QUIET)
    if(TT_VORBIS_LIBRARIES)
      message("vorbis:     on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_VORBIS_DECODE__\n")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_VORBIS_ENCODE__\n")
      add_definitions(-D__ENABLE_VORBIS_DECODE__ -D__ENABLE_VORBIS_ENCODE__)
      include_directories(${TT_VORBIS_INCLUDE_DIRS})
      set(TTLIBC_ENCODER_HEADERS
        ${TTLIBC_ENCODER_HEADERS}
        ttLibC/encoder/vorbisEncoder.h)
      set(TTLIBC_DECODER_HEADERS
        ${TTLIBC_DECODER_HEADERS}
        ttLibC/decoder/vorbisDecoder.h)
    else()
      message("vorbis:     off(library is missing)")
    endif()
  endif()
else()
  message("vorbis:     off")
endif()

# x264 GPL
if(TT_X264)
  if(TT_LICENSE STREQUAL "GPL")
    if(WIN32)
      message("x264:       on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_X264__\n")
      add_definitions(-D__ENABLE_X264__)
    else()
      pkg_check_modules(TT_X264 x264 QUIET)
      if(TT_X264_LIBRARIES)
        message("x264:       on")
        file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_X264__\n")
        add_definitions(-D__ENABLE_X264__)
        include_directories(${TT_X264_INCLUDE_DIRS})
        set(TTLIBC_ENCODER_HEADERS
          ${TTLIBC_ENCODER_HEADERS}
          ttLibC/encoder/x264Encoder.h)
      else()
        message("x264:       off(library is missing)")
      endif()
    endif()
  else()
    message("x264:       off(under gpl only)")
  endif()
else()
  message("x264:       off")
endif()

# x265 GPL
if(TT_X265)
  if(TT_LICENSE STREQUAL "GPL")
    if(WIN32)
      message("x265:       on")
      file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_X265__\n")
      add_definitions(-D__ENABLE_X265__)
    else()
      pkg_check_modules(TT_X265 x265 QUIET)
      if(TT_X265_LIBRARIES)
        message("x265:       on")
        file(APPEND ${CMAKE_BINARY_DIR}/ttLibC_def.h "#define __ENABLE_X265__\n")
        add_definitions(-D__ENABLE_X265__)
        include_directories(${TT_X265_INCLUDE_DIRS})
        set(TTLIBC_ENCODER_HEADERS
          ${TTLIBC_ENCODER_HEADERS}
          ttLibC/encoder/x265Encoder.h)
      else()
        message("x265:       off(library is missing)")
      endif()
    endif()
  else()
    message("x265:       off(under gpl only)")
  endif()
else()
  message("x265:       off")
endif()

if(TT_TEST)
  add_subdirectory(gtest)
  enable_testing()
endif()

add_library(ttLibC SHARED
  ttLibC/allocator.c
  ttLibC/net/udp.c
  ttLibC/net/tetty/promise.c
  ttLibC/net/tetty/context.c
  ttLibC/net/tetty/bootstrap.c
  ttLibC/net/tcp.c
  ttLibC/net/tetty2/tcpBootstrap.c
  ttLibC/net/net.c
  ttLibC/net/client/rtmp2/rtmpConnection.c
  ttLibC/net/client/rtmp2/message/amf0Command.c
  ttLibC/net/client/rtmp2/message/aggregateMessage.c
  ttLibC/net/client/rtmp2/message/videoMessage.c
  ttLibC/net/client/rtmp2/message/amf0DataMessage.c
  ttLibC/net/client/rtmp2/message/audioMessage.c
  ttLibC/net/client/rtmp2/message/setPeerBandwidth.c
  ttLibC/net/client/rtmp2/message/userControlMessage.c
  ttLibC/net/client/rtmp2/message/windowAcknowledgementSize.c
  ttLibC/net/client/rtmp2/message/acknowledgement.c
  ttLibC/net/client/rtmp2/message/setChunkSize.c
  ttLibC/net/client/rtmp2/message/rtmpMessage.c
  ttLibC/net/client/rtmp2/tetty2/rtmpEncoder.c
  ttLibC/net/client/rtmp2/tetty2/rtmpClientHandler.c
  ttLibC/net/client/rtmp2/tetty2/rtmpCommandHandler.c
  ttLibC/net/client/rtmp2/tetty2/rtmpHandshake.c
  ttLibC/net/client/rtmp2/tetty2/rtmpDecoder.c
  ttLibC/net/client/rtmp2/rtmpStream.c
  ttLibC/net/client/rtmp2/data/clientObject.c
  ttLibC/net/client/rtmp2/header/rtmpHeader.c
  ttLibC/net/client/websocket2/handler.c
  ttLibC/net/client/websocket2/handshake.c
  ttLibC/net/client/websocket2/websocket.c
  ttLibC/util/flvFrameUtil.c
  ttLibC/util/openalUtil.c
  ttLibC/util/crc32Util.c
  ttLibC/util/tetty2/promise.c
  ttLibC/util/tetty2/context.c
  ttLibC/util/tetty2/bootstrap.c
  ttLibC/util/dynamicBufferUtil.c
  ttLibC/util/linkedListUtil.c
  ttLibC/util/amfUtil.c
  ttLibC/util/forkUtil.c
  ttLibC/util/ioUtil.c
  ttLibC/util/hexUtil.c
  ttLibC/util/httpUtil.c
  ttLibC/util/audioUnitUtil.c
  ttLibC/util/byteUtil.c
  ttLibC/util/beepUtil.c
  ttLibC/encoder/theoraEncoder.c
  ttLibC/encoder/x264Encoder.c
  ttLibC/encoder/x265Encoder.c
  ttLibC/encoder/jpegEncoder.c
  ttLibC/encoder/faacEncoder.c
  ttLibC/encoder/fdkaacEncoder.c
  ttLibC/encoder/mp3lameEncoder.c
  ttLibC/encoder/speexEncoder.c
  ttLibC/encoder/avcodecEncoder.c
  ttLibC/encoder/audioConverterEncoder.c
  ttLibC/encoder/vtCompressSessionEncoder.c
  ttLibC/encoder/opusEncoder.c
  ttLibC/encoder/vorbisEncoder.c
  ttLibC/ttLibC.c
  ttLibC/decoder/vorbisDecoder.c
  ttLibC/decoder/opusDecoder.c
  ttLibC/decoder/audioConverterDecoder.c
  ttLibC/decoder/pngDecoder.c
  ttLibC/decoder/speexDecoder.c
  ttLibC/decoder/avcodecDecoder.c
  ttLibC/decoder/mp3lameDecoder.c
  ttLibC/decoder/vtDecompressSessionDecoder.c
  ttLibC/decoder/jpegDecoder.c
  ttLibC/decoder/theoraDecoder.c
  ttLibC/frame/video/bgr.c
  ttLibC/frame/video/flv1.c
  ttLibC/frame/video/jpeg.c
  ttLibC/frame/video/vp9.c
  ttLibC/frame/video/video.c
  ttLibC/frame/video/yuv420.c
  ttLibC/frame/video/h265.c
  ttLibC/frame/video/wmv1.c
  ttLibC/frame/video/h264.c
  ttLibC/frame/video/theora.c
  ttLibC/frame/video/wmv2.c
  ttLibC/frame/video/vp8.c
  ttLibC/frame/video/png.c
  ttLibC/frame/video/vp6.c
  ttLibC/frame/audio/vorbis.c
  ttLibC/frame/audio/pcmf32.c
  ttLibC/frame/audio/pcmMulaw.c
  ttLibC/frame/audio/pcmAlaw.c
  ttLibC/frame/audio/opus.c
  ttLibC/frame/audio/adpcmImaWav.c
  ttLibC/frame/audio/aac.c
  ttLibC/frame/audio/speex.c
  ttLibC/frame/audio/mp3.c
  ttLibC/frame/audio/pcms16.c
  ttLibC/frame/audio/audio.c
  ttLibC/frame/audio/nellymoser.c
  ttLibC/frame/frame.c
  ttLibC/resampler/swresampleResampler.c
  ttLibC/resampler/swscaleResampler.c
  ttLibC/resampler/libyuvResampler.c
  ttLibC/resampler/imageResizer.c
  ttLibC/resampler/speexdspResampler.c
  ttLibC/resampler/imageResampler.c
  ttLibC/resampler/audioResampler.c
  ttLibC/container/flv/flvReader.c
  ttLibC/container/flv/type/headerTag.c
  ttLibC/container/flv/type/audioTag.c
  ttLibC/container/flv/type/metaTag.c
  ttLibC/container/flv/type/videoTag.c
  ttLibC/container/flv/flvTag.c
  ttLibC/container/flv/flvWriter.c
  ttLibC/container/mpegts/mpegtsReader.c
  ttLibC/container/mpegts/type/pes.c
  ttLibC/container/mpegts/type/sdt.c
  ttLibC/container/mpegts/type/pat.c
  ttLibC/container/mpegts/type/pmt.c
  ttLibC/container/mpegts/mpegtsPacket.c
  ttLibC/container/mpegts/mpegtsWriter.c
  ttLibC/container/mkv/mkvTag.c
  ttLibC/container/mkv/mkvReader.c
  ttLibC/container/mkv/mkvWriter.c
  ttLibC/container/mkv/type/simpleBlock.c
  ttLibC/container/misc2.c
  ttLibC/container/mp3/mp3Writer.c
  ttLibC/container/mp3/mp3Frame.c
  ttLibC/container/mp3/mp3Reader.c
  ttLibC/container/mp4/mp4Reader.c
  ttLibC/container/mp4/mp4Atom.c
  ttLibC/container/mp4/type/stts.c
  ttLibC/container/mp4/type/stco.c
  ttLibC/container/mp4/type/stsc.c
  ttLibC/container/mp4/type/ctts.c
  ttLibC/container/mp4/type/stsz.c
  ttLibC/container/mp4/type/elst.c
  ttLibC/container/mp4/type/trun.c
  ttLibC/container/mp4/mp4Writer.c
  ttLibC/container/container.c
  ttLibC/util/audioUnitRecorderUtil.cpp
  ttLibC/util/mmAudioLoopbackUtil.cpp
  ttLibC/util/msAudioCapturerUtil.cpp
  ttLibC/util/msGlobalUtil.cpp
  ttLibC/util/msVideoCapturerUtil.cpp
  ttLibC/util/opencvUtil.cpp
  ttLibC/util/stlListUtil.cpp
  ttLibC/util/stlMapUtil.cpp
  ttLibC/encoder/msAacEncoder.cpp
  ttLibC/encoder/msH264Encoder.cpp
  ttLibC/encoder/openh264Encoder.cpp
  ttLibC/decoder/openh264Decoder.cpp
  ttLibC/resampler/msImageResampler.cpp
  ttLibC/resampler/soundtouchResampler.cpp)

target_link_libraries(ttLibC PRIVATE
  ${TT_APPLE_LIBRARIES}
  ${TT_FAAC_LIBRARIES}
  ${TT_FDKAAC_LIBRARIES}
  ${TT_FFMPEG_LIBRARIES}
  ${TT_JPEG_LIBRARIES}
  ${TT_LIBPNG_LIBRARIES}
  ${TT_LIBYUV_LIBRARIES}
  ${TT_MP3LAME_LIBRARIES}
  ${TT_OPENAL_LIBRARIES}
  ${TT_OPENCV_LIBRARIES}
  ${TT_OPENH264_LIBRARIES}
  ${TT_OPUS_LIBRARIES}
  ${TT_SOUNDTOUCH_LIBRARIES}
  ${TT_SPEEX_LIBRARIES}
  ${TT_SPEEXDSP_LIBRARIES}
  ${TT_THEORA_LIBRARIES}
  ${TT_VORBIS_LIBRARIES}
  ${TT_X264_LIBRARIES}
  ${TT_X265_LIBRARIES})

if(WIN32)
  add_definitions("-DTT_ATTRIBUTE_INNER=")
  add_definitions("-DTT_ATTRIBUTE_API=__declspec(dllexport)")
else()
  set(prefix ${CMAKE_INSTALL_PREFIX})
  set(exec_prefix "\${prefix}")
  set(libdir "\${exec_prefix}/lib")
  set(includedir "\${prefix}/include")

  configure_file(ttLibC.pc.in ttLibC.pc @ONLY)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ttLibC.pc
    DESTINATION lib/pkgconfig)

  install(TARGETS ttLibC
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)
  install(FILES ${TTLIBC_HEADERS}
    DESTINATION include/ttLibC)
  install(FILES ${TTLIBC_CONTAINER_HEADERS}
    DESTINATION include/ttLibC/container)
  install(FILES ${TTLIBC_DECODER_HEADERS}
    DESTINATION include/ttLibC/decoder)
  install(FILES ${TTLIBC_ENCODER_HEADERS}
    DESTINATION include/ttLibC/encoder)
  install(FILES ${TTLIBC_FRAME_HEADERS}
    DESTINATION include/ttLibC/frame)
  install(FILES ${TTLIBC_FRAME_VIDEO_HEADERS}
    DESTINATION include/ttLibC/frame/video)
  install(FILES ${TTLIBC_FRAME_AUDIO_HEADERS}
    DESTINATION include/ttLibC/frame/audio)
  install(FILES ${TTLIBC_FRAME_AUDIO_HEADERS}
    DESTINATION include/ttLibC/frame/audio)
  install(FILES ${TTLIBC_NET_HEADERS}
    DESTINATION include/ttLibC/net)
  install(FILES ${TTLIBC_NET_CLIENT_HEADERS}
    DESTINATION include/ttLibC/net/client)
  install(FILES ${TTLIBC_NET_TETTY2_HEADERS}
    DESTINATION include/ttLibC/net/tetty2)
  install(FILES ${TTLIBC_RESAMPLER_HEADERS}
    DESTINATION include/ttLibC/resampler)
  install(FILES ${TTLIBC_UTIL_HEADERS}
    DESTINATION include/ttLibC/util)
endif()
