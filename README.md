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
    * mp3.h: mp3 read / write.
    * mpegts.h: mpegts read / write.
  * decoder: decode frames.
    * avcodecDecoder.h: decode frame with libavcodec(ffmpeg). LGPL or GPL.
    * mp3lameDecoder.h: decode frame with mp3lame. GPL.
    * openh264Decoder.h: decode frame with openh264.
    * opusDecoder.h: decode frame with libopus.
    * speexDecoder.h: decode frame with libspeex.
  * encoder: encode frames
    * avcodecEncoder.h: encode frame with libavcodec(ffmpeg). LGPL or GPL.
    * faacEncoder.h: encode frame with libfaac. LGPL.
    * mp3lameEncoder.h: encode frame with mp3lame. LGPL.
    * openh264Encoder.h: encode frame with openh264.
    * opusEncoder.h: encode frame with libopus.
    * speexEncoder.h: encode frame with libspeex.
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
      * theora.h
      * vp6.h
      * vp8.h
      * wmv1.h
      * wmv2.h
      * yuv420.h
  * resampler: resample data
    * audioResampler.h: conversion between pcms16 and pcmf32.
    * imageResampler.h: conversion between bgr and yuv420.
    * speexdspResampler.h: resample audio sample rate with libspeexdsp.
  * util: utility for misc.
    * amfUtil.h: support to handle amf0 message.
    * beepUtil.h: create beep sound.
    * bitUtil.h: helper to read bit data.
    * crc32Util.h: crc32 support.
    * hexUtil.h: helper to handle hex data.
    * httpUtil.h: http client.
    * openalUtil.h: audio play with openal.
    * opencvUtil.h: camera capture and bgr draw with opencv.
* cuteSrc: test code. GPLv3

##<a name="how to use"></a>How to use.

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
