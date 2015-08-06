AUTOMAKE_OPTIONS=subdir-objects

lib_LIBRARIES=libttLibC.a

libttLibC_a_SOURCES= \
	decoder/mp3lameDecoder.c \
	decoder/openh264Decoder.cpp \
	decoder/opusDecoder.c \
	decoder/speexDecoder.c \
	encoder/faacEncoder.c \
	encoder/mp3lameEncoder.c \
	encoder/openh264Encoder.cpp \
	encoder/opusEncoder.c \
	encoder/speexEncoder.c \
	frame/audio/aac.c \
	frame/audio/audio.c \
	frame/audio/mp3.c \
	frame/audio/opus.c \
	frame/audio/pcms16.c \
	frame/audio/speex.c \
	frame/video/bgr.c \
	frame/video/h264.c \
	frame/video/video.c \
	frame/video/vp8.c \
	frame/video/yuv420.c \
	frame/frame.c \
	resampler/imageResampler.c \
	resampler/speexdspResampler.c \
	util/beepUtil.c \
	util/bitUtil.c \
	util/hexUtil.c \
	util/httpUtil.c \
	util/openalUtil.c \
	util/opencvUtil.c \
	ttLibC.c

libttLibC_a_CFLAGS=$(OPENCV_CFLAGS) $(OPENAL_CFLAGS) $(MP3LAME_CFLAGS) $(FAAC_CFLAGS) $(OPENH264_CFLAGS) $(SPEEXDSP_CFLAGS) $(SPEEX_CFLAGS) $(OPUS_CFLAGS) $(LIBAVCODEC_CFLAGS)
libttLibC_a_CPPFLAGS=$(OPENCV_CFLAGS) $(OPENAL_CFLAGS) $(MP3LAME_CFLAGS) $(FAAC_CFLAGS) $(OPENH264_CFLAGS) $(SPEEXDSP_CFLAGS) $(SPEEX_CFLAGS) $(OPUS_CFLAGS) $(LIBAVCODEC_CFLAGS)
