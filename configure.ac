dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(ttLibC, 1.0)

TTLIBC_DESCRIPTION="ttLibC library."

AC_CANONICAL_SYSTEM
AC_CANONICAL_HOST
AC_PROG_RANLIB
AM_INIT_AUTOMAKE()

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CC_STDC
AM_PROG_CC_C_O
AC_CHECK_TOOL(AR, ar, :)
AC_CHECK_TOOL(PKG_CONFIG, pkg-config, :)

# user input flag
lgpl_flag=no
gpl_flag=no
debug_flag=0
test_flag=no
file_flag=no
socket_flag=no
opencv_flag=no
openal_flag=no
jpeg_flag=no
openh264_flag=no
x264_flag=no
x265_flag=no
theora_flag=no
daala_flag=no
mp3lame_flag=no
faac_flag=no
speexdsp_flag=no
speex_flag=no
opus_flag=no
faad_flag=no
vorbis_flag=no
avcodec_flag=no
apple_flag=no
webrtc_flag=no
soundtouch_flag=no
fdkaac_flag=no
libyuv_flag=no
swscale_flag=no
swresample_flag=no
libpng_flag=no

# check result
cute_check=no
opencv_check=no
openal_check=no
jpeg_check=no
openh264_check=no
x264_check=no
x265_check=no
theora_check=no
daala_encode_check=no
daala_decode_check=no
mp3lame_encode_check=no
mp3lame_decode_check=no
faac_encode_check=no
speexdsp_check=no
speex_check=no
opus_check=no
faad_decode_check=no
vorbis_encode_check=no
vorbis_decode_check=no
avcodec_check=no
apple_check=no
webrtc_check=no
soundtouch_check=no
fdkaac_check=no
libyuv_check=no
swscale_check=no
swresample_check=no
libpng_check=no

AC_ARG_ENABLE(
	[lgpl],
	AS_HELP_STRING([--enable-lgpl], [enable lgpl code. license will be LGPLv3.]),
	[lgpl_flag=$enableval]
)
AC_ARG_ENABLE(
	[gpl],
	AS_HELP_STRING([--enable-gpl], [enable gpl code. license will be GPLv3.]),
	[gpl_flag=$enableval]
)
if test x$gpl_flag = "xyes"; then
	lgpl_flag=yes
fi

AC_ARG_ENABLE(
	[debug],
	AS_HELP_STRING([--enable-debug], [enable debug log output.]),
	[
		if test x$enableval = "xyes"; then
			debug_flag=1
		fi
	]
)
AC_SUBST(debug_flag)

AC_ARG_ENABLE(
	[test],
	AS_HELP_STRING([--enable-test], [enable test code. require cute_lib in /usr/local/include, license will be gpl, and debug flag is on.]),
	[
		test_flag=$enableval
		if test x$test_flag="xyes"; then
			debug_flag=1
			gpl_flag=yes
			lgpl_flag=yes
		fi
	]
)

if test x$test_flag = "xyes"; then
	# check cute library
	if test -f /usr/local/include/cute_lib/cute.h; then
		cute_check=yes
	else
		cute_check=no
	fi
fi

# library flag
AC_ARG_ENABLE(
	[all],
	AS_HELP_STRING([--enable-all], [enable all extra library support.]),
	[
		file_flag=$enableval
		socket_flag=$enableval
		opencv_flag=$enableval
		openal_flag=$enableval
		mp3lame_flag=$enableval
		faac_flag=$enableval
		openh264_flag=$enableval
		speexdsp_flag=$enableval
		speex_flag=$enableval
		opus_flag=$enableval
		avcodec_flag=$enableval
		faad_flag=$enableval
		jpeg_flag=$enableval
		apple_flag=$enableval
		x264_flag=$enableval
		x265_flag=$enableval
		theora_flag=$enableval
		daala_flag=$enableval
		vorbis_flag=$enableval
#		webrtc_flag=$enableval
		soundtouch_flag=$enableval
		fdkaac_flag=$enableval
		libyuv_flag=$enableval
		swscale_flag=$enableval
		swresample_flag=$enableval
		libpng_flag=$enableval
	]
)

AC_ARG_ENABLE(
	[opencv],
	AS_HELP_STRING([--enable-opencv/--disable-opencv], [enable/disable opencv code.]),
	[opencv_flag=$enableval]
)

AC_ARG_ENABLE(
	[openal],
	AS_HELP_STRING([--enable-openal/--disable-openal], [enable/disable openal code.]),
	[openal_flag=$enableval]
)

AC_ARG_ENABLE(
	[mp3lame],
	AS_HELP_STRING([--enable-mp3lame/--disable-mp3lame], [enable/disable mp3lame code.]),
	[mp3lame_flag=$enableval]
)

AC_ARG_ENABLE(
	[faac],
	AS_HELP_STRING([--enable-faac/--disable-faac], [enable/disable faac code.]),
	[faac_flag=$enableval]
)

AC_ARG_ENABLE(
	[jpeg],
	AS_HELP_STRING([--enable-jpeg/--disable-jpeg], [enable/disable jpeg code.]),
	[jpeg_flag=$enableval]
)

AC_ARG_ENABLE(
	[openh264],
	AS_HELP_STRING([--enable-openh264/--disable-openh264], [enable/disable openh264 code.]),
	[openh264_flag=$enableval]
)

