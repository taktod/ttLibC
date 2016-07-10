AUTOMAKE_OPTIONS=subdir-objects

lib_LIBRARIES=libttLibC.a

libttLibC_a_SOURCES= \
	frame/audio/aac.c \
	frame/audio/adpcmImaWav.c \
	frame/audio/audio.c \
	frame/audio/mp3.c \
	frame/audio/nellymoser.c \
	frame/audio/opus.c \
	frame/audio/pcmAlaw.c \
	frame/audio/pcmf32.c \
	frame/audio/pcmMulaw.c \
	frame/audio/pcms16.c \
	frame/audio/speex.c \
	frame/audio/vorbis.c \
	frame/video/bgr.c \
	frame/video/flv1.c \
	frame/video/h264.c \
	frame/video/h265.c \
	frame/video/jpeg.c \
	frame/video/theora.c \
	frame/video/video.c \
	frame/video/vp6.c \
	frame/video/vp8.c \
	frame/video/vp9.c \
	frame/video/wmv1.c \
	frame/video/wmv2.c \
	frame/video/yuv420.c \
	frame/frame.c \
	resampler/audioResampler.c \
	resampler/imageResampler.c \
	resampler/speexdspResampler.c \
	util/beepUtil.c \
	util/byteUtil.c \
	util/crc32Util.c \
	util/dynamicBufferUtil.c \
	util/forkUtil.c \
	util/hexUtil.c \
	util/ioUtil.c \
	util/openalUtil.c \
	util/opencvUtil.c \
	util/stlListUtil.cpp \
	util/stlMapUtil.cpp \
	allocator.c \
	ttLibC.c

libttLibC_a_CFLAGS=$(OPENCV_CFLAGS) $(OPENAL_CFLAGS) $(MP3LAME_CFLAGS) $(SPEEXDSP_CFLAGS)
libttLibC_a_CXXFLAGS=-std=c++0x $(OPENCV_CFLAGS) $(OPENAL_CFLAGS) $(MP3LAME_CFLAGS) $(SPEEXDSP_CFLAGS)
