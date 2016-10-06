/**
 * @file   webrtcTest.cpp
 * @brief  webrtc test code.
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2016/10/06
 */

#include <cute.h>
#include <ttLibC/log.h>

#ifdef __ENABLE_SOCKET__
#	include <ttLibC/net/client/websocket.h>
#endif

#ifdef __ENABLE_WEBRTC__
#	include <webrtc/base/scoped_ref_ptr.h>
#	include <webrtc/modules/audio_device/include/audio_device.h>
#endif

#if defined(__ENABLE_WEBRTC__) && defined(__ENABLE_SOCKET__)
#endif

static void peerConnectTest() {
#if defined(__ENABLE_WEBRTC__) && defined(__ENABLE_SOCKET__)
#endif
}

//#	include <webrtc/base/scoped_ref_ptr.h>
//#	include <webrtc/modules/audio_device/include/audio_device.h>
static void test() {
#ifdef __ENABLE_WEBRTC__
	LOG_PRINT("webrtc test");
	rtc::scoped_refptr<webrtc::AudioDeviceModule> audio = webrtc::AudioDeviceModule::Create(0, webrtc::AudioDeviceModule::kPlatformDefaultAudio);
	audio->Init();
	int num, ret;
	num = audio->RecordingDevices();
	for(int i = 0;i < num;++ i) {
		char name[webrtc::kAdmMaxDeviceNameSize];
		char guid[webrtc::kAdmMaxGuidSize];
		int ret = audio->RecordingDeviceName(i, name, guid);
		if(ret != -1) {
			LOG_PRINT("    %d %s %s", i, name, guid);
		}
	}
#endif
}

cute::suite webrtcTests(cute::suite s) {
	s.clear();
	s.push_back(CUTE(peerConnectTest));
	s.push_back(CUTE(test));
	return s;
}
