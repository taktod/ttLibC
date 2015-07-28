dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(ttLibC, 1.0)

TTLIBC_DESCRIPTION="ttLibC library."

AC_CANONICAL_SYSTEM
AC_PROG_RANLIB
AM_INIT_AUTOMAKE()

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CC_STDC
AM_PROG_CC_C_O

# user input flg
lgpl_flg=no
gpl_flg=no
debug_flg=0
test_flg=no

opencv_flg=no
openal_flg=no
mp3lame_flg=no
faac_flg=no
openh264_flg=no
speexdsp_flg=no
speex_flg=no

# check result
cute_check=no 
opencv_check=no
openal_check=no
mp3lame_encode_check=no
mp3lame_decode_check=no
faac_encode_check=no
openh264_check=no
speexdsp_check=no
speex_check=no

AC_ARG_ENABLE(
	[lgpl],
	AS_HELP_STRING([--enable-lgpl], [enable lgpl code. license will be LGPLv3.]),
	[lgpl_flg=$enableval]
)
AC_ARG_ENABLE(
	[gpl],
	AS_HELP_STRING([--enable-gpl], [enable gpl code. license will be GPLv3.]),
	[gpl_flg=$enableval]
)
if test x$gpl_flg = "xyes"; then
	lgpl_flg=yes
fi

AC_ARG_ENABLE(
	[debug],
	AS_HELP_STRING([--enable-debug], [enable debug log output.]),
	[
		if test x$enableval = "xyes"; then
			debug_flg=1
		fi
	]
)
AC_SUBST(debug_flg)

AC_ARG_ENABLE(
	[test],
	AS_HELP_STRING([--enable-test], [enable test code. require cute_lib in /usr/local/include, license will be gpl, and debug flg is on.]),
	[
		test_flg=$enableval
		if test x$test_flg="xyes"; then
			debug_flg=1
			gpl_flg=yes
			lgpl_flg=yes
		fi
	]
)

if test x$test_flg = "xyes"; then
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
		opencv_flg=$enableval
		openal_flg=$enableval
		mp3lame_flg=$enableval
		faac_flg=$enableval
		openh264_flg=$enableval
		speexdsp_flg=$enableval
		speex_flg=$enableval
	]
)

AC_ARG_ENABLE(
	[opencv],
	AS_HELP_STRING([--enable-opencv/--disable-opencv], [enable/disable opencv code.]),
	[opencv_flg=$enableval]
)

AC_ARG_ENABLE(
	[openal],
	AS_HELP_STRING([--enable-openal/--disable-openal], [enable/disable openal code.]),
	[openal_flg=$enableval]
)

AC_ARG_ENABLE(
	[mp3lame],
	AS_HELP_STRING([--enable-mp3lame/--disable-mp3lame], [enable/disable mp3lame code.]),
	[mp3lame_flg=$enableval]
)

AC_ARG_ENABLE(
	[faac],
	AS_HELP_STRING([--enable-faac/--disable-faac], [enable/disable faac code.]),
	[faac_flg=$enableval]
)

AC_ARG_ENABLE(
	[openh264],
	AS_HELP_STRING([--enable-openh264/--disable-openh264], [enable/disable openh264 code.]),
	[openh264_flg=$enableval]
)

AC_ARG_ENABLE(
	[speexdsp],
	AS_HELP_STRING([--enable-speexdsp/--disable-speexdsp], [enable/disable speexdsp code.]),
	[speexdsp_flg=$enableval]
)

AC_ARG_ENABLE(
	[speex],
	AS_HELP_STRING([--enable-speex/--disable-speex], [enable/disable speex code.]),
	[speex_flg=$enableval]
)

if test x$opencv_flg = "xyes"; then
	PKG_CHECK_MODULES(
		[OPENCV],
		[opencv],
		[opencv_check=yes]
	)
fi
if test x$openal_flg$gpl_flg = "xyesyes"; then
	PKG_CHECK_MODULES(
		[OPENAL],
		[openal],
		[openal_check=yes]
	)
fi
if test x$mp3lame_flg = "xyes"; then
	if test -f /usr/local/include/lame/lame.h; then
		AC_CHECK_LIB(
			mp3lame,
			get_lame_version,
			[
				MP3LAME_CFLAGS='-I/usr/local/include'
				MP3LAME_LIBS='-L/usr/local/lib -lmp3lame'
				AC_SUBST([MP3LAME_CFLAGS])
				AC_SUBST([MP3LAME_LIBS])
				AC_DEFINE([__ENABLE_MP3LAME__])
				if test x$gpl_flg = "xyes"; then
					mp3lame_decode_check=yes
				fi
				if test x$lgpl_flg = "xyes"; then
					mp3lame_encode_check=yes
				fi
			]
		)
	fi
fi
if test x$faac_flg = "xyes"; then
	AC_CHECK_HEADER(
		faac.h,
		[
			AC_CHECK_LIB(
				faac,
				faacEncGetVersion,
				[
					FAAC_CFLAGS='-I/usr/local/include'
					FAAC_LIBS='-L/usr/local/lib -lfaac'
					AC_SUBST([FAAC_CFLAGS])
					AC_SUBST([FAAC_LIBS])
					AC_DEFINE([__ENABLE_FAAC__])
					if test x$lgpl_flg = "xyes"; then
						faac_encode_check=yes
					fi
				]
			)
		],
	)
fi
if test x$openh264_flg = "xyes"; then
	PKG_CHECK_MODULES(
		[OPENH264],
		[openh264],
		[openh264_check=yes]
	)

fi
if test x$speexdsp_flg = "xyes"; then
	PKG_CHECK_MODULES(
		[SPEEXDSP],
		[speexdsp],
		[speexdsp_check=yes]
	)