AC_ARG_ENABLE(
	[speexdsp],
	AS_HELP_STRING([--enable-speexdsp/--disable-speexdsp], [enable/disable speexdsp code.]),
	[speexdsp_flag=$enableval]
)

AC_ARG_ENABLE(
	[speex],
	AS_HELP_STRING([--enable-speex/--disable-speex], [enable/disable speex code.]),
	[speex_flag=$enableval]
)

AC_ARG_ENABLE(
	[file],
	AS_HELP_STRING([--enable-file/--disable-file], [enable/disable FILE* code.]),
	[file_flag=$enableval]
)

AC_ARG_ENABLE(
	[socket],
	AS_HELP_STRING([--enable-socket/--disable-socket], [enable/disable network code.]),
	[socket_flag=$enableval]
)

AC_ARG_ENABLE(
	[opus],
	AS_HELP_STRING([--enable-opus/--disable-opus], [enable/disable opus code.]),
	[opus_flag=$enableval]
)

AC_ARG_ENABLE(
	[avcodec],
	AS_HELP_STRING([--enable-avcodec/--disable-avcodec], [enable/disable avcodec(ffmpeg or libav) code.]),
	[avcodec_flag=$enableval]
)

AC_ARG_ENABLE(
	[faad],
	AS_HELP_STRING([--enable-faad/--disable-faad], [enable/disable faad2 code.]),
	[faad_flag=$enableval]
)

AC_ARG_ENABLE(
	[apple],
	AS_HELP_STRING([--enable-apple/--disable-apple], [enable/disable OSX/IOS code.]),
	[apple_flag=$enableval]
)

AC_ARG_ENABLE(
	[x264],
	AS_HELP_STRING([--enable-x264/--disable-x264], [enable/diable x264 code.]),
	[x264_flag=$enableval]
)

AC_ARG_ENABLE(
	[x265],
	AS_HELP_STRING([--enable-x265/--disable-x265], [enable/diable x265 code.]),
	[x265_flag=$enableval]
)

AC_ARG_ENABLE(
	[theora],
	AS_HELP_STRING([--enable-theora/--disable-theora], [enable/disable theora code.]),
	[theora_flag=$enableval]
)

AC_ARG_ENABLE(
	[daala],
	AS_HELP_STRING([--enable-daala/--disable-daala], [enable/disable daala code.]),
	[daala_flag=$enableval]
)

AC_ARG_ENABLE(
	[vorbis],
	AS_HELP_STRING([--enable-vorbis/--disable-vorbis], [enable/disable vorbis code.]),
	[vorbis_flag=$enableval]
)

AC_ARG_ENABLE(
	[webrtc],
	AS_HELP_STRING([--enable-webrtc/--disable-webrtc], [enable/disable webrtc code.]),
	[webrtc_flag=$enableval]
)

AC_ARG_ENABLE(
	[soundtouch],
	AS_HELP_STRING([--enable-soundtouch/--disable-soundtouch], [enable/disable soundtouch code.]),
	[soundtouch_flag=$enableval]
)

AC_ARG_ENABLE(
	[fdkaac],
	AS_HELP_STRING([--enable-fdkaac/--disable-fdkaac], [enable/disable fdkaac code.]),
	[fdkaac_flag=$enableval]
)

AC_ARG_ENABLE(
	[libyuv],
	AS_HELP_STRING([--enable-libyuv/--disable-libyuv], [enable/disable libyuv code.]),
	[libyuv_flag=$enableval]
)

AC_ARG_ENABLE(
	[swscale],
	AS_HELP_STRING([--enable-swscale/--disable-swscale], [enable/disable swscale(ffmpeg or libav) code.]),
	[swscale_flag=$enableval]
)

AC_ARG_ENABLE(
	[swresample],
	AS_HELP_STRING([--enable-swresample/--disable-swresample], [enable/disable swresample(ffmpeg or libav) code.]),
	[swresample_flag=$enableval]
)

AC_ARG_ENABLE(
	[libpng],
	AS_HELP_STRING([--enable-libpng/--disable-libpng], [enable/disable libpng code.]),
	[libpng_flag=$enableval]
)

if test x$opencv_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[OPENCV],
		[opencv],
		[opencv_check=yes],
		[
			PKG_CHECK_MODULES(
				[OPENCV],
				[opencv4],
				[opencv_check=yes],
				[opencv_check=no]
			)
		]
	)
fi
if test x$mp3lame_flag = "xyes"; then
	CFLAGS_ORG=$CFLAGS
	CFLAGS="-I/usr/local/include/ $CFLAGS"
	CPPFLAGS_ORG=$CPPFLAGS
	CPPFLAGS="-I/usr/local/include/ $CPPFLAGS"
	AC_PREPROC_IFELSE(
		[AC_LANG_SOURCE([@%:@include <lame/lame.h>])],
		[
			MP3LAME_CFLAGS='-I/usr/local/include'
			MP3LAME_LIBS='-L/usr/local/lib -lmp3lame'
			AC_SUBST([MP3LAME_CFLAGS])
			AC_SUBST([MP3LAME_LIBS])
			AC_DEFINE([__ENABLE_MP3LAME__])
			if test x$gpl_flag = "xyes"; then
				mp3lame_decode_check=yes
			fi
			if test x$lgpl_flag = "xyes"; then
				mp3lame_encode_check=yes
			fi
			AC_MSG_CHECKING(for MP3LAME)
			AC_MSG_RESULT([yes])
		],
		[
			AC_MSG_CHECKING(for MP3LAME)
			AC_MSG_RESULT([no])
		]
	)
	CFLAGS=$CFLAGS_ORG
	CPPFLAGS=$CPPFLAGS_ORG
