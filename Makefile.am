nobase_include_HEADERS= \
	ttLibC/frame/audio/aac.h \
	ttLibC/frame/audio/adpcmImaWav.h \
	ttLibC/frame/audio/audio.h \
	ttLibC/frame/audio/mp3.h \
	ttLibC/frame/audio/nellymoser.h \
	ttLibC/frame/audio/opus.h \
	ttLibC/frame/audio/pcmAlaw.h \
	ttLibC/frame/audio/pcmf32.h \
	ttLibC/frame/audio/pcmMulaw.h \
	ttLibC/frame/audio/pcms16.h \
	ttLibC/frame/audio/speex.h \
	ttLibC/frame/audio/vorbis.h \
	ttLibC/frame/video/bgr.h \
	ttLibC/frame/video/flv1.h \
	ttLibC/frame/video/h264.h \
	ttLibC/frame/video/h265.h \
	ttLibC/frame/video/jpeg.h \
	ttLibC/frame/video/theora.h \
	ttLibC/frame/video/video.h \
	ttLibC/frame/video/vp6.h \
	ttLibC/frame/video/vp8.h \
	ttLibC/frame/video/vp9.h \
	ttLibC/frame/video/wmv1.h \
	ttLibC/frame/video/wmv2.h \
	ttLibC/frame/video/yuv420.h \
	ttLibC/frame/frame.h \
	ttLibC/resampler/audioResampler.h \
	ttLibC/resampler/imageResampler.h \
	ttLibC/util/beepUtil.h \
	ttLibC/util/byteUtil.h \
	ttLibC/util/crc32Util.h \
	ttLibC/util/dynamicBufferUtil.h \
	ttLibC/util/hexUtil.h \
	ttLibC/util/ioUtil.h \
	ttLibC/util/stlListUtil.h \
	ttLibC/util/stlMapUtil.h \
	ttLibC/allocator.h \
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

if ENABLE_SPEEXDSP
nobase_include_HEADERS += \
	ttLibC/resampler/speexdspResampler.h
endif

if ENABLE_FILE
nobase_include_HEADERS += \
	ttLibC/util/httpUtil.h \
	ttLibC/util/forkUtil.h
endif

pkgconfigdir=@pkgconfigdir@
pkgconfig_DATA=ttLibC.pc
CLEANFILES=ttLibC.pc

SUBDIRS=ttLibC cuteSrc
