AUTOMAKE_OPTIONS=subdir-objects

lib_LIBRARIES=libttLibC.a

libttLibC_a_SOURCES= \
	encoder/faacEncoder.c \
	encoder/mp3lameEncoder.c \
	frame/audio/aac.c \
	frame/audio/audio.c \
	frame/audio/pcms16.c \
	frame/audio/mp3.c \
	frame/video/video.c \
	frame/video/bgr.c \
	frame/video/yuv420.c \
	frame/frame.c \
	resampler/imageResampler.c \
	util/hexUtil.c \
	util/opencvUtil.c \
	util/openalUtil.c \
	util/beepUtil.c \
	ttLibC.c

libttLibC_a_CFLAGS=$(OPENCV_CFLAGS) $(OPENAL_CFLAGS) $(MP3LAME_CFLAGS) $(FAAC_CFLAGS)
libttLibC_a_CPPFLAGS=$(OPENCV_CFLAGS) $(OPENAL_CFLAGS) $(MP3LAME_CFLAGS) $(FAAC_CFLAGS)
