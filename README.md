#ttLibC: taktod's Library in C

## Author

name: taktod

twitter: https://twitter.com/taktod

##Overview

ttLibC is C library. Many media conversion library is released under LGPL or GPL, like ffmpeg. However, I wanna make such library under 3-cause BSD license.

This library is under dual license among, 3-Cause BSD license, LGPLv3 license, and GPLv3 license. Almost of all code is under 3-Cause BSD. However, some code is others, which depends on using external libraries.

For instance, if you want mp3 encode with lame, you need to configure ttLibC with --enable-mp3lame and --enable-lgpl and produced library license become LGPLv3.

###Components

* ttLibC: library program.
  * container: media container
    * flv.h: flv read / write.
    * mkv.h: matroska read / write.
    * mp3.h: mp3 read / write.
    * mp4.h: mp4 read / write.(write for fragmented only.)
    * mpegts.h: mpegts read / write.
  * decoder: decode frames.
    * audioConverterDecoder.h: decode with apple audioConverter.
    * avcodecDecoder.h: decode with libavcodec(ffmpeg). LGPL or GPL.
    * jpegDecoder.h: decode libjpeg.
    * mp3lameDecoder.h: decode with mp3lame. GPL.
    * openh264Decoder.h: decode with openh264.
    * opusDecoder.h: decode with libopus.
    * pngDecoder.h: decode with libpng
    * speexDecoder.h: decode with libspeex.
    * theoraDecoder.h: decode with libtheora.
    * vorbisDecoder.h: decode with libvorbis.
    * vtDecompressSessionDecoder.h: decode with apple videoToolbox.
  * encoder: encode frames
    * audioConverterEncoder.h: encode with apple audioConverter.
    * faacEncoder.h: encode with libfaac. LGPL.
    * jpegEncoder.h: encode with libjpeg.
    * mp3lameEncoder.h: encode with mp3lame. LGPL.
    * msAacEncoder.h: encode with windows encoder.
    * msH264Encoder.h: encode with windows encoder.
    * openh264Encoder.h: encode with openh264.
    * opusEncoder.h: encode with libopus.
    * speexEncoder.h: encode with libspeex.
    * theoraEncoder.h: encode with libtheora.
    * vorbisEncoder.h: encode with libvorbis.
    * vtCompressSessionEncoder.h: encode with apple videoToolbox
    * x264Encoder.h: encode with x264. GPL only.
    * x265Encoder.h: encode with x265. GPL only.
  * frame: media frame
    * audio: audio frame object.
      * aac.h
      * adpcmImaWav.h
      * mp3.h
      * nellymoser.h
      * opus.h
      * pcmAlaw.h
      * pcmf32.h
      * pcmMulaw.h
      * pcms16.h
      * speex.h
      * vorbis.h
    * video: video frame object.
      * bgr.h
      * flv1.h
      * h264.h
      * h265.h
      * jpeg.h
      * png.h
      * theora.h
      * vp6.h
      * vp8.h
      * vp9.h
      * wmv1.h
      * wmv2.h
      * yuv420.h
  * resampler: resample data
    * audioResampler.h: conversion between pcms16 and pcmf32.
    * imageResampler.h: conversion between bgr and yuv420.
    * imageResizer.h: resize bgr or yuv image.
    * libyuvResampler.h: resize or rotate for yuv image.
    * soundtouchResampler.h: change pitch. GPL or LGPL.
    * speexdspResampler.h: resample audio sample rate with libspeexdsp.
    * swresamplerResampler.h: resample audio frame with swresample. GPL or LGPL
    * swscaleResampler.h: resampler image frame with swscale. GPL or LGPL
  * util: utility for misc.
    * amfUtil.h: support to handle amf0 message.
    * audioUnitUtil.h: play sound with apple audioUnit.
    * beepUtil.h: create beep sound.
    * byteUtil.h: helper to read bit data.
    * crc32Util.h: crc32 support.
    * dynamicBufferUtil.h: scalable byte buffer support.
    * flvFrameUtil.h: helper to handle flv related system.
    * forkUtil.h: helper for fork process.
    * hexUtil.h: helper to handle hex data.
    * mmAudioLoopbackUtil.h: helper to handle windows audio loopback.
    * msGlobalUtil.h: helper for windows global value.
    * openalUtil.h: audio play with openal.
    * opencvUtil.h: camera capture and bgr draw with opencv.

##<a name="how to use"></a>How to use.

### cmake

```
$ git clone git@github.com:taktod/ttLibC
$ cd ttLibC
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make install
```

### autotool

```
$ git clone git@github.com:taktod/ttLibC
$ cd ttLibC
$ autoreconf
$ ./configure --enable-all --enable-test
$ make
$ make install
```

for OSX.

cute test code will be good example to use.

1. get the cute
  http://cute-test.com/projects/cute/wiki/CUTE_standalone
  make /usr/local/include/cute_lib/

2. get boost
  boost is required for cute.
  $ brew install boost

3. compile.
  $ autoreconf
  $ ./configure --enable-all --enable-test
  $ make
  $ cuteSrc/cuteTest
  will run all test.

4. camera with opencv.
  $ brew tap homebrew/science
  $ brew install opencv
  $ ./configure --enable-all --enable-test
  $ make clean
  $ make

5. sound play with openal.
  $ brew install openal
  $ ./configure --enable-all --enable-test
  $ make clean
  $ make

##<a name="Using libraries"></a>Using libraries

#### include
* klib (khash) 
  https://github.com/attractivechaos/klib/

#### ref
* cute
  http://cute-test.com/
* opencv
  http://opencv.jp/
* openal
  http://kcat.strangesoft.net/openal.html
* mp3lame
  http://lame.sourceforge.net/
* faac faad
  http://www.audiocoding.com/index.html
* openh264
  http://www.openh264.org/
* speex speexdsp
  http://www.speex.org/
* opus
  https://www.opus-codec.org/
* avcodec
  https://www.ffmpeg.org/
