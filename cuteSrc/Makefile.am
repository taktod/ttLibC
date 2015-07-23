if ENABLE_TEST

noinst_PROGRAMS=cuteTest

cuteTest_SOURCES= \
	cuteTest.cpp \
	containerTest.cpp \
	audioTest.cpp \
	videoTest.cpp \
	utilTest.cpp \
	encoderDecoderTest.cpp

cuteTest_CFLAGS=-I../ $(TEST_CFLAGS) $(OPENCV_CFLAGS) $(OPENAL_CFLAGS) $(MP3LAME_CFLAGS) $(FAAC_CFLAGS)
cuteTest_CPPFLAGS=-I../ $(TEST_CFLAGS) $(OPENCV_CFLAGS) $(OPENAL_CFLAGS) $(MP3LAME_CFLAGS) $(FAAC_CFLAGS)
cuteTest_LDADD=../ttLibC/libttLibC.a $(TEST_LIBS) $(OPENCV_LIBS) $(OPENAL_LIBS) $(MP3LAME_LIBS) $(FAAC_LIBS)

endif
