/**
 * @file   netTest.cpp
 * @brief  net test code.
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2015/08/26
 */

#include <cute.h>

#ifdef __ENABLE_SOCKET__

#include <ttLibC/log.h>
#include <ttLibC/allocator.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ttLibC/util/hexUtil.h>
#include <ttLibC/util/amfUtil.h>
#include <ttLibC/util/stlListUtil.h>
#include <string.h>
#include <time.h>
#include <unistd.h> // require for calling close, and so on...

#include <ttLibC/net/tetty.h>
#include <ttLibC/net/tcp.h>
#include <ttLibC/net/udp.h>

#include <ttLibC/net/client/rtmp.h>

#ifdef __ENABLE_FILE__
#	include <ttLibC/util/forkUtil.h>
#endif

#ifdef __ENABLE_OPENCV__
#	include <ttLibC/util/opencvUtil.h>
#endif

#ifdef __ENABLE_APPLE__
#	include <ttLibC/util/audioUnitUtil.h>
#	include <ttLibC/encoder/audioConverterEncoder.h>
#	include <ttLibC/encoder/vtCompressSessionH264Encoder.h>
#	include <ttLibC/decoder/audioConverterDecoder.h>
#	include <ttLibC/decoder/vtDecompressSessionH264Decoder.h>
#endif

#include <ttLibC/resampler/imageResampler.h>

static tetty_errornum udpTettyServerTest_channelActive(ttLibC_TettyContext *ctx) {
	LOG_PRINT("channelActive");
	return 0;
}

static tetty_errornum udpTettyServerTest_channelInactive(ttLibC_TettyContext *ctx) {
	LOG_PRINT("channelInactive");
	return 0;
}

static tetty_errornum udpTettyServerTest_channelRead(ttLibC_TettyContext *ctx, void *data, size_t data_size) {
	LOG_PRINT("channelRead");
	ttLibC_DatagramPacket *datagramPacket = (ttLibC_DatagramPacket *)data;
	LOG_PRINT("address:%x port:%x", ctx->socket_info->addr.sin_addr.s_addr, ctx->socket_info->addr.sin_port);
	return 0;
}

static tetty_errornum udpTettyServerTest_bind(ttLibC_TettyContext *ctx) {
	LOG_PRINT("bind");
	return 0;
}

static tetty_errornum udpTettyServerTest_connect(ttLibC_TettyContext *ctx) {
	LOG_PRINT("connect");
	return 0;
}

static tetty_errornum udpTettyServerTest_disconnect(ttLibC_TettyContext *ctx) {
	LOG_PRINT("disconnect");
	return 0;
}

static tetty_errornum udpTettyServerTest_close(ttLibC_TettyContext *ctx) {
	LOG_PRINT("close");
	return 0;
}

static tetty_errornum udpTettyServerTest_write(ttLibC_TettyContext *ctx, void *data, size_t data_size) {
	LOG_PRINT("write");
	return 0;
}

static tetty_errornum udpTettyServerTest_flush(ttLibC_TettyContext *ctx) {
	LOG_PRINT("flush");
	return 0;
}

static void udpTettyServerTest_exceptionCaught(ttLibC_TettyContext *ctx, tetty_errornum error_num) {
	LOG_PRINT("exception Caught");
	return;
}

