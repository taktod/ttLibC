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
#include "mediaStream.h"
#include "../../../allocator.h"
#include "../../../_log.h"
#include <webrtc/base/ssladapter.h>
#include <uuid/uuid.h>

using namespace ttLibC_webrtc;

void WebrtcConstraint::analyze(
		Constraints& target,
		ttLibC_net_client_WebrtcConstraintData *data) {
	// ttLibCの中身データを取り込む
	if(data->minAspectRatio != NULL) {
		add(target, MediaConstraintsInterface::kMinAspectRatio, data->minAspectRatio);
	}
	if(data->maxAspectRatio != NULL) {
		add(target, MediaConstraintsInterface::kMaxAspectRatio, data->maxAspectRatio);
	}
	if(data->maxWidth != NULL) {
		add(target, MediaConstraintsInterface::kMaxWidth, data->maxWidth);
	}
	if(data->minWidth != NULL) {
		add(target, MediaConstraintsInterface::kMinWidth, data->minWidth);
	}
	if(data->maxHeight != NULL) {
		add(target, MediaConstraintsInterface::kMaxHeight, data->maxHeight);
	}
	if(data->minHeight != NULL) {
		add(target, MediaConstraintsInterface::kMinHeight, data->minHeight);
	}
	if(data->maxFrameRate != NULL) {
		add(target, MediaConstraintsInterface::kMaxFrameRate, data->maxFrameRate);
	}
	if(data->minFrameRate != NULL) {
		add(target, MediaConstraintsInterface::kMinFrameRate, data->minFrameRate);
	}
	if(data->echoCancellation != NULL) {
		add(target, MediaConstraintsInterface::kEchoCancellation, data->echoCancellation);
	}
	if(data->googEchoCancellation != NULL) {
		add(target, MediaConstraintsInterface::kGoogEchoCancellation, data->googEchoCancellation);
	}
	if(data->googEchoCancellation2 != NULL) {
		add(target, MediaConstraintsInterface::kExtendedFilterEchoCancellation, data->googEchoCancellation2);
	}
	if(data->googDAEchoCancellation != NULL) {
		add(target, MediaConstraintsInterface::kDAEchoCancellation, data->googDAEchoCancellation);
	}
	if(data->googAutoGainControl != NULL) {
		add(target, MediaConstraintsInterface::kAutoGainControl, data->googAutoGainControl);
	}
	if(data->googAutoGainControl2 != NULL) {
		add(target, MediaConstraintsInterface::kExperimentalAutoGainControl, data->googAutoGainControl2);
	}
	if(data->googNoiseSuppression != NULL) {
		add(target, MediaConstraintsInterface::kNoiseSuppression, data->googNoiseSuppression);
	}
	if(data->googNoiseSuppression2 != NULL) {
		add(target, MediaConstraintsInterface::kExperimentalNoiseSuppression, data->googNoiseSuppression2);
	}

	// この２つはnode-webrtcに含まれるライブラリでは、存在してなかったやつ。
	if(data->intelligibilityEnhancer != NULL) {
		add(target, MediaConstraintsInterface::kIntelligibilityEnhancer, data->intelligibilityEnhancer);
	}
	if(data->levelControl != NULL) {
		add(target, MediaConstraintsInterface::kLevelControl, data->levelControl);
	}

	if(data->googHighpassFilter != NULL) {
		add(target, MediaConstraintsInterface::kHighpassFilter, data->googHighpassFilter);
	}
	if(data->googTypingNoiseDetection != NULL) {
		add(target, MediaConstraintsInterface::kTypingNoiseDetection, data->googTypingNoiseDetection);
	}
	if(data->googAudioMirroring != NULL) {
		add(target, MediaConstraintsInterface::kAudioMirroring, data->googAudioMirroring);
	}
	if(data->googNoiseReduction != NULL) {
		add(target, MediaConstraintsInterface::kNoiseReduction, data->googNoiseReduction);
	}
	if(data->offerToReceiveVideo != NULL) {
		add(target, MediaConstraintsInterface::kOfferToReceiveVideo, data->offerToReceiveVideo);
	}
	if(data->offerToReceiveAudio != NULL) {
		add(target, MediaConstraintsInterface::kOfferToReceiveAudio, data->offerToReceiveAudio);
	}
	if(data->voiceActivityDetection != NULL) {
		add(target, MediaConstraintsInterface::kVoiceActivityDetection, data->voiceActivityDetection);
	}
	if(data->iceRestart != NULL) {
		add(target, MediaConstraintsInterface::kIceRestart, data->iceRestart);
	}
	if(data->googUseRtpMux != NULL) {
		add(target, MediaConstraintsInterface::kUseRtpMux, data->googUseRtpMux);
	}
	if(data->EnableDTLS_SRTP != NULL) {
		add(target, MediaConstraintsInterface::kEnableDtlsSrtp, data->EnableDTLS_SRTP);
	}
	if(data->EnableRTPDataChannels != NULL) {
		add(target, MediaConstraintsInterface::kEnableRtpDataChannels, data->EnableRTPDataChannels);
	}
	if(data->googDscp != NULL) {
		add(target, MediaConstraintsInterface::kEnableDscp, data->googDscp);
	}
	if(data->googIPv6 != NULL) {
		add(target, MediaConstraintsInterface::kEnableIPv6, data->googIPv6);
	}
	if(data->googSuspendBelowMinBitrate != NULL) {
		add(target, MediaConstraintsInterface::kEnableVideoSuspendBelowMinBitrate, data->googSuspendBelowMinBitrate);
	}
	if(data->googCombinedAudioVideoBwe != NULL) {
		add(target, MediaConstraintsInterface::kCombinedAudioVideoBwe, data->googCombinedAudioVideoBwe);
	}
	if(data->googScreencastMinBitrate != NULL) {
		add(target, MediaConstraintsInterface::kScreencastMinBitrate, data->googScreencastMinBitrate);
	}
	if(data->googCpuOveruseDetection != NULL) {
		add(target, MediaConstraintsInterface::kCpuOveruseDetection, data->googCpuOveruseDetection);
	}
	if(data->googPayloadPadding != NULL) {
		add(target, MediaConstraintsInterface::kPayloadPadding, data->googPayloadPadding);
	}
}