fi
if test x$faac_flag = "xyes"; then
	CFLAGS_ORG=$CFLAGS
	CFLAGS="-I/usr/local/include/ $CFLAGS"
	CPPFLAGS_ORG=$CPPFLAGS
	CPPFLAGS="-I/usr/local/include/ $CPPFLAGS"
	AC_MSG_CHECKING(for FAAC)
	AC_PREPROC_IFELSE(
		[AC_LANG_SOURCE([@%:@include <faac.h>])],
		[
			FAAC_CFLAGS='-I/usr/local/include'
			FAAC_LIBS='-L/usr/local/lib -lfaac'
			AC_SUBST([FAAC_CFLAGS])
			AC_SUBST([FAAC_LIBS])
			AC_DEFINE([__ENABLE_FAAC__])
			if test x$lgpl_flag = "xyes"; then
				faac_encode_check=yes
			fi
			AC_MSG_RESULT([yes])
		],
		[
			AC_MSG_RESULT([no])
		]
	)
	CFLAGS=$CFLAGS_ORG
	CPPFLAGS=$CPPFLAGS_ORG
fi

if test x$jpeg_flag = "xyes"; then
	CFLAGS_ORG=$CFLAGS
	CFLAGS="-I/usr/local/include/ $CFLAGS"
	CPPFLAGS_ORG=$CPPFLAGS
	CPPFLAGS="-I/usr/local/include/ $CPPFLAGS"
	AC_MSG_CHECKING(for JPEG)
	AC_PREPROC_IFELSE(
		[AC_LANG_SOURCE([@%:@include <jpeglib.h>])],
		[
			JPEG_CFLAGS='-I/usr/local/include'
			JPEG_LIBS='-L/usr/local/lib -ljpeg'
			AC_SUBST([JPEG_CFLAGS])
			AC_SUBST([JPEG_LIBS])
			jpeg_check=yes
			AC_MSG_RESULT([yes])
		],
		[
			AC_MSG_RESULT([no])
		]
	)
	CFLAGS=$CFLAGS_ORG
	CPPFLAGS=$CPPFLAGS_ORG
fi

if test x$openh264_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[OPENH264],
		[openh264],
		[openh264_check=yes],
		[openh264_check=no]
	)
fi
if test x$speexdsp_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[SPEEXDSP],
		[speexdsp],
		[speexdsp_check=yes],
		[speexdsp_check=no]
	)
fi
if test x$speex_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[SPEEX],
		[speex],
		[speex_check=yes],
		[speex_check=no]
	)
fi
if test x$opus_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[OPUS],
		[opus],
		[opus_check=yes],
		[opus_check=no]
	)
fi
if test x$avcodec_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[LIBAVCODEC],
		[libavcodec, libavutil],
		[avcodec_check=yes],
		[avcodec_check=no]
	)
fi
if test x$faad_flag = "xyes"; then
	CFLAGS_ORG=$CFLAGS
	CFLAGS="-I/usr/local/include/ $CFLAGS"
	CPPFLAGS_ORG=$CPPFLAGS
	CPPFLAGS="-I/usr/local/include/ $CPPFLAGS"
	AC_MSG_CHECKING(for FAAD)
	AC_PREPROC_IFELSE(
		[AC_LANG_SOURCE([@%:@include <faad.h>])],
		[
			FAAD_CFLAGS='-I/usr/local/include'
			FAAD_LIBS='-L/usr/local/lib -lfaad'
			AC_SUBST([FAAD_CFLAGS])
			AC_SUBST([FAAD_LIBS])
			AC_DEFINE([__ENABLE_FAAD__])
			if test x$gpl_flag = "xyes"; then
				faad_decode_check=yes
			fi
			AC_MSG_RESULT([yes])
		],
		[
			AC_MSG_RESULT([no])
		]
	)
	CFLAGS=$CFLAGS_ORG
	CPPFLAGS=$CPPFLAGS_ORG
fi
if test x$apple_flag = "xyes"; then
	# check "gcc --help" has framework or not.
	AC_MSG_CHECKING(for APPLE)
	CC_CHECK=`$CC --help | grep framework`
	if test "x$CC_CHECK" = "x"; then
		apple_check=no
	else 
		apple_check=yes
		APPLE_CFLAGS=''
		APPLE_LIBS='-framework AudioToolbox -framework AudioUnit -framework VideoToolbox -framework CoreMedia -framework CoreFoundation -framework CoreVideo -framework CoreAudio'
		AC_SUBST([APPLE_CFLAGS])
		AC_SUBST([APPLE_LIBS])
	fi
	AC_MSG_RESULT([$apple_check])