static void udpTettyServerTest() {
	LOG_PRINT("udpTettyServerTest");
	// write in tetty format for udp server.
	ttLibC_TettyBootstrap *bootstrap = ttLibC_TettyBootstrap_make();
	ttLibC_TettyBootstrap_channel(bootstrap, ChannelType_Udp); // udp
	// pipeline
	ttLibC_TettyChannelHandler channelHandler;
	memset(&channelHandler, 0, sizeof(ttLibC_TettyChannelHandler));
	channelHandler.channelActive = udpTettyServerTest_channelActive;
	channelHandler.channelInactive = udpTettyServerTest_channelInactive;
	channelHandler.channelRead = udpTettyServerTest_channelRead;
	channelHandler.bind = udpTettyServerTest_bind;
	channelHandler.connect = udpTettyServerTest_connect;
	channelHandler.disconnect = udpTettyServerTest_disconnect;
	channelHandler.close = udpTettyServerTest_close;
	channelHandler.write = udpTettyServerTest_write;
	channelHandler.flush = udpTettyServerTest_flush;
	ttLibC_TettyBootstrap_pipeline_addLast(bootstrap, &channelHandler);
	// bind
	ttLibC_TettyBootstrap_bind(bootstrap, 12345);
	for(int i = 0;i < 10;i ++) {
		ttLibC_TettyBootstrap_update(bootstrap, 1200000);
	}
	ttLibC_TettyBootstrap_close(&bootstrap);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void udpClientTest() {
	LOG_PRINT("udpClientTest");
	ttLibC_UdpSocketInfo *socket_info = ttLibC_UdpSocket_make(6543);
	ttLibC_UdpSocket_open(socket_info);
	uint8_t buf[6] = {0, 1, 2, 3, 4, 5};
	LOG_PRINT("try to make packet.");
	ttLibC_DatagramPacket *packet = ttLibC_DatagramPacket_makeWithTarget(buf, 6, "127.0.0.1", 12345);
	LOG_PRINT("write data to packet.");
	ttLibC_UdpSocket_write(socket_info, packet);
	ttLibC_DatagramPacket_close(&packet);
	ttLibC_UdpSocket_close(&socket_info);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void udpServerTest() {
	LOG_PRINT("udpServerTest");
	ttLibC_UdpSocketInfo *socket_info = ttLibC_UdpSocket_make(12345);
	if(socket_info == NULL) {
		ERR_PRINT("no socket");
		return;
	}
	if(!ttLibC_UdpSocket_open(socket_info)) {
		ERR_PRINT("failed to open socket.");
		return;
	}
	uint8_t buf[256];
	ttLibC_DatagramPacket *packet = ttLibC_DatagramPacket_make(
			buf, 256);
	ssize_t sz = ttLibC_UdpSocket_read(socket_info, packet);
	LOG_PRINT("read_size:%ld", sz);
	LOG_DUMP(buf, sz, true);
	ttLibC_DatagramPacket_close(&packet);
	ttLibC_UdpSocket_close(&socket_info);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#ifdef __ENABLE_FILE__
tetty_errornum tettyServerTest_channelRead(ttLibC_TettyContext *ctx, void *data, size_t data_size) {
	if(strncmp((const char *)data, "closeServer", 11) == 0) {
		LOG_PRINT("close server is called.");
		ctx->bootstrap->error_number = 1; // treat as error, quit server.
		return 0;
	}
	if(strncmp((const char *)data, "close", 5) == 0) {
		LOG_PRINT("close command is called.");
		ttLibC_TettyContext_close(ctx);
		return 0;
	}
	LOG_PRINT("got data:%s", (const char *)data);
	ttLibC_TettyBootstrap_channels_writeAndFlush(ctx->bootstrap, data, data_size);
	return 0;
}

tetty_errornum tettyClientTest_channelRead(ttLibC_TettyContext *ctx, void *data, size_t data_size) {
	LOG_PRINT("got data:%s", (const char *)data);
	return 0;
}

tetty_errornum tettyClientTest_channelActive(ttLibC_TettyContext *ctx) {
	ttLibC_TettyContext_channel_writeAndFlush(ctx, (void *)"hello", 5);
	return 0;
}
#endif

static void tettyClientTest() {
	LOG_PRINT("tettyClientTest");
#ifdef __ENABLE_FILE__
	ttLibC_TettyBootstrap *bootstrap = ttLibC_TettyBootstrap_make();
	ttLibC_TettyBootstrap_channel(bootstrap, ChannelType_Tcp);
	ttLibC_TettyBootstrap_option(bootstrap, Option_SO_KEEPALIVE);
	ttLibC_TettyBootstrap_option(bootstrap, Option_TCP_NODELAY);

	ttLibC_TettyChannelHandler handler;
	memset(&handler, 0, sizeof(handler));
	handler.channelRead = tettyClientTest_channelRead;
	handler.channelActive = tettyClientTest_channelActive;
	ttLibC_TettyBootstrap_pipeline_addLast(bootstrap, &handler);

	ttLibC_TettyBootstrap_connect(bootstrap, "localhost", 12345);

	ttLibC_TettyFuture *future = ttLibC_TettyBootstrap_closeFuture(bootstrap);
	ttLibC_TettyPromise_await(future);

	ttLibC_TettyBootstrap_close(&bootstrap);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void tettyServerTest() {
	LOG_PRINT("tettyServerTest");
#ifdef __ENABLE_FILE__
	ttLibC_TettyBootstrap *bootstrap = ttLibC_TettyBootstrap_make();
	ttLibC_TettyBootstrap_channel(bootstrap, ChannelType_Tcp);
	ttLibC_TettyBootstrap_option(bootstrap, Option_SO_KEEPALIVE);
	ttLibC_TettyBootstrap_option(bootstrap, Option_TCP_NODELAY);
	ttLibC_TettyBootstrap_option(bootstrap, Option_SO_REUSEADDR);

	ttLibC_TettyChannelHandler handler;
	memset(&handler, 0, sizeof(handler));
	handler.channelRead = tettyServerTest_channelRead;
	ttLibC_TettyBootstrap_pipeline_addLast(bootstrap, &handler);

	ttLibC_TettyBootstrap_bind(bootstrap, 12345);

	ttLibC_TettyFuture *future = ttLibC_TettyBootstrap_closeFuture(bootstrap);
	ttLibC_TettyPromise_await(future);

	ttLibC_TettyBootstrap_close(&bootstrap);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void tcpServerTest() {
	LOG_PRINT("tcpServerTest");

#ifdef __ENABLE_FILE__
	int ok = 1;
	ttLibC_TcpServerInfo *server_info;
	ttLibC_TcpClientInfo *client_info;
	if(ok) {
		ttLibC_ForkUtil_setup();
		server_info = ttLibC_TcpServer_make(0x00000000UL, 8080);
		ok = ttLibC_TcpServer_open(server_info);
	}
	if(ok) {
		while((client_info = ttLibC_TcpServer_wait(server_info)) != NULL) {
			pid_t child_pid = ttLibC_ForkUtil_fork();
			if(child_pid == -1) {
				break;
			}
			else if(child_pid == 0) {
				ttLibC_TcpServer_close(&server_info);
				ttLibC_TcpClient_close(&client_info);
				ASSERT(ttLibC_Allocator_dump() == 0);
				exit(0);
			}
			else {
				ttLibC_TcpClient_close(&client_info);
			}
		}
	}
	ttLibC_TcpServer_close(&server_info);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_OPENCV__) && defined(__ENABLE_APPLE__)
typedef struct rtmpWatchTest_t {
	ttLibC_CvWindow *window;
//	ttLibC_AuPlayer *player;

	// decoder
//	ttLibC_AcDecoder *aac_decoder;
	ttLibC_VtH264Decoder *h264_decoder;

	ttLibC_RtmpConnection *conn;
	ttLibC_RtmpStream *stream;
	ttLibC_Bgr *bgr;
} rtmpWatchTest_t;

static bool rtmpWatchTest_h264DecodeCallback(void *ptr, ttLibC_Yuv420 *yuv) {
	rtmpWatchTest_t *testData = (rtmpWatchTest_t *)ptr;
	ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->bgr, BgrType_bgr, yuv);
	if(b == NULL) {
		return false;
	}
	testData->bgr = b;
	ttLibC_CvWindow_showBgr(testData->window, testData->bgr);
	return true;
}

static bool rtmpWatchTest_getFrameCallback(void *ptr, ttLibC_Frame *frame) {
	rtmpWatchTest_t *testData = (rtmpWatchTest_t *)ptr;
	switch(frame->type) {
	case frameType_h264:
		if(testData->h264_decoder == NULL) {
			return true;
		}
		if(((int32_t)frame->pts) < 0) {
			return true;
		}
		return ttLibC_VtH264Decoder_decode(
				testData->h264_decoder,
				(ttLibC_H264 *)frame,
				rtmpWatchTest_h264DecodeCallback,
				testData);
	case frameType_aac:
		break;
	case frameType_mp3:
		break;
	default:
		break;
	}
	return true;
}

static bool rtmpWatchTest_onStatusEventCallback(
		void *ptr,
		ttLibC_Amf0Object *obj) {
	rtmpWatchTest_t *testData = (rtmpWatchTest_t *)ptr;
	ttLibC_Amf0Object *code = ttLibC_Amf0_getElement(obj, "code");
	if(code != NULL && code->type == amf0Type_String) {
		LOG_PRINT("code:%s", (const char *)code->object);
		if(strcmp((const char *)code->object, "NetConnection.Connect.Success") == 0) {
			// make netStream and play
			testData->stream = ttLibC_RtmpStream_make(testData->conn);
			ttLibC_RtmpStream_setBufferLength(testData->stream, 1000); // bufferLength = 1;
			ttLibC_RtmpStream_addEventListener(testData->stream, rtmpWatchTest_onStatusEventCallback, testData);
			ttLibC_RtmpStream_addFrameListener(testData->stream, rtmpWatchTest_getFrameCallback, testData);
			testData->window = ttLibC_CvWindow_make("watch");
			ttLibC_RtmpStream_play(testData->stream, "test", true, true);
		}
	}
	return true;
}
#endif

static void rtmpWatchTest() {
	LOG_PRINT("rtmpWatchTest");
#if defined(__ENABLE_OPENCV__) && defined(__ENABLE_APPLE__)
	rtmpWatchTest_t testData;
//	testData.aac_decoder = NULL;
	testData.h264_decoder = ttLibC_VtH264Decoder_make();

//	testData.player = NULL;
	testData.window = NULL;
	testData.stream = NULL;
	testData.conn = ttLibC_RtmpConnection_make();

	testData.bgr = NULL;

	ttLibC_RtmpConnection_addEventListener(
			testData.conn,
			rtmpWatchTest_onStatusEventCallback,
			&testData);
	ttLibC_RtmpConnection_connect(
			testData.conn,
			"rtmp://localhost/live");
	while(true) {
		// check the socket status
		if(!ttLibC_RtmpConnection_update(testData.conn, 10000)) {
			break;
		}
		if(testData.window != NULL) {
			uint8_t keyChar = ttLibC_CvWindow_waitForKeyInput(1);
			if(keyChar ==Keychar_Esc) {
				break;
			}
		}
	}
//	ttLibC_AcDecoder_close(&testData.aac_decoder);
	ttLibC_VtH264Decoder_close(&testData.h264_decoder);

	ttLibC_RtmpStream_close(&testData.stream);
	ttLibC_RtmpConnection_close(&testData.conn);

//	ttLibC_AuPlayer_close(&testData.player);
	ttLibC_CvWindow_close(&testData.window);

	ttLibC_Bgr_close(&testData.bgr);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_OPENCV__) && defined(__ENABLE_APPLE__)
typedef struct rtmpPublishTest_t {
	// setting
	uint32_t sample_rate;
	uint32_t channel_num;
	uint32_t width;
	uint32_t height;

	// capture
	ttLibC_CvCapture *capture;
	ttLibC_CvWindow *window;
	ttLibC_AuRecorder *recorder;
	ttLibC_StlList *frame_list;
	ttLibC_StlList *used_frame_list;

	// encoder
	ttLibC_AcEncoder *aac_encoder;
	ttLibC_VtH264Encoder *h264_encoder;

	// rtmp
	ttLibC_RtmpConnection *conn;
	ttLibC_RtmpStream *stream;
} rtmpPublishTest_t;

static bool rtmpPublishTest_aacEncodeCallback(void *ptr, ttLibC_Audio *aac) {
	rtmpPublishTest_t *testData = (rtmpPublishTest_t *)ptr;
	ttLibC_RtmpStream_addFrame(testData->stream, (ttLibC_Frame *)aac);
	return true;
}

static bool rtmpPublishTest_h264EncodeCallback(void *ptr, ttLibC_H264 *h264) {
	rtmpPublishTest_t *testData = (rtmpPublishTest_t *)ptr;
	ttLibC_RtmpStream_addFrame(testData->stream, (ttLibC_Frame *)h264);
	return true;
}

static bool rtmpPublishTest_makePcmCallback(void *ptr, ttLibC_Audio *audio) {
	rtmpPublishTest_t *testData = (rtmpPublishTest_t *)ptr;
	if(audio->inherit_super.type != frameType_pcmS16) {
		// work with only pcms16
		return false;
	}
	ttLibC_Frame *prev_frame = NULL;
	if(testData->used_frame_list->size > 3) {
		prev_frame = (ttLibC_Frame *)ttLibC_StlList_refFirst(testData->used_frame_list);
		if(prev_frame != NULL) {
			ttLibC_StlList_remove(testData->used_frame_list, prev_frame);
		}
	}
	ttLibC_Frame *cloned_frame = ttLibC_Frame_clone(
			prev_frame,
			(ttLibC_Frame *)audio);
	if(cloned_frame == NULL) {
		return false;
	}
	ttLibC_StlList_addLast(testData->frame_list, cloned_frame);
	return true;
}

static bool rtmpPublishTest_onStatusEventCallback(
		void *ptr,
		ttLibC_Amf0Object *obj) {
	rtmpPublishTest_t *testData = (rtmpPublishTest_t *)ptr;
	ttLibC_Amf0Object *code = ttLibC_Amf0_getElement(obj, "code");
	if(code != NULL && code->type == amf0Type_String) {
		LOG_PRINT("code:%s", (const char *)code->object);
		if(strcmp((const char *)code->object, "NetConnection.Connect.Success") == 0) {
			// make netStream and publish
			testData->stream = ttLibC_RtmpStream_make(testData->conn);
			ttLibC_RtmpStream_addEventListener(testData->stream, rtmpPublishTest_onStatusEventCallback, testData);
			ttLibC_RtmpStream_publish(testData->stream, "test");
			return true;
		}
		if(strcmp((const char *)code->object, "NetStream.Publish.Start") == 0) {
			// now ready to make capture and recorder
			testData->capture = ttLibC_CvCapture_make(0, testData->width, testData->height);
			testData->recorder = ttLibC_AuRecorder_make(testData->sample_rate, testData->channel_num, AuRecorderType_DefaultInput, 0);
			ttLibC_Bgr *bgr = ttLibC_CvCapture_queryFrame(testData->capture, NULL);
			ttLibC_Bgr_close(&bgr);
			ttLibC_AuRecorder_start(testData->recorder, rtmpPublishTest_makePcmCallback, testData);
			testData->window = ttLibC_CvWindow_make("original");
		}
		if(strcmp((const char *)code->object, "NetStream.Unpublish.Success") == 0) {
			// done
		}
	}
	return true;
}

static bool rtmpPublishTest_closeAllFrame(void *ptr, void *item) {
	if(item != NULL) {
		ttLibC_Frame_close((ttLibC_Frame **)&item);
	}
	return true;
}
#endif

static void rtmpPublishTest() {
	LOG_PRINT("rtmpPublishTest");
#if defined(__ENABLE_OPENCV__) && defined(__ENABLE_APPLE__)
	rtmpPublishTest_t testData;
	testData.sample_rate = 44100;
	testData.channel_num = 1;
	testData.width       = 320;
	testData.height      = 240;

	testData.capture         = NULL;
	testData.window          = NULL;
	testData.recorder        = NULL;
	testData.frame_list      = ttLibC_StlList_make();
	testData.used_frame_list = ttLibC_StlList_make();

	testData.aac_encoder  = ttLibC_AcEncoder_make(testData.sample_rate, testData.channel_num, 96000, frameType_aac);
	testData.h264_encoder = ttLibC_VtH264Encoder_make(testData.width, testData.height);

	testData.conn = ttLibC_RtmpConnection_make();
	testData.stream = NULL;

	ttLibC_Bgr    *bgr = NULL;
	ttLibC_Yuv420 *yuv = NULL;

	ttLibC_RtmpConnection_addEventListener(
			testData.conn,
			rtmpPublishTest_onStatusEventCallback,
			&testData);
	ttLibC_RtmpConnection_connect(
			testData.conn,
			"rtmp://localhost/live");
	while(true) {
		// check the socket status
		if(!ttLibC_RtmpConnection_update(testData.conn, 10000)) {
			break;
		}
		if(testData.window != NULL) {
			ttLibC_Bgr *b = ttLibC_CvCapture_queryFrame(testData.capture, bgr);
			if(b == NULL) {
				break;
			}
			bgr = b;
			ttLibC_CvWindow_showBgr(testData.window, bgr);
			ttLibC_Yuv420 *y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
			if(y == NULL) {
				break;
			}
			yuv = y;
			ttLibC_VtH264Encoder_encode(testData.h264_encoder, yuv, rtmpPublishTest_h264EncodeCallback, &testData);
			ttLibC_Frame *frame = NULL;
			while(testData.frame_list->size > 2 && (frame = (ttLibC_Frame *)ttLibC_StlList_refFirst(testData.frame_list)) != NULL) {
				ttLibC_StlList_remove(testData.frame_list, frame);
				// encode to aac
				ttLibC_AcEncoder_encode(testData.aac_encoder, (ttLibC_PcmS16 *)frame, rtmpPublishTest_aacEncodeCallback, &testData);
				ttLibC_StlList_addLast(testData.used_frame_list, frame);
			}
			uint8_t keyChar = ttLibC_CvWindow_waitForKeyInput(1);
			if(keyChar == Keychar_Esc) {
				break;
			}
		}
	}
	// 1st close stream, 2nd close connection.
	ttLibC_RtmpStream_close(&testData.stream);
	ttLibC_RtmpConnection_close(&testData.conn);

	ttLibC_AuRecorder_close(&testData.recorder);
	ttLibC_CvCapture_close(&testData.capture);
	ttLibC_CvWindow_close(&testData.window);
	ttLibC_StlList_forEach(testData.frame_list, rtmpPublishTest_closeAllFrame, NULL);
	ttLibC_StlList_close(&testData.frame_list);
	ttLibC_StlList_forEach(testData.used_frame_list, rtmpPublishTest_closeAllFrame, NULL);
	ttLibC_StlList_close(&testData.used_frame_list);

	ttLibC_AcEncoder_close(&testData.aac_encoder);
	ttLibC_VtH264Encoder_close(&testData.h264_encoder);

	ttLibC_Bgr_close(&bgr);
	ttLibC_Yuv420_close(&yuv);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void echoServerTest() {
	// just try to write echo server.
	LOG_PRINT("echo server test");
	int connected_socket, listening_socket;
	struct sockaddr_in sin;
	int len, ret;
	int sock_optval = 1;
	int port = 5080;
	listening_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(listening_socket == -1) {
		ERR_PRINT("socket error");
		return;
	}
	if(setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR,
			&sock_optval, sizeof(sock_optval)) == -1) {
		ERR_PRINT("set sock opt.");
		return;
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listening_socket, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		ERR_PRINT("bind");
		return;
	}
	ret = listen(listening_socket, SOMAXCONN);
	if(ret == -1) {
		ERR_PRINT("listen failed");
		return;
	}
	while(1) {
		struct hostent *peer_host;
		struct sockaddr_in peer_sin;
		socklen_t len = sizeof(peer_sin);
		connected_socket = accept(listening_socket, (struct sockaddr *)&peer_sin, &len);
		if(connected_socket == -1) {
			ERR_PRINT("accept failed");
			break;
		}
//		FILE *fp = fdopen(connected_socket, "r+");
		while(1) {
			int read_size;
			char buf[25];
			int write_size;
			char wbuf[256] = "hello";
//			read_size = fread(buf, 1, 10, fp);
			read_size = read(connected_socket, buf, 25);
			if(read_size == 0) {
				ERR_PRINT("size = 0, disconnected.");
				break;
			}
			write(connected_socket, (const void *)wbuf, strlen(wbuf));
			ttLibC_HexUtil_dump(buf, read_size, true);
		}
//		if(fp) {
//			fclose(fp);
//		}
		ret = close(connected_socket);
		if(ret == -1) {
			ERR_PRINT("clientclose failed");
			break;
		}
		break;
	}
	ret = close(listening_socket);
	if(ret == -1) {
		ERR_PRINT("close faileds");
	}
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#endif // end of __ENABLE_SOCKET__

/**
 * define all test for net package.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite netTests(cute::suite s) {
	s.clear();
#ifdef __ENABLE_SOCKET__
	s.push_back(CUTE(udpTettyServerTest));
	s.push_back(CUTE(udpClientTest));
	s.push_back(CUTE(udpServerTest));
	s.push_back(CUTE(tettyClientTest));
	s.push_back(CUTE(tettyServerTest));
	s.push_back(CUTE(tcpServerTest));
	s.push_back(CUTE(rtmpWatchTest));
	s.push_back(CUTE(rtmpPublishTest));
	s.push_back(CUTE(echoServerTest));
#endif
	return s;
}
