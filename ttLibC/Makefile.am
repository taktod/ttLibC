AUTOMAKE_OPTIONS=subdir-objects

lib_LIBRARIES=libttLibC.a

libttLibC_a_SOURCES= \
	container/flv/type/audioTag.c \
	container/flv/type/headerTag.c \
	container/flv/type/metaTag.c \
	container/flv/type/videoTag.c \
	container/flv/flvReader.c \
	container/flv/flvTag.c \
	container/flv/flvWriter.c \
	container/mpegts/type/pat.c \
	container/mpegts/type/pes.c \
	container/mpegts/type/pmt.c \
	container/mpegts/type/sdt.c \
	container/mpegts/mpegtsPacket.c \
	container/mpegts/mpegtsReader.c \
	container/mpegts/mpegtsWriter.c \
	container/container.c \
	container/misc.c \
	decoder/avcodecDecoder.c \
	decoder/mp3lameDecoder.c \
	decoder/openh264Decoder.cpp \
	decoder/opusDecoder.c \
	decoder/speexDecoder.c \
	encoder/avcodecEncoder.c \
	encoder/faacEncoder.c \
	encoder/mp3lameEncoder.c \
	encoder/openh264Encoder.cpp \
	encoder/opusEncoder.c \
	encoder/speexEncoder.c \
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
	util/bitUtil.c \
	util/crc32Util.c \
	util/hexUtil.c \
	util/httpUtil.c \
	util/ioUtil.c \
	util/openalUtil.c \
	util/opencvUtil.c \
	ttLibC.c

libttLibC_a_CFLAGS=$(OPENCV_CFLAGS) $(OPENAL_CFLAGS) $(MP3LAME_CFLAGS) $(FAAC_CFLAGS) $(OPENH264_CFLAGS) $(SPEEXDSP_CFLAGS) $(SPEEX_CFLAGS) $(OPUS_CFLAGS) $(LIBAVCODEC_CFLAGS) $(FAAD_CFLAGS)
libttLibC_a_CPPFLAGS=$(OPENCV_CFLAGS) $(OPENAL_CFLAGS) $(MP3LAME_CFLAGS) $(FAAC_CFLAGS) $(OPENH264_CFLAGS) $(SPEEXDSP_CFLAGS) $(SPEEX_CFLAGS) $(OPUS_CFLAGS) $(LIBAVCODEC_CFLAGS) $(FAAD_CFLAGS)