fi
if test x$openal_flag = "xyes"; then
	if test x$apple_check = "xyes"; then
		# use OpenAL.framework
		OPENAL_CFLAGS=''
		OPENAL_LIBS='-framework OpenAL'
		openal_check=yes
	else
		PKG_CHECK_MODULES(
			[OPENAL],
			[openal],
			[openal_check=yes],
			[openal_check=no]
		)
	fi
fi
if test x$x264_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[X264],
		[x264],
		[x264_check=yes],
		[x264_check=no]
	)
fi
if test x$x265_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[X265],
		[x265],
		[x265_check=yes],
		[x265_check=no]
	)
fi
if test x$theora_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[THEORA],
		[theora],
		[theora_check=yes],
		[theora_check=no]
	)
fi
if test x$daala_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[DAALAENC],
		[daalaenc],
		[daala_encode_check=yes],
		[daala_encode_check=no]
	)
	PKG_CHECK_MODULES(
		[DAALADEC],
		[daaladec],
		[daala_decode_check=yes],
		[daala_decode_check=no]
	)
fi
if test x$vorbis_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[VORBISENC],
		[vorbisenc],
		[vorbis_encode_check=yes],
		[vorbis_encode_check=no]
	)
	PKG_CHECK_MODULES(
		[VORBISDEC],
		[vorbis],
		[vorbis_decode_check=yes],
		[vorbis_decode_check=no]
	)
fi
if test x$webrtc_flag = "xyes"; then
	AC_MSG_CHECKING(for WEBRTC)
	if test x$socket_flag = "xyes"; then
		AC_ARG_VAR([WEBRTC_INCLUDE_PATH], [path for webrtc compiled directory.])
		AC_ARG_VAR([WEBRTC_PATH], [path for webrtc compiled directory.])
		if test x$WEBRTC_PATH = "x"; then
			AC_MSG_RESULT([no])
			webrtc_check=no
		else
			webrtc_check=yes
			if test x$WEBRTC_INCLUDE_PATH = "x"; then
				WEBRTC_INCLUDE_PATH=$WEBRTC_PATH/../../
			fi
			WEBRTC_CFLAGS="-I${WEBRTC_INCLUDE_PATH}"
			WEBRTC_CFLAGS+=' -pthread'
#			WEBRTC_CFLAGS+=' -fno-exceptions'
			WEBRTC_CFLAGS+=' -fno-strict-aliasing'
			WEBRTC_CFLAGS+=' -Wall'
			WEBRTC_CFLAGS+=' -Wno-unused-parameter'
			WEBRTC_CFLAGS+=' -Wno-missing-field-initializers'
			WEBRTC_CFLAGS+=' -Wextra'
			WEBRTC_CFLAGS+=' -Wno-unused-local-typedefs'
			WEBRTC_CFLAGS+=' -Wno-uninitialized'
			WEBRTC_CFLAGS+=' -Wno-unused-variable'
#			WEBRTC_CFLAGS+=' -Wno-unused-but-set-variable'
			WEBRTC_CFLAGS+=' -pipe'
			WEBRTC_CFLAGS+=' -fno-ident'
			WEBRTC_CFLAGS+=' -fdata-sections'
			WEBRTC_CFLAGS+=' -ffunction-sections'
			WEBRTC_CFLAGS+=' -fPIC'
			WEBRTC_CFLAGS+=' -fpermissive'
#			WEBRTC_CFLAGS+=' -std=c++11'
			
			WEBRTC_CFLAGS+=' -DWEBRTC_THREAD_RR'
			WEBRTC_CFLAGS+=' -DEXPAT_RELATIVE_PATH'
			WEBRTC_CFLAGS+=' -DGTEST_RELATIVE_PATH'
			WEBRTC_CFLAGS+=' -DJSONCPP_RELATIVE_PATH'
			WEBRTC_CFLAGS+=' -DWEBRTC_RELATIVE_PATH'
	
			WEBRTC_LIBS=`find ${WEBRTC_PATH} -name lib*.a -exec echo {} +`
			case "${host_os}" in
			linux*)
				WEBRTC_CFLAGS+=' -DWEBRTC_LINUX'
				WEBRTC_CFLAGS+=' -DWEBRTC_POSIX=1'
				WEBRTC_CFLAGS+=' -DNOMINMAX'
				;;
			cygwin*|mingw*)
				WEBRTC_CFLAGS+=' -DWEBRTC_WIN'
				WEBRTC_CFLAGS+=' -DNOGUI'
				WEBRTC_CFLAGS+=' -DNOMINMAX'
				;;
			darwin*)
