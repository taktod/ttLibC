nobase_include_HEADERS= \
	ttLibC/frame/audio/audio.h \
	ttLibC/frame/audio/pcms16.h \
	ttLibC/frame/audio/mp3.h \
	ttLibC/frame/video/bgr.h \
	ttLibC/frame/video/video.h \
	ttLibC/frame/video/yuv420.h \
	ttLibC/frame/frame.h \
	ttLibC/resampler/imageResampler.h \
	ttLibC/util/beepUtil.h \
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

if ENABLE_FAAC_ENCODE
nobase_include_HEADERS += \
	ttLibC/encoder/faacEncoder.h
endif

if ENABLE_OPENH264
nobase_include_HEADERS += \
	ttLibC/encoder/openh264Encoder.h \
	ttLibC/decoder/openh264Decoder.h
endif

pkgconfigdir=@pkgconfigdir@
pkgconfig_DATA=ttLibC.pc
CLEANFILES=ttLibC.pc

SUBDIRS=ttLibC cuteSrc
