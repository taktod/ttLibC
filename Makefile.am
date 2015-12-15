nobase_include_HEADERS= \
	ttLibC/container/container.h \
	ttLibC/container/flv.h \
	ttLibC/container/mp3.h \
	ttLibC/container/mpegts.h \
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
	ttLibC/net/server/tcp.h \
	ttLibC/resampler/audioResampler.h \
	ttLibC/resampler/imageResampler.h \
	ttLibC/util/amfUtil.h \
	ttLibC/util/beepUtil.h \
	ttLibC/util/byteUtil.h \
	ttLibC/util/crc32Util.h \
	ttLibC/util/dynamicBufferUtil.h \
	ttLibC/util/hexUtil.h \
	ttLibC/util/ioUtil.h \
	ttLibC/util/linkedListUtil.h \
	ttLibC/util/stlListUtil.h \
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
	ttLibC/util/httpUtil.h \
	ttLibC/util/forkUtil.h
endif

if ENABLE_OPUS
nobase_include_HEADERS += \
	ttLibC/encoder/opusEncoder.h \
	ttLibC/decoder/opusDecoder.h
endif

if ENABLE_AVCODEC
nobase_include_HEADERS += \
	ttLibC/encoder/avcodecEncoder.h \
	ttLibC/decoder/avcodecDecoder.h
endif

if ENABLE_JPEG
nobase_include_HEADERS += \
	ttLibC/encoder/jpegEncoder.h \
	ttLibC/decoder/jpegDecoder.h
endif

pkgconfigdir=@pkgconfigdir@
pkgconfig_DATA=ttLibC.pc
CLEANFILES=ttLibC.pc

SUBDIRS=ttLibC cuteSrc