#				WEBRTC_CFLAGS+=' -std=gnu++0x'
				WEBRTC_CFLAGS+=' -Wno-c++0x-extensions'
				WEBRTC_CFLAGS+=' -Wno-c++11-extensions'
				WEBRTC_CFLAGS+=' -stdlib=libc++'
				WEBRTC_CFLAGS+=' -DNOGUI'
				WEBRTC_CFLAGS+=' -DWEBRTC_MAC'
				WEBRTC_CFLAGS+=' -DWEBRTC_IOS'
				WEBRTC_CFLAGS+=' -DWEBRTC_POSIX=1'
				WEBRTC_LIBS+=' -stdlib=libc++'
				WEBRTC_LIBS+=' -framework Foundation'
				WEBRTC_LIBS+=' -framework Cocoa'
				WEBRTC_LIBS+=' -framework IOKit'
				WEBRTC_LIBS+=' -framework Security'
				WEBRTC_LIBS+=' -framework SystemConfiguration'
				WEBRTC_LIBS+=' -framework AVFoundation'
				WEBRTC_LIBS+=' -framework ApplicationServices'
				WEBRTC_LIBS+=' -framework CoreGraphics'
				WEBRTC_LIBS+=' -framework CoreServices'
				WEBRTC_LIBS+=' -framework AppKit'
				WEBRTC_LIBS+=' -framework OpenGL'
				WEBRTC_LIBS+=' -framework AudioToolbox'
				WEBRTC_LIBS+=' -framework CoreMedia'
				WEBRTC_LIBS+=' -framework CoreVideo'
				WEBRTC_LIBS+=' -framework CoreAudio'
				;;
			*)
				AC_MSG_ERROR(["OS $host_os is not supported"])
				;;
			esac
			AC_SUBST([WEBRTC_CFLAGS])
			AC_SUBST([WEBRTC_LIBS])
			AC_MSG_RESULT([yes])
		fi
	else
		AC_MSG_RESULT([no])
		webrtc_check=no
	fi
fi
if test x$soundtouch_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[SOUNDTOUCH],
		[soundtouch],
		[soundtouch_check=yes],
		[soundtouch_check=no]
	)
fi
if test x$fdkaac_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[FDKAAC],
		[fdk-aac],
		[fdkaac_check=yes],
		[fdkaac_check=no]
	)
fi
if test x$libyuv_flag = "xyes"; then
	AC_MSG_CHECKING(for LIBYUV)
	AC_ARG_VAR([LIBYUV_INCLUDE_PATH], [path for libyuv compiled directory.])
	AC_ARG_VAR([LIBYUV_PATH], [path for libyuv library.])
	if test x$LIBYUV_PATH = "x";then
		LIBYUV_PATH=/usr/local/lib
		LIBYUV_INCLUDE_PATH=/usr/local/include
	else
		if test x$LIBYUV_INCLUDE_PATH = "x"; then
			LIBYUV_INCLUDE_PATH=$LIBYUV_PATH/include/
		fi
	fi
	CFLAGS_ORG=$CFLAGS
	CFLAGS="-I$LIBYUV_INCLUDE_PATH $CFLAGS"
	CPPFLAGS_ORG=$CPPFLAGS
	CPPFLAGS="-I$LIBYUV_INCLUDE_PATH $CPPFLAGS"
	AC_PREPROC_IFELSE(
		[AC_LANG_SOURCE([@%:@include <libyuv.h>])],
		[
			LIBYUV_CFLAGS="-I${LIBYUV_INCLUDE_PATH}"
			LIBYUV_LIBS="-L${LIBYUV_PATH} -lyuv"
			AC_SUBST([LIBYUV_CFLAGS])
			AC_SUBST([LIBYUV_LIBS])
			libyuv_check=yes
			AC_MSG_RESULT([yes])
		],
		[
			AC_MSG_RESULT([no])
		]
	)
	CFLAGS=$CFLAGS_ORG
	CPPFLAGS=$CPPFLAGS_ORG
fi
if test x$swscale_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[LIBSWSCALE],
		[libswscale],
		[swscale_check=yes],
		[swscale_check=no]
	)
fi
if test x$swresample_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[LIBSWRESAMPLE],
		[libswresample],
		[swresample_check=yes],
		[swresample_check=no]
	)
fi
if test x$libpng_flag = "xyes"; then
	PKG_CHECK_MODULES(
		[LIBPNG],
		[libpng],
		[libpng_check=yes],
		[libpng_check=no]
	)
fi

AC_MSG_CHECKING(for license)
if test x$gpl_flag = "xyes"; then
	AC_DEFINE([__ENABLE_GPL__])
	AC_MSG_RESULT([GPLv3])
else
	if test x$lgpl_flag = "xyes"; then
		AC_DEFINE([__ENABLE_LGPL__])
		AC_MSG_RESULT([LGPLv3])
	else
		AC_MSG_RESULT([The 3 Cause BSD])
	fi
fi

# test & gpl & cute
if test x$test_flag$cute_check$gpl_flag = "xyesyesyes"; then
	TEST_CFLAGS='-I/usr/local/include/cute_lib'
#	TEST_LIBS='-ltcmalloc'
	AC_SUBST([TEST_CFLAGS])
	AC_SUBST([TEST_LIBS])
fi

AM_CONDITIONAL(ENABLE_TEST, test x$test_flag$cute_check$gpl_flag = "xyesyesyes")

if test x$opencv_flag$opencv_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_OPENCV__])
fi
AM_CONDITIONAL(ENABLE_OPENCV, test x$opencv_flag$opencv_check = "xyesyes")

if test x$openal_flag$gpl_flag$openal_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_OPENAL__])
fi
AM_CONDITIONAL(ENABLE_OPENAL, test x$openal_flag$gpl_flag$openal_check = "xyesyesyes")

if test x$mp3lame_flag$gpl_flag$mp3lame_decode_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_MP3LAME_DECODE__])
fi
AM_CONDITIONAL(ENABLE_MP3LAME_DECODE, test x$mp3lame_flag$gpl_flag$mp3lame_decode_check = "xyesyesyes")