void WebrtcConstraint::update(ttLibC_WebrtcConstraint *constraint) {
	analyze(_mandatory, &constraint->mandatory);
	analyze(_optional, &constraint->optional);
}

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
	// このタイミングでstreamを解放しなければならない。
	std::map<std::string, ttLibC_WebrtcMediaStream *>::iterator iter = _localStreams.begin();
	while(iter != _localStreams.end()) {
		ttLibC_WebrtcMediaStream_ *stream = (ttLibC_WebrtcMediaStream_ *)(*iter).second;
		ttLibC_free((void *)stream->inherit_super.name);
		delete stream->streamWrapper;
		ttLibC_free(stream);
	}
	_localStreams.clear();
	iter = _remoteStreams.begin();
	while(iter != _remoteStreams.end()) {
		ttLibC_WebrtcMediaStream_ *stream = (ttLibC_WebrtcMediaStream_ *)(*iter).second;
		ttLibC_free((void *)stream->inherit_super.name);
		delete stream->streamWrapper;
		ttLibC_free(stream);
	}
}

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> FactoryWrapper::refNativeFactory() {
	return _nativeFactory;
}

void FactoryWrapper::registMediaStream(
		ttLibC_WebrtcMediaStream *stream,
		bool isLocal) {
	if(isLocal) {
		_localStreams.insert(std::map<std::string, ttLibC_WebrtcMediaStream *>::value_type(std::string(stream->name), stream));
	}
	else {
		_remoteStreams.insert(std::map<std::string, ttLibC_WebrtcMediaStream *>::value_type(std::string(stream->name), stream));
	}
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

ttLibC_WebrtcMediaStream *ttLibC_WebrtcFactory_createLocalStream(
		ttLibC_WebrtcFactory *factory) {
	if(factory == NULL) {
		return NULL;
	}
	ttLibC_WebrtcFactory_ *factory_ = (ttLibC_WebrtcFactory_ *)factory;
	ttLibC_WebrtcMediaStream_ *stream = (ttLibC_WebrtcMediaStream_ *)ttLibC_malloc(sizeof(ttLibC_WebrtcMediaStream_));
	uuid_t value;
	uuid_generate(value);
	uuid_string_t name;
	uuid_unparse(value, name);
	char *name_buffer = (char *)ttLibC_malloc(strlen(name) + 1);
	strncpy(name_buffer, name, strlen(name));
	stream->inherit_super.name = name_buffer;
	stream->streamWrapper = new MediaStreamWrapper(factory_->factoryWrapper, name);
	factory_->factoryWrapper->registMediaStream((ttLibC_WebrtcMediaStream *)stream, true);
	return (ttLibC_WebrtcMediaStream *)stream;
}

ttLibC_WebrtcMediaStream *ttLibC_WebrtcFactory_createRemoteStream(
		FactoryWrapper *factoryWrapper,
		rtc::scoped_refptr<webrtc::MediaStreamInterface> nativeStream) {
	if(factoryWrapper == NULL) {
		return NULL;
	}
	if(nativeStream == nullptr) {
		return NULL;
	}
	ttLibC_WebrtcMediaStream_ *stream = (ttLibC_WebrtcMediaStream_ *)ttLibC_malloc(sizeof(ttLibC_WebrtcMediaStream_));
	const char *name = nativeStream->label().c_str();
	char *name_buffer = (char *)ttLibC_malloc(strlen(name) + 1);
	strncpy(name_buffer, name, strlen(name));
	stream->inherit_super.name = name_buffer;
	stream->streamWrapper = new MediaStreamWrapper(
			factoryWrapper,
			nativeStream);
	factoryWrapper->registMediaStream((ttLibC_WebrtcMediaStream *)stream, false);
	return (ttLibC_WebrtcMediaStream *)stream;
}

} /* extern "C" */

#endif

