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

if(WIN32)
  # windowsシステム
  # とりあえず強制でENABLE付いてたら、フラグをopenしようか・・・
  add_compile_definitions(__ENABLE_WIN32__)
else()
  set(CMAKE_CXX_FLAGS "-std=c++0x")
  if(APPLE)
    # rpath対策
    set(CMAKE_MACOSX_RPATH 1)
  endif()
  # package
  find_package(PkgConfig QUIET)
  # includeファイルがあるか確認する動作
  function(checkHeader TARGET RESULT)
    FILE(WRITE "check.c" "#include <${TARGET}> \nint main(){return 0;}")
    execute_process(
      COMMAND gcc -E check.c
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
      OUTPUT_VARIABLE TT_RESULT
      ERROR_QUIET)
    string(REGEX MATCH [a-zA-z\\/]+${TARGET} TT_RESULT ${TT_RESULT})
    string(REPLACE ${TARGET} "" TT_RESULT ${TT_RESULT})
    set(${RESULT} ${TT_RESULT} PARENT_SCOPE)
    FILE(REMOVE "check.c")
  endfunction()
endif()

# それぞれのプログラム
# License (動作ライセンス) gpl lgpl bsd
message("")
message("Result ...")
if(TT_LICENSE STREQUAL "GPL")
  configure_file("gpl-3.0.txt" "LICENSE" COPYONLY)
message("license:    GPLv3")
  add_compile_definitions(__ENABLE_GPL__)
elseif(TT_LICENSE STREQUAL "LGPL")
  configure_file("lgpl-3.0.txt" "LICENSE" COPYONLY)
message("license:    LGPLv3")
  add_compile_definitions(__ENABLE_LGPL__)
else()
  configure_file("3causeBSD.txt" "LICENSE" COPYONLY)
message("license:    The 3-clause BSD")
endif()

# debug (ログ出力を調整) any
if(TT_DEBUG)
message("debug:      on")
  add_compile_definitions(__DEBUG_FLAG__)
  set(debug_flag 1)
else()
message("debug:      off")
  set(debug_flag 0)
endif()
configure_file(ttLibC/log.h.in ${CMAKE_CURRENT_SOURCE_DIR}/ttLibC/log.h @ONLY)


# test (unitテスト googletestに書き換える予定) gpl -> any
if(TT_TEST)
message("test:       on")
else()
message("test:       off")
endif()

# file any
if(TT_FILE)
message("file:       on")
  add_compile_definitions(__ENABLE_FILE__)
else()
message("file:       off")
endif()
# socket any
if(TT_SOCKET)
message("socket:     on")
  add_compile_definitions(__ENABLE_SOCKET__)
else()
message("socket:     off")
endif()

# apple any for apple
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
    add_compile_definitions(__ENABLE_APPLE__)
  else()
message("apple:      off")
  endif()
endif()

# faac LGPL
if(TT_FAAC)
  if(TT_LICENSE STREQUAL "GPL" OR TT_LICENSE STREQUAL "LGPL")
    if(WIN32)
      #windowsでOKになってる場合は、フラグをONにしてやる
message("faac:       on")
      add_compile_definitions(__ENABLE_FAAC__ __ENABLE_FAAC_ENCODE__)
    else()
      #widnows以外なら、コンパイル動作やってみて決める？
      checkHeader(faac.h TT_RESULT)
      if(TT_RESULT)
message("faac:       on")
        add_compile_definitions(__ENABLE_FAAC__ __ENABLE_FAAC_ENCODE__)
        include_directories(${TT_RESULT})
        link_directories(/usr/local/lib)
        set(TT_FAAC_LIBRARIES "faac")
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

# fdkaac lgpl
if(TT_FDKAAC)
  if(TT_LICENSE STREQUAL "GPL" OR TT_LICENSE STREQUAL "LGPL")
    if(WIN32)
      #windowsでOKになってる場合は、フラグをONにしてやる
message("fdkaac:     on")
      add_compile_definitions(__ENABLE_FDKAAC__)
    else()
      pkg_check_modules(TT_FDKAAC fdk-aac QUIET)
      if(TT_FDKAAC_LIBRARIES)
message("fdkaac:     on")
        add_compile_definitions(__ENABLE_FDKAAC__)
        include_directories(${TT_FDKAAC_INCLUDE_DIRS})
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
      #windowsでOKになってる場合は、フラグをONにしてやる
message("ffmpeg:     on")
      add_compile_definitions(__ENABLE_AVCODEC__ __ENABLE_SWSCALE__ __ENABLE_SWRESAMPLE__)
    else()
      pkg_check_modules(TT_FFMPEG libavcodec libswscale libswresample QUIET)
      if(TT_FFMPEG_LIBRARIES)
message("ffmpeg:     on")
        include_directories(${TT_FFMPEG_INCLUDE_DIRS})
        add_compile_definitions(__ENABLE_AVCODEC__ __ENABLE_SWSCALE__ __ENABLE_SWRESAMPLE__)
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
    #windowsでOKになってる場合は、フラグをONにしてやる