if test x$mp3lame_flag$lgpl_flag$mp3lame_encode_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_MP3LAME_ENCODE__])
fi
AM_CONDITIONAL(ENABLE_MP3LAME_ENCODE, test x$mp3lame_flag$lgpl_flag$mp3lame_encode_check = "xyesyesyes")

if test x$faac_flag$lgpl_flag$faac_encode_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_FAAC_ENCODE__])
fi
AM_CONDITIONAL(ENABLE_FAAC_ENCODE, test x$faac_flag$lgpl_flag$faac_encode_check = "xyesyesyes")

if test x$jpeg_flag$jpeg_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_JPEG__])
fi
AM_CONDITIONAL(ENABLE_JPEG, test x$jpeg_flag$jpeg_check = "xyesyes")

if test x$openh264_flag$openh264_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_OPENH264__])
fi
AM_CONDITIONAL(ENABLE_OPENH264, test x$openh264_flag$openh264_check = "xyesyes")

if test x$speexdsp_flag$speexdsp_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_SPEEXDSP__])
fi
AM_CONDITIONAL(ENABLE_SPEEXDSP, test x$speexdsp_flag$speexdsp_check = "xyesyes")

if test x$speex_flag$speex_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_SPEEX__])
fi
AM_CONDITIONAL(ENABLE_SPEEX, test x$speex_flag$speex_check = "xyesyes")

if test x$file_flag = "xyes"; then
	AC_DEFINE([__ENABLE_FILE__])
fi
AM_CONDITIONAL(ENABLE_FILE, test x$file_flag = "xyes")

if test x$socket_flag = "xyes"; then
	AC_DEFINE([__ENABLE_SOCKET__])
fi
AM_CONDITIONAL(ENABLE_SOCKET, test x$socket_flag = "xyes")

if test x$opus_flag$opus_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_OPUS__])
fi
AM_CONDITIONAL(ENABLE_OPUS, test x$opus_flag$opus_check = "xyesyes")

if test x$avcodec_flag$lgpl_flag$avcodec_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_AVCODEC__])
fi
AM_CONDITIONAL(ENABLE_AVCODEC, test x$avcodec_flag$lgpl_flag$avcodec_check = "xyesyesyes")

if test x$faad_flag$gpl_flag$faad_decode_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_FAAD_DECODE__])
fi
AM_CONDITIONAL(ENABLE_FAAD_DECODE, test x$faad_flag$gpl_flag$faad_decode_check = "xyesyesyes")

if test x$apple_flag$apple_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_APPLE__])
fi
AM_CONDITIONAL(ENABLE_APPLE, test x$apple_flag$apple_check = "xyesyes")

if test x$x264_flag$gpl_flag$x264_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_X264__])
fi
AM_CONDITIONAL(ENABLE_X264, test x$x264_flag$gpl_flag$x264_check = "xyesyesyes")

if test x$x265_flag$gpl_flag$x265_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_X265__])
fi
AM_CONDITIONAL(ENABLE_X265, test x$x265_flag$gpl_flag$x265_check = "xyesyesyes")

if test x$theora_flag$theora_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_THEORA__])
fi
AM_CONDITIONAL(ENABLE_THEORA, test x$theora_flag$theora_check = "xyesyes")

if test x$daala_flag$daala_encode_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_DAALA_ENCODE__])
fi
AM_CONDITIONAL(ENABLE_DAALA_ENCODE, test x$daala_flag$daala_encode_check = "xyesyes")
if test x$daala_flag$daala_decode_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_DAALA_DECODE__])
fi
AM_CONDITIONAL(ENABLE_DAALA_DECODE, test x$daala_flag$daala_decode_check = "xyesyes")

if test x$vorbis_flag$vorbis_encode_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_VORBIS_ENCODE__])
fi
AM_CONDITIONAL(ENABLE_VORBIS_ENCODE, test x$vorbis_flag$vorbis_encode_check = "xyesyes")

if test x$vorbis_flag$vorbis_decode_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_VORBIS_DECODE__])
fi
AM_CONDITIONAL(ENABLE_VORBIS_DECODE, test x$vorbis_flag$vorbis_decode_check = "xyesyes")

if test x$webrtc_flag$webrtc_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_WEBRTC__])
fi
AM_CONDITIONAL(ENABLE_WEBRTC, test x$webrtc_flag$webrtc_check = "xyesyes")

if test x$soundtouch_flag$lgpl_flag$soundtouch_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_SOUNDTOUCH__])
fi
AM_CONDITIONAL(ENABLE_SOUNDTOUCH, test x$soundtouch_flag$lgpl_flag$soundtouch_check = "xyesyesyes")

if test x$fdkaac_flag$lgpl_flag$fdkaac_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_FDKAAC__])
fi
AM_CONDITIONAL(ENABLE_FDKAAC, test x$fdkaac_flag$lgpl_flag$fdkaac_check = "xyesyesyes")

if test x$libyuv_flag$libyuv_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_LIBYUV__])
fi
AM_CONDITIONAL(ENABLE_LIBYUV, test x$libyuv_flag$libyuv_check = "xyesyes")

if test x$swscale_flag$lgpl_flag$swscale_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_SWSCALE__])
fi
AM_CONDITIONAL(ENABLE_SWSCALE, test x$swscale_flag$lgpl_flag$swscale_check = "xyesyesyes")

