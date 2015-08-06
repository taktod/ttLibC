nobase_include_HEADERS= \
	ttLibC/frame/audio/aac.h \
	ttLibC/frame/audio/audio.h \
	ttLibC/frame/audio/mp3.h \
	ttLibC/frame/audio/opus.h \
	ttLibC/frame/audio/pcms16.h \
	ttLibC/frame/audio/speex.h \
	ttLibC/frame/video/bgr.h \
	ttLibC/frame/video/h264.h \
	ttLibC/frame/video/video.h \
	ttLibC/frame/video/vp8.h \
	ttLibC/frame/video/yuv420.h \
	ttLibC/frame/frame.h \
	ttLibC/resampler/imageResampler.h \
	ttLibC/util/beepUtil.h \
	ttLibC/util/bitUtil.h \
	ttLibC/util/hexUtil.h \
	ttLibC/log.h \
	ttLibC/ttLibC.h

if ENABLE_OPENAL
nobase_include_HEADERS += \
	ttLibC/util/openalUtil.h
endif

if ENABLE_OPENCV
nobase_include_HEADERS += \
	ttLibC/util/opencvUtil.h
endif

if ENABLE_MP3LAME_ENCODE
nobase_include_HEADERS += \
	ttLibC/encoder/mp3lameEncoder.h
endif

if ENABLE_MP3LAME_DECODE
nobase_include_HEADERS += \
	ttLibC/decoder/mp3lameDecoder.h
endif

if ENABLE_FAAC_ENCODE
nobase_include_HEADERS += \
	ttLibC/encoder/faacEncoder.h
endif

if ENABLE_OPENH264
nobase_include_HEADERS += \
	ttLibC/encoder/openh264Encoder.h \
	ttLibC/decoder/openh264Decoder.h
endif

if ENABLE_SPEEXDSP
nobase_include_HEADERS += \
	ttLibC/resampler/speexdspResampler.h
endif

if ENABLE_SPEEX
nobase_include_HEADERS += \
	ttLibC/encoder/speexEncoder.h \
	ttLibC/decoder/speexDecoder.h
endif

if ENABLE_FILE
nobase_include_HEADERS += \
	ttLibC/util/httpUtil.h
endif

if ENABLE_OPUS
nobase_include_HEADERS += \
	ttLibC/encoder/opusEncoder.h \
	ttLibC/decoder/opusDecoder.h
endif

pkgconfigdir=@pkgconfigdir@
pkgconfig_DATA=ttLibC.pc
CLEANFILES=ttLibC.pc

SUBDIRS=ttLibC cuteSrc
