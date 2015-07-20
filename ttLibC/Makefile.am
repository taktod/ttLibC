AUTOMAKE_OPTIONS=subdir-objects

lib_LIBRARIES=libttLibC.a

libttLibC_a_SOURCES= \
	ttLibC.c \
	frame/frame.c \
	frame/audio/audio.c \
	frame/audio/pcms16.c \
	frame/video/video.c \
	frame/video/bgr.c \
	frame/video/yuv420.c \
	resampler/imageResampler.c \
	util/hexUtil.c \
	util/opencvUtil.c \
	util/openalUtil.c \
	util/beepUtil.c

libttLibC_a_CFLAGS=$(OPENCV_CFLAGS) $(OPENAL_CFLAGS) $(MP3LAME_CFLAGS)
libttLibC_a_CPPFLAGS=$(OPENCV_CFLAGS) $(OPENAL_CFLAGS) $(MP3LAME_CFLAGS)