if test x$swresample_flag$lgpl_flag$swresample_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_SWRESAMPLE__])
fi
AM_CONDITIONAL(ENABLE_SWRESAMPLE, test x$swresample_flag$lgpl_flag$swresample_check = "xyesyesyes")

if test x$libpng_flag$libpng_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_LIBPNG__])
fi
AM_CONDITIONAL(ENABLE_LIBPNG, test x$libpng_flag$libpng_check = "xyesyes")

AC_ARG_WITH(pkgconfigdir,
	AS_HELP_STRING([--with-pkgconfigdir], [Use the specified pkgconfig dir.(default is $libdir/pkgconfig)]),
	[pkgconfigdir=${withval}],
	[pkgconfigdir='${libdir}/pkgconfig'])

AC_SUBST([pkgconfigdir])

AC_SUBST(TTLIBC_DESCRIPTION)

AC_CONFIG_FILES(
	Makefile
	[ttLibC/log.h:ttLibC/log.h.in]
	ttLibC/Makefile
	cuteSrc/Makefile
	[ttLibC.pc:ttLibC.pc.in])

AC_OUTPUT

echo ""
echo "Result ..."

# about license
if test x$gpl_flag = "xyes"; then
echo "license: GPLv3"
	cp gpl-3.0.txt LICENSE
else
	if test x$lgpl_flag = "xyes"; then
echo "license: LGPLv3"
		cp lgpl-3.0.txt LICENSE
	else
echo "license: The 3 Cause BSD."
		cp 3causeBSD.txt LICENSE
	fi
fi
# about debug flag
if test x$debug_flag = "x1"; then
echo "debug:            on"
else
echo "debug:            off"
fi
# about test code.
if test x$test_flag = "xyes"; then
	if test x$gpl_flag = "xyes"; then
		if test x$cute_check = "xyes"; then
echo "test:             on"
		else
echo "test:             off(cause of no cute library.)"
		fi
	else
echo "test:             off(under gpl only.)"
	fi
else
echo "test:             off"
fi
# about library support
if test x$file_flag = "xyes"; then
echo "file feature:     on"
else
echo "file feature:     off"
fi
if test x$socket_flag = "xyes"; then
echo "socket feature:   on"
else
echo "socket feature:   off"
fi
if test x$opencv_flag = "xyes"; then
	if test x$opencv_check = "xyes"; then
echo "opencv:           on"
	else
echo "opencv:           off(opencv library is missing.)"
	fi
else
echo "opencv:           off"
fi
if test x$openal_flag = "xyes"; then
	if test x$gpl_flag = "xyes"; then
		if test x$openal_check = "xyes"; then
echo "openal:           on"
		else
echo "openal:           off(openal library is missing.)"
		fi
	else
echo "openal:           off(under gpl only.)"
	fi
else
echo "openal:           off"
fi
if test x$mp3lame_flag = "xyes"; then
	if test x$lgpl_flag = "xyes"; then
		if test x$mp3lame_encode_check = "xyes"; then
echo "mp3lame(encoder): on"
		else
echo "mp3lame(encoder): off(mp3lame library is missing.)"
		fi
	else
echo "mp3lame(encoder): off(under lgpl only.)"
	fi
else
echo "mp3lame(encoder): off"
fi
if test x$mp3lame_flag = "xyes"; then
	if test x$gpl_flag = "xyes"; then
		if test x$mp3lame_decode_check = "xyes"; then
echo "mp3lame(decoder): on"
		else
echo "mp3lame(decoder): off(mp3lame library is missing.)"
		fi
	else
echo "mp3lame(decoder): off(under gpl only.)"
	fi
else
echo "mp3lame(decoder): off"
fi
if test x$faac_flag = "xyes"; then
	if test x$lgpl_flag = "xyes"; then
		if test x$faac_encode_check = "xyes"; then
echo "faac(encoder):    on"
		else
echo "faac(encoder):    off(faac library is missing.)"
		fi
	else
echo "faac(encoder):    off(under lgpl only.)"
	fi
else
echo "faac(encoder):    off"
fi
if test x$faad_flag = "xyes"; then
	if test x$gpl_flag = "xyes"; then
		if test x$faad_decode_check = "xyes"; then
echo "faad(decoder):    on"
		else
echo "faad(decoder):    off(faad library is missing.)"
		fi
	else
echo "faad(decoder):    off(under gpl only.)"
	fi
else
echo "faad(decoder):    off"
fi
if test x$openh264_flag = "xyes"; then
	if test x$openh264_check = "xyes"; then
echo "openh264:         on"
	else
echo "openh264:         off(openh264 library is missing.)"
	fi
else
echo "openh264:         off"
fi
if test x$x264_flag = "xyes"; then
	if test x$gpl_flag = "xyes"; then
		if test x$x264_check = "xyes"; then
echo "x264:             on"
		else
echo "x264:             off(x264 library is missing.)"
		fi
	else
echo "x264:             off(under gpl only.)"
	fi
else
echo "x264:             off"
fi
if test x$x265_flag = "xyes"; then
	if test x$gpl_flag = "xyes"; then
		if test x$x265_check = "xyes"; then
echo "x265:             on"
		else
echo "x265:             off(x265 library is missing.)"
		fi
	else
echo "x265:             off(under gpl only.)"
	fi
