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
#include <ttLibC/log.h>
#include <ttLibC/allocator.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ttLibC/util/hexUtil.h>
#include <ttLibC/util/amfUtil.h>
#include <string.h>
#include <ttLibC/net/client/rtmp.h>
#include <time.h>

typedef struct {
	bool work_flg;
	ttLibC_RtmpConnection *netConnection;
	ttLibC_RtmpStream *netStream;
	uint32_t work_id;
} rtmpTest_t;

// callback for rtmp frame downloads.(play only)
bool rtmpTest_frameReceiveCallback(void *ptr, ttLibC_Frame *frame) {
	rtmpTest_t *testData = (rtmpTest_t *)ptr;
	return true;
}

// callback for rtmp event
bool rtmpTest_onStatusEvent(void *ptr, ttLibC_Amf0Object *amf0_obj) {
	rtmpTest_t *testData = (rtmpTest_t *)ptr;
	switch(amf0_obj->type) {
	case amf0Type_Object:
	case amf0Type_Map:
		{
			ttLibC_Amf0Object *code = ttLibC_Amf0_getElement(amf0_obj, "code");
			LOG_PRINT("code:%s", (const char *)code->object);
			if(code != NULL && strcmp((const char *)code->object, "NetConnection.Connect.Success") == 0) {
				LOG_PRINT("connect success.");
				testData->work_flg = false;
				// make netStream
				testData->netStream = ttLibC_RtmpStream_make(testData->netConnection);
				if(testData->netStream != NULL) {
					// do publish
					testData->work_id = ttLibC_RtmpStream_publish(testData->netStream, "test");
//					testData->work_id = ttLibC_RtmpStream_play(testData->netStream, "test", rtmpTest_frameReceiveCallback, ptr);
				}
			}
		}
		break;
	default:
		return false;
	}
	return true;
}

static void rtmpTest() {
	rtmpTest_t testData;
	testData.work_flg = true;
	testData.netConnection = ttLibC_RtmpConnection_make();
	testData.netStream = NULL;
	ttLibC_RtmpConnection_connect(
			testData.netConnection,
			"rtmp://localhost/live",
			rtmpTest_onStatusEvent,
			&testData);
	while(ttLibC_RtmpConnection_read(testData.netConnection)) {
		if(!testData.work_flg) {
			break;
		}
//		ttLibC_RtmpStream_feed(testData.netStream, testData.work_id, frame);
	}
	ttLibC_RtmpStream_close(&testData.netStream);
	ttLibC_RtmpConnection_close(&testData.netConnection);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void echoServerTest2() {
	LOG_PRINT("echo server test2");
	// 複数接続対策してみる。
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

/**
 * define all test for net package.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite netTests(cute::suite s) {
	s.clear();
	s.push_back(CUTE(rtmpTest));
	s.push_back(CUTE(echoServerTest2));
	s.push_back(CUTE(echoServerTest));
	return s;
}