fi
if test x$speex_flg = "xyes";then
	PKG_CHECK_MODULES(
		[SPEEX],
		[speex],
		[speex_check=yes]
	)
fi

if test x$gpl_flg = "xyes"; then
	AC_DEFINE([__ENABLE_GPL__])
else
	if test x$lgpl_flg = "xyes"; then
		AC_DEFINE([__ENABLE_LGPL__])
	fi
fi

# test & gpl & cute
if test x$test_flg$cute_check$gpl_flg = "xyesyesyes"; then
	TEST_CFLAGS='-I/usr/local/include/cute_lib'
	TEST_LIBS=''
	AC_SUBST([TEST_CFLAGS])
	AC_SUBST([TEST_LIBS])
fi
AM_CONDITIONAL(ENABLE_TEST, test x$test_flg$cute_check$gpl_flg = "xyesyesyes")

if test x$opencv_flg$opencv_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_OPENCV__])
fi
AM_CONDITIONAL(ENABLE_OPENCV, test x$opencv_flg$opencv_check = "xyesyes")

if test x$opencv_flg$gpl_flg$openal_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_OPENAL__])
fi
AM_CONDITIONAL(ENABLE_OPENAL, test x$openal_flg$gpl_flg$openal_check = "xyesyesyes")

if test x$mp3lame_flg$gpl_flg$mp3lame_decode_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_MP3LAME_DECODE__])
fi
AM_CONDITIONAL(ENABLE_MP3LAME_DECODE, test x$mp3lame_flg$gpl_flg$mp3lame_decode_check = "xyesyesyes")

if test x$mp3lame_flg$lgpl_flg$mp3lame_encode_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_MP3LAME_ENCODE__])
fi
AM_CONDITIONAL(ENABLE_MP3LAME_ENCODE, test x$mp3lame_flg$lgpl_flg$mp3lame_encode_check = "xyesyesyes")

if test x$faac_flg$lgpl_flg$faac_encode_check = "xyesyesyes"; then
	AC_DEFINE([__ENABLE_FAAC_ENCODE__])
fi
AM_CONDITIONAL(ENABLE_FAAC_ENCODE, test x$faac_flg$lgpl_flg$faac_encode_check = "xyesyesyes")

if test x$openh264_flg$openh264_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_OPENH264__])
fi
AM_CONDITIONAL(ENABLE_OPENH264, test x$openh264_flg$openh264_check = "xyesyes")

if test x$speexdsp_flg$speexdsp_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_SPEEXDSP__])
fi
AM_CONDITIONAL(ENABLE_SPEEXDSP, test x$speexdsp_flg$speexdsp_check = "xyesyes")

if test x$speex_flg$speex_check = "xyesyes"; then
	AC_DEFINE([__ENABLE_SPEEX__])
fi
AM_CONDITIONAL(ENABLE_SPEEX, test x$speex_flg$speex_check = "xyesyes")

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
if test x$gpl_flg = "xyes"; then
	echo "license: GPLv3"
	cp gpl-3.0.txt LICENSE
else
	if test x$lgpl_flg = "xyes"; then
		echo "license: LGPLv3"
		cp lgpl-3.0.txt LICENSE
	else
		echo "license: The 3 Cause BSD."
		cp 3causeBSD.txt LICENSE
	fi
fi
# about debug flg
if test x$debug_flg = "x1"; then
	echo "debug: on"
else
	echo "debug: off"
fi
# about test code.
if test x$test_flg = "xyes"; then
	if test x$gpl_flg = "xyes"; then
		if test x$cute_check = "xyes"; then
			echo "test: on"
		else
			echo "test: off(cause of no cute library.)"
		fi
	else
		echo "test: off(under gpl only.)"
	fi
else
	echo "test: off"
fi
# about library support
if test x$opencv_flg = "xyes"; then
	if test x$opencv_check = "xyes"; then
		echo "opencv: on"
	else
		echo "opencv: off(opencv library is missing.)"
	fi
else
	echo "opencv: off"
fi
if test x$openal_flg = "xyes"; then
	if test x$gpl_flg = "xyes"; then
		if test x$openal_check = "xyes"; then
			echo "openal: on"
		else
			echo "openal: off(openal library is missing.)"
		fi
	else
		echo "openal: off(under gpl only.)"
	fi
else
	echo "openal: off"
fi
if test x$mp3lame_flg = "xyes"; then
	if test x$gpl_flg = "xyes"; then
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
if test x$mp3lame_flg = "xyes"; then
	if test x$lgpl_flg = "xyes"; then
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
if test x$faac_flg = "xyes"; then
	if test x$lgpl_flg = "xyes"; then
		if test x$faac_encode_check = "xyes"; then
			echo "faac(encoder): on"
		else
			echo "faac(encoder): off(faac library is missing.)"
		fi
	else
		echo "faac(encoder): off(under lgpl only.)"
	fi
else
	echo "faac(encode): off"
fi
if test x$openh264_flg = "xyes"; then
	if test x$openh264_check = "xyes"; then
		echo "openh264: on"
	else
		echo "openh264: off(openh264 library is missing.)"
	fi
else
	echo "openh264: off"
fi
if test x$speexdsp_flg = "xyes"; then
	if test x$speexdsp_check = "xyes"; then
		echo "speexdsp: on"
	else
		echo "speexdsp: off(speexdsp library is missing.)"
	fi
else
	echo "speexdsp: off"
fi
if test x$speex_flg = "xyes"; then
	if test x$speex_check = "xyes"; then
		echo "speex: on"
	else
		echo "speex: off(speex library is missing.)"
	fi
else
	echo "speex: off"
fi