else
echo "x265:             off"
fi
if test x$theora_flag = "xyes"; then
	if test x$theora_check = "xyes"; then
echo "theora:           on"
	else
echo "theora:           off(theora library is missing.)"
	fi
else
echo "theora:           off"
fi
if test x$daala_flag = "xyes"; then
	if test x$daala_encode_check = "xyes"; then
echo "daala(encoder):   on"
	else
echo "daala(encoder):   off(daalaenc library is missing.)"
	fi
	if test x$daala_decode_check = "xyes"; then
echo "daala(decoder):   on"
	else
echo "daala(decoder):   off(daaladec library is missing.)"
	fi
else
echo "daala(encoder):   off"
echo "daala(decoder):   off"
fi
if test x$vorbis_flag = "xyes"; then
	if test x$vorbis_encode_check = "xyes"; then
echo "vorbis(encoder):  on"
	else
echo "vorbis(encoder):  off(vorbisenc library is missing.)"
	fi
	if test x$vorbis_decode_check = "xyes"; then
echo "vorbis(decoder):  on"
	else
echo "vorbis(decoder):  off(vorbis library is missing.)"
	fi
else
echo "vorbis(encoder):  off"
echo "vorbis(decoder):  off"
fi
if test x$speexdsp_flag = "xyes"; then
	if test x$speexdsp_check = "xyes"; then
echo "speexdsp:         on"
	else
echo "speexdsp:         off(speexdsp library is missing.)"
	fi
else
echo "speexdsp:         off"
fi
if test x$speex_flag = "xyes"; then
	if test x$speex_check = "xyes"; then
echo "speex:            on"
	else
echo "speex:            off(speex library is missing.)"
	fi
else
echo "speex:            off"
fi
if test x$opus_flag = "xyes"; then
	if test x$opus_check = "xyes"; then
echo "opus:             on"
	else
echo "opus:             off(opus library is missing.)"
	fi
else
echo "opus:             off"
fi
if test x$avcodec_flag = "xyes"; then
	if test x$lgpl_flag = "xyes"; then
		if test x$avcodec_check = "xyes"; then
			if test x$gpl_flag = "xyes"; then
echo "avcodec:          on"
			else
echo "avcodec:          on(restricted by license. lgpl)"
			fi
		else 
echo "avcodec:          off(avcodec library is missing.)"
		fi
	else
echo "avcodec:          off(under lgpl or gpl only.)"
	fi
else
echo "avcodec:          off"
fi
if test x$jpeg_flag = "xyes"; then
	if test x$jpeg_check = "xyes"; then
echo "jpeg:             on"
	else
echo "jpeg:             off(libjpeg is missing)"
	fi
else
echo "jpeg:             off"
fi
if test x$apple_flag = "xyes"; then
	if test x$apple_check = "xyes"; then
echo "apple:            on"
	else
echo "apple:            off(no framework detection.)"
	fi
else
echo "apple:            off"
fi
if test x$webrtc_flag = "xyes"; then
	if test x$socket_flag = "xyes"; then
		if test x$webrtc_check = "xyes"; then
echo "webrtc:           on"
		else
echo "webrtc:           off(no compile data detection.)"
		fi
	else
echo "webrtc:           off(no socket code.)"
	fi
else
echo "webrtc:           off"
fi
if test x$soundtouch_flag = "xyes"; then
	if test x$lgpl_flag = "xyes"; then
		if test x$soundtouch_check = "xyes"; then
echo "soundtouch:       on"
		else
echo "soundtouch:       off(soundtouch library is missing.)"
		fi
	else
echo "soundtouch:       off(under lgpl only.)"
	fi
else
echo "soundtouch:       off"
fi
if test x$fdkaac_flag = "xyes"; then
	if test x$lgpl_flag = "xyes"; then
		if test x$fdkaac_check = "xyes"; then
echo "fdkaac:           on"
		else
echo "fdkaac:           off(fdkaac library is missing.)"
		fi
	else
echo "fdkaac:           off(under lgpl only.)"
	fi
else
echo "fdkaac:           off"
fi
if test x$libyuv_flag = "xyes"; then
	if test x$libyuv_check = "xyes"; then
echo "libyuv:           on"
	else
echo "libyuv:           off(libyuv library is missing.)"
	fi
else
echo "libyuv:           off"
fi
if test x$swscale_flag = "xyes"; then
	if test x$lgpl_flag = "xyes"; then
		if test x$swscale_check = "xyes"; then
echo "swscale:          on"
		else 
echo "swscale:          off(swscale library is missing.)"
		fi
	else
echo "swscale:          off(under lgpl or gpl only.)"
	fi
else
echo "swscale:          off"
fi
if test x$swresample_flag = "xyes"; then
	if test x$lgpl_flag = "xyes"; then
		if test x$swresample_check = "xyes"; then
echo "swresample:       on"
		else 
echo "swresample:       off(swresample library is missing.)"
		fi
	else
echo "swresample:       off(under lgpl or gpl only.)"
	fi
else
echo "swresample:       off"
fi
if test x$libpng_flag = "xyes"; then
	if test x$libpng_check = "xyes"; then
echo "libpng:           on"
	else
echo "libpng:           off(libpng library is missing.)"
	fi
else
echo "libpng:           off"
fi