message("jpeg:       on")
    add_compile_definitions(__ENABLE_JPEG__)
  else()
    #widnows以外なら、コンパイル動作やってみて決める？
    checkHeader(jpeglib.h TT_RESULT)
    if(TT_RESULT)
message("jpeg:       on")
      include_directories(${TT_RESULT})
      add_compile_definitions(__ENABLE_JPEG__)
      link_directories(/usr/local/lib)
      set(TT_JPEG_LIBRARIES "jpeg")
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
    #windowsでOKになってる場合は、フラグをONにしてやる
message("libpng:     on")
    add_compile_definitions(__ENABLE_LIBPNG__)
  else()
    pkg_check_modules(TT_LIBPNG libpng QUIET)
    if(TT_LIBPNG_LIBRARIES)
message("libpng:     on")
      add_compile_definitions(__ENABLE_LIBPNG__)
      include_directories(${TT_LIBPNG_INCLUDE_DIRS})
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
    #windowsでOKになってる場合は、フラグをONにしてやる
message("libyuv:     on")
    add_compile_definitions(__ENABLE_LIBYUV__)
  else()
    #widnows以外なら、コンパイル動作やってみて決める？
    checkHeader(libyuv.h TT_RESULT)
    if(TT_RESULT)
message("libyuv:     on")
      include_directories(${TT_RESULT})
      add_compile_definitions(__ENABLE_LIBYUV__)
      link_directories(/usr/local/lib)
      set(TT_LIBYUV_LIBRARIES "yuv")
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
      #windowsでOKになってる場合は、フラグをONにしてやる
message("mp3lame:    on")
      add_compile_definitions(__ENABLE_MP3LAME__)
      if(TT_LICENSE STREQUAL "GPL")
        add_compile_definitions(__ENABLE_MP3LAME_DECODE__)
      endif()
    else()
      #widnows以外なら、コンパイル動作やってみて決める？
      checkHeader(lame/lame.h TT_RESULT)
      if(TT_RESULT)
message("mp3lame:    on")
        add_compile_definitions(__ENABLE_MP3LAME__)
        if(TT_LICENSE STREQUAL "GPL")
          add_compile_definitions(__ENABLE_MP3LAME_DECODE__)
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
        add_compile_definitions(__ENABLE_OPENAL__)
      else()
message("openal:     off(library is missing)")
      endif()
    elseif(WIN32)
message("openal:     on")
      add_compile_definitions(__ENABLE_OPENAL__)
    else()
      pkg_check_modules(TT_OPENAL openal QUIET)
      if(TT_OPENAL_LIBRARIES)
message("openal:     on")
        add_compile_definitions(__ENABLE_OPENAL__)
        include_directories(${TT_OPENAL_INCLUDE_DIRS})
      else()
message("libpng:     off(library is missing)")
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
    #windowsでOKになってる場合は、フラグをONにしてやる
message("opencv:     on")
    add_compile_definitions(__ENABLE_OPENCV__)
  else()
    pkg_check_modules(TT_OPENCV opencv QUIET)
    if(TT_OPENCV_LIBRARIES)
message("opencv:     on")
      add_compile_definitions(__ENABLE_OPENCV__)
      include_directories(${TT_OPENCV_INCLUDE_DIRS})
    else()
      pkg_check_modules(TT_OPENCV opencv4 QUIET)
      if(TT_OPENCV_LIBRARIES)
message("opencv:     on")
        add_compile_definitions(__ENABLE_OPENCV__)
        include_directories(${TT_OPENCV_INCLUDE_DIRS})
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
    #windowsでOKになってる場合は、フラグをONにしてやる
message("openh264:   on")
    add_compile_definitions(__ENABLE_OPENH264__)
  else()
    pkg_check_modules(TT_OPENH264 openh264 QUIET)
    if(TT_OPENH264_LIBRARIES)
message("openh264:   on")
      add_compile_definitions(__ENABLE_OPENH264__)
      include_directories(${TT_OPENH264_INCLUDE_DIRS})
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
    #windowsでOKになってる場合は、フラグをONにしてやる
message("opus:       on")
    add_compile_definitions(__ENABLE_OPUS__)
  else()
    pkg_check_modules(TT_OPUS opus QUIET)
    if(TT_OPUS_LIBRARIES)
message("opus:       on")
      add_compile_definitions(__ENABLE_OPUS__)
      include_directories(${TT_OPUS_INCLUDE_DIRS})
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
      #windowsでOKになってる場合は、フラグをONにしてやる
message("soundtouch: on")
      add_compile_definitions(__ENABLE_SOUNDTOUCH__)
    else()
      pkg_check_modules(TT_SOUNDTOUCH soundtouch QUIET)
      if(TT_SOUNDTOUCH_LIBRARIES)
message("soundtouch: on")
        add_compile_definitions(__ENABLE_SOUNDTOUCH__)
        include_directories(${TT_SOUNDTOUCH_INCLUDE_DIRS})
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
    #windowsでOKになってる場合は、フラグをONにしてやる
