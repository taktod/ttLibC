/*
 * @file   factory.cpp
 * @brief  factory for webrtc objects.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/10/08
 */

#ifdef __ENABLE_WEBRTC__

#include "factory.h"
#include "peerConnection.h"
#include "../../../allocator.h"
#include "../../../log.h"
#include <webrtc/base/ssladapter.h>

using namespace ttLibC_webrtc;

FactoryWrapper::FactoryWrapper() {
	_signalingThread.Start();
	_workerThread.Start();
	_nativeFactory = webrtc::CreatePeerConnectionFactory(
			&_workerThread,
			&_signalingThread,
			nullptr,
			nullptr,
			nullptr);
}

FactoryWrapper::~FactoryWrapper() {
	_signalingThread.Stop();
	_workerThread.Stop();
}

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> FactoryWrapper::refNativeFactory() {
	return _nativeFactory;
}

extern "C" {

void ttLibC_WebrtcFactory_initSSL() {
	rtc::InitializeSSL();
}
void ttLibC_WebrtcFactory_cleanSSL() {
	rtc::CleanupSSL();
}

ttLibC_WebrtcFactory *ttLibC_WebrtcFactory_make() {
	ttLibC_WebrtcFactory_ *factory = (ttLibC_WebrtcFactory_ *)ttLibC_malloc(sizeof(ttLibC_WebrtcFactory_));
	if(factory == NULL) {
		return NULL;
	}
	// 専用のthreadをつくって利用する。
	// 必要があればあとで外部から設定可能にしてもいいかな。
	factory->factoryWrapper = new FactoryWrapper();
	return (ttLibC_WebrtcFactory *)factory;
}

void ttLibC_WebrtcFactory_close(ttLibC_WebrtcFactory **factory) {
	ttLibC_WebrtcFactory_ *target = (ttLibC_WebrtcFactory_ *)*factory;
	if(target == NULL) {
		return;
	}
	delete target->factoryWrapper;
	ttLibC_free(target);
	*factory = NULL;
}

ttLibC_WebrtcPeerConnection *ttLibC_WebrtcFactory_createPeerConnection(
		ttLibC_WebrtcFactory *factory,
		ttLibC_WebrtcConfig *config) {
	if(factory == NULL) {
		return NULL;
	}
	ttLibC_WebrtcFactory_ *factory_ = (ttLibC_WebrtcFactory_ *)factory;
	// peerConnectionをつくって応答する。
	// 実際のnativePeerConnectionはoffer作成時 or answer作成時に作ることにする。(生成時のconstraintをどうすべきか決まらないため、ここで必要な情報がたまっていない。)
	ttLibC_WebrtcPeerConnection_ *peerConnection = (ttLibC_WebrtcPeerConnection_ *)ttLibC_malloc(sizeof(ttLibC_WebrtcPeerConnection_));
	peerConnection->inherit_super.onAddStream                    = NULL;
	peerConnection->inherit_super.onDataChannel                  = NULL;
	peerConnection->inherit_super.onIceCandidate                 = NULL;
	peerConnection->inherit_super.onIceConnectionChange          = NULL;
	peerConnection->inherit_super.onIceConnectionReceivingChange = NULL;
	peerConnection->inherit_super.onIceGatheringChange           = NULL;
	peerConnection->inherit_super.onRemoveStream                 = NULL;
	peerConnection->inherit_super.onRenegotiationNeeded          = NULL;
	peerConnection->inherit_super.onSignalingChange              = NULL;
	peerConnection->peerConnectionWrapper = new PeerConnectionWrapper(factory_->factoryWrapper, config, (ttLibC_WebrtcPeerConnection *)peerConnection);
	return (ttLibC_WebrtcPeerConnection *)peerConnection;
}

} /* extern "C" */

#endif

