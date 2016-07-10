nobase_include_HEADERS= \
<<<<<<< HEAD
	ttLibC/container/container.h \
	ttLibC/container/flv.h \
	ttLibC/container/mkv.h \
	ttLibC/container/mp3.h \
	ttLibC/container/mp4.h \
	ttLibC/container/mpegts.h \
=======
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
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
	ttLibC/util/amfUtil.h \
	ttLibC/util/beepUtil.h \
	ttLibC/util/byteUtil.h \
	ttLibC/util/crc32Util.h \
	ttLibC/util/dynamicBufferUtil.h \
	ttLibC/util/hexUtil.h \
	ttLibC/util/ioUtil.h \
<<<<<<< HEAD
	ttLibC/util/linkedListUtil.h \
=======
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
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

<<<<<<< HEAD
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
	ttLibC/decoder/openh264Decoder.h \
	ttLibC/encoder/openh264Encoder.h
endif

=======
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
if ENABLE_SPEEXDSP
nobase_include_HEADERS += \
	ttLibC/resampler/speexdspResampler.h
endif

<<<<<<< HEAD
if ENABLE_SPEEX
nobase_include_HEADERS += \
	ttLibC/decoder/speexDecoder.h \
	ttLibC/encoder/speexEncoder.h
endif

if ENABLE_FILE
nobase_include_HEADERS += \
	ttLibC/util/httpUtil.h \
	ttLibC/util/forkUtil.h
endif

if ENABLE_SOCKET
nobase_include_HEADERS += \
	ttLibC/net/client/rtmp.h \
	ttLibC/net/client/websocket.h \
	ttLibC/net/net.h \
	ttLibC/net/tcp.h \
	ttLibC/net/tetty.h \
	ttLibC/net/udp.h
endif

if ENABLE_OPUS
nobase_include_HEADERS += \
	ttLibC/decoder/opusDecoder.h \
	ttLibC/encoder/opusEncoder.h
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

if ENABLE_X264
nobase_include_HEADERS += \
	ttLibC/encoder/x264Encoder.h
endif

if ENABLE_THEORA
nobase_include_HEADERS += \
	ttLibC/encoder/theoraEncoder.h \
	ttLibC/decoder/theoraDecoder.h
endif

if ENABLE_VORBIS_ENCODE
nobase_include_HEADERS += \
	ttLibC/encoder/vorbisEncoder.h
endif

if ENABLE_VORBIS_DECODE
nobase_include_HEADERS += \
	ttLibC/decoder/vorbisDecoder.h
endif

if ENABLE_APPLE
nobase_include_HEADERS += \
	ttLibC/decoder/audioConverterDecoder.h \
	ttLibC/decoder/vtDecompressSessionDecoder.h \
	ttLibC/decoder/vtDecompressSessionH264Decoder.h \
	ttLibC/encoder/audioConverterEncoder.h \
	ttLibC/encoder/vtCompressSessionEncoder.h \
	ttLibC/encoder/vtCompressSessionH264Encoder.h \
	ttLibC/util/audioUnitUtil.h
endif

=======
if ENABLE_FILE
nobase_include_HEADERS += \
	ttLibC/util/forkUtil.h
endif

>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
pkgconfigdir=@pkgconfigdir@
pkgconfig_DATA=ttLibC.pc
CLEANFILES=ttLibC.pc

SUBDIRS=ttLibC cuteSrc