message("speex:      on")
    add_compile_definitions(__ENABLE_SPEEX__)
  else()
    pkg_check_modules(TT_SPEEX speex QUIET)
    if(TT_SPEEX_LIBRARIES)
message("speex:      on")
      add_compile_definitions(__ENABLE_SPEEX__)
      include_directories(${TT_SPEEX_INCLUDE_DIRS})
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
    #windowsでOKになってる場合は、フラグをONにしてやる
message("speexdsp:   on")
    add_compile_definitions(__ENABLE_SPEEXDSP__)
  else()
    pkg_check_modules(TT_SPEEXDSP speexdsp QUIET)
    if(TT_SPEEXDSP_LIBRARIES)
message("speexdsp:   on")
      add_compile_definitions(__ENABLE_SPEEXDSP__)
      include_directories(${TT_SPEEXDSP_INCLUDE_DIRS})
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
    #windowsでOKになってる場合は、フラグをONにしてやる
message("theora:     on")
    add_compile_definitions(__ENABLE_THEORA__)
  else()
    pkg_check_modules(TT_THEORA theora QUIET)
    if(TT_THEORA_LIBRARIES)
message("theora:     on")
      add_compile_definitions(__ENABLE_THEORA__)
      include_directories(${TT_THEORA_INCLUDE_DIRS})
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
    #windowsでOKになってる場合は、フラグをONにしてやる
message("vorbis:     on")
    add_compile_definitions(__ENABLE_VORBIS_DECODE__ __ENABLE_VORBIS_ENCODE__)
  else()
    pkg_check_modules(TT_VORBIS vorbis vorbisenc QUIET)
    if(TT_VORBIS_LIBRARIES)
message("vorbis:     on")
      add_compile_definitions(__ENABLE_VORBIS_DECODE__ __ENABLE_VORBIS_ENCODE__)
      include_directories(${TT_VORBIS_INCLUDE_DIRS})
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
      #windowsでOKになってる場合は、フラグをONにしてやる
message("x264:       on")
      add_compile_definitions(__ENABLE_X264__)
    else()
      pkg_check_modules(TT_X264 x264 QUIET)
      if(TT_X264_LIBRARIES)
message("x264:       on")
        add_compile_definitions(__ENABLE_X264__)
        include_directories(${TT_X264_INCLUDE_DIRS})
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
      #windowsでOKになってる場合は、フラグをONにしてやる
message("x265:       on")
      add_compile_definitions(__ENABLE_X265__)
    else()
      pkg_check_modules(TT_X265 x265 QUIET)
      if(TT_X265_LIBRARIES)
message("x265:       on")
        add_compile_definitions(__ENABLE_X265__)
        include_directories(${TT_X265_INCLUDE_DIRS})
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

# __ENABLE_SOCKET__
# __ENABLE_APPLE__
# __ENABLE_GPL__
# __ENABLE_LGPL__
# __ENABLE_FILE__
# __ENABLE_SWSCALE__
# __ENABLE_AVCODEC__
# __ENABLE_SWRESAMPLE__
# __ENABLE_WIN32__
# __ENABLE_JPEG__
# __ENABLE_LIBPNG__
# __ENABLE_LIBYUV__
# __ENABLE_MP3LAME__
# __ENABLE_MP3LAME_DECODE__
# __ENABLE_FAAC_ENCODE__
# __ENABLE_OPENAL__
# __ENABLE_OPENCV__
# __ENABLE_OPENH264__
# __ENABLE_OPUS__
# __ENABLE_SOUNDTOUCH__
# __ENABLE_SPEEX__
# __ENABLE_SPEEXDSP__
# __ENABLE_THEORA__
# __ENABLE_VORBIS_DECODE__
# __ENABLE_VORBIS_ENCODE__
# __ENABLE_X264__
# __ENABLE_X265__

# __ENABLE_WEBRTC__

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
  ttLibC/util/mmAudioLoopbackUtil.cpp
  ttLibC/util/stlListUtil.cpp
  ttLibC/util/msGlobalUtil.cpp
  ttLibC/util/opencvUtil.cpp
  ttLibC/util/stlMapUtil.cpp
  ttLibC/encoder/msAacEncoder.cpp
  ttLibC/encoder/msH264Encoder.cpp
  ttLibC/encoder/openh264Encoder.cpp
  ttLibC/decoder/openh264Decoder.cpp
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
else()
#  set_property(TARGET ttLibC
#    PROPERTY PUBLIC_HEADER
#    "ttLibC/container/container.h"
#    "ttLibC/ttLibC.h")

  set(prefix ${CMAKE_INSTALL_PREFIX})
  set(exec_prefix "\${prefix}")
  set(libdir "\${exec_prefix}/lib")
  set(includedir "\${prefix}/include")

  configure_file(ttLibC.pc.in ttLibC.pc @ONLY)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ttLibC.pc
    DESTINATION lib/pkgconfig)

  install(TARGETS ttLibC
    ARCHIVE DESTINATION lib)
#  install(FILES "ttLibC/container/container.h" DESTINATION include/ttLibC/container)
endif()