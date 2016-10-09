/*
 * @file   peerConnection.cpp
 * @brief  support peerConnection.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/10/08
 */

#ifdef __ENABLE_WEBRTC__

#include "peerConnection.h"
#include "../../../allocator.h"
#include "factory.h"

#include <webrtc/api/mediastreaminterface.h>

using namespace webrtc;
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

void CreateSdpObserver::OnSuccess(SessionDescriptionInterface *sdp) {
	// sdpからttLibC_WebrtcSdpをつくって、funcを呼んでやる
	ttLibC_WebrtcSdp cSdp;
	std::string str;
	sdp->ToString(&str);

	cSdp.type = sdp->type().c_str();
	cSdp.value = str.c_str();

	ttLibC_WebrtcEvent event = {};
	event.target = _cPeerConnection;
	event.sdp = &cSdp;
	if(_func != NULL) {
		_func(&event);
	}
}

void CreateSdpObserver::OnFailure(const std::string& msg) {
	// とりあえずデータなしで送っとく。いまのところ未使用
	ttLibC_WebrtcEvent event = {};
	event.target = _cPeerConnection;
	if(_func != NULL) {
		_func(&event);
	}
}

PeerConnectionWrapper::PeerConnectionWrapper(
		FactoryWrapper *factoryWrapper,
		ttLibC_WebrtcConfig *config,
		ttLibC_WebrtcPeerConnection *cPeerConnection) {
	// ここでnativePeerConnectionを作ろうとすると、データが足りないので、必要なデータを保持しておいて、createOffer or createAnswerで作ることにする。
	_nativePeerConnection = nullptr;
	// factoryはそのままコピーしてもっとく。
	_nativePeerConnectionFactory = factoryWrapper->refNativeFactory();
	// configをコピーしておいとく。
	for(int i = 0;i < config->size;++ i) {
		PeerConnectionInterface::IceServer iceServer;
		if(config->iceServers[i].password != NULL) {
			iceServer.password = std::string(config->iceServers[i].password);
		}
		if(config->iceServers[i].uri != NULL) {
			iceServer.uri = std::string(config->iceServers[i].uri);
		}
		if(config->iceServers[i].username != NULL) {
			iceServer.username = std::string(config->iceServers[i].username);
		}
		_iceServers.push_back(iceServer);
	}
	_cPeerConnection = cPeerConnection; // callback処理で必要になる。
	_sdpCopy = NULL;
	// sdp設定で必要になるobserverを作っておく。
	_setSdpObserver = new rtc::RefCountedObject<SetSdpObserver>();
}

PeerConnectionWrapper::~PeerConnectionWrapper() {
}

bool PeerConnectionWrapper::createOffer(
		ttLibC_WebrtcEventFunc func,
		ttLibC_WebrtcConstraint *constraint) {
	// nullチェックいれねば
	// constraint準備する
	WebrtcConstraint nativeConstraint;
	nativeConstraint.update(constraint);
	// config(iceServerとか)を準備する
	PeerConnectionInterface::RTCConfiguration configuration;
	configuration.servers = _iceServers;
	// Peerをつくる
	_nativePeerConnection = _nativePeerConnectionFactory->CreatePeerConnection(
			configuration,
			&nativeConstraint,
			nullptr,
			nullptr,
			this);
	// localStreamが設定されている場合は、ここでnativePeerConnectionに適応しなければならない。
	// offerを作る。(あとはcallbackの仕事)
	_nativePeerConnection->CreateOffer(new rtc::RefCountedObject<CreateSdpObserver>(func, _cPeerConnection), &nativeConstraint);
	return true;
}

bool PeerConnectionWrapper::createAnswer(
		ttLibC_WebrtcEventFunc func,
		ttLibC_WebrtcConstraint *constraint) {
	// nullチェックいれないと・・・
	// constraint準備する。
	WebrtcConstraint nativeConstraint;
	nativeConstraint.update(constraint);
	// config(iceServerとか準備する。)
	PeerConnectionInterface::RTCConfiguration configuration;
	configuration.servers = _iceServers;
	// peerを作る。
	_nativePeerConnection = _nativePeerConnectionFactory->CreatePeerConnection(
			configuration,
			&nativeConstraint,
			nullptr,
			nullptr,
			this);
	// sdpのデータを保持している場合は、先にセットする(でないとcreateAnswerが失敗する)
	if(_sdpCopy != NULL) {
		setRemoteDescription(_sdpCopy);
/*		char *type = (char *)_sdpCopy->type;
		char *value = (char *)_sdpCopy->value;
		puts("ここで死んでる気がする。");
		ttLibC_free(type);
		ttLibC_free(value);
		ttLibC_free(_sdpCopy);*/
		_sdpCopy = NULL;
		puts("あとはanswerつくるだけ。");
	}
	// localStreamが設定されている場合は、ここでnativePeerConnectionに適応しなければならない。
	_nativePeerConnection->CreateAnswer(new rtc::RefCountedObject<CreateSdpObserver>(func, _cPeerConnection), &nativeConstraint);
	return true;
}

bool PeerConnectionWrapper::setLocalDescription(ttLibC_WebrtcSdp *sdp) {
	if(_nativePeerConnection == nullptr) {
		return false;
	}
	if(sdp == NULL) {
		return false;
	}
	SdpParseError error;
	// sdpを復元
	SessionDescriptionInterface *nativeSdp = webrtc::CreateSessionDescription(
			std::string(sdp->type),
			std::string(sdp->value),
			&error);
	// セット
	_nativePeerConnection->SetLocalDescription(_setSdpObserver, nativeSdp);
	return true;
}

bool PeerConnectionWrapper::setRemoteDescription(ttLibC_WebrtcSdp *sdp) {
	if(_nativePeerConnection == nullptr) {
		if(_sdpCopy != NULL) {
			puts("すでにsdpCopyが提供済みになってる。なにかがおかしい。");
			return false;
		}
		// sdp情報の設定を先におこないたい場合は_sdpCopyにデータをいれておいて、createAnswerのときに利用する。
		// メモリーコピーにしたかったけど、よくわからないエラーがでたので、とりあえずrefにした。
/*		_sdpCopy = (ttLibC_WebrtcSdp *)ttLibC_malloc(sizeof(ttLibC_WebrtcSdp));
		char *type = (char *)ttLibC_malloc(256);
		strcpy(type, sdp->type);
		char *value = (char *)ttLibC_malloc(1024);
		strcpy(value, sdp->value);
		_sdpCopy->type = type;
		_sdpCopy->value = value;*/

		_sdpCopy = sdp;
		return true;
	}
	if(sdp == NULL) {
		puts("sdpがないので、できない。");
		return false;
	}
	SdpParseError error;
	// ttLibC_WebrtcSdpからnativeのsdpに変換する。
	SessionDescriptionInterface *nativeSdp = webrtc::CreateSessionDescription(
			std::string(sdp->type),
			std::string(sdp->value),
			&error);
	// セット
	_nativePeerConnection->SetRemoteDescription(_setSdpObserver, nativeSdp);
	return true;
}

bool PeerConnectionWrapper::addIceCandidate(ttLibC_WebrtcCandidate *candidate) {
	if(_nativePeerConnection == nullptr) {
		return false;
	}
	if(candidate == NULL) {
		return false;
	}
	SdpParseError error;
	// iceCandidateを復元
	IceCandidateInterface *nativeCandidate = webrtc::CreateIceCandidate(
			std::string(candidate->sdpMid),
			candidate->sdpMLineIndex,
			std::string(candidate->value),
			&error);
	// セット
	_nativePeerConnection->AddIceCandidate(nativeCandidate);
	return true;
}

void PeerConnectionWrapper::OnSignalingChange(PeerConnectionInterface::SignalingState new_state) {
	ttLibC_WebrtcEvent event = {};
	event.target = _cPeerConnection;
	if(_cPeerConnection->onSignalingChange != NULL) {
		_cPeerConnection->onSignalingChange(&event);
	}
}

// テストで追加した、videoFrameの取得動作(あとでつくり込む)この処理は消すべし
namespace ttLibC_webrtc {
class VideoFrameListener
	: public rtc::VideoSinkInterface<cricket::VideoFrame> {
public:
	void OnFrame(const cricket::VideoFrame& frame) override {
		puts("映像ゲット");
	}
	virtual ~VideoFrameListener() {}
};
}

void PeerConnectionWrapper::OnAddStream(rtc::scoped_refptr<MediaStreamInterface> stream) {
	ttLibC_WebrtcEvent event = {};
	event.target = _cPeerConnection;
	// event.stream; // ここをいじってc言語側からトラックにアクセスできるようにしなければならない。
	// データ通信できているかどうか調べるために、videoTrack１つ目について、データを取得しているか調査
	VideoTrackVector videoTracks = stream->GetVideoTracks();
	if(videoTracks.size() != 0) {
		VideoTrackInterface *videoTrack = videoTracks.front();
		rtc::VideoSinkWants wants; // これ・・・よくわからないので、調査する必要がある。
		wants.rotation_applied = false;
		wants.black_frames = true;
		videoTrack->AddOrUpdateSink(new VideoFrameListener(), wants);
	}
	// c言語側へのcallbackを流しておく。
	if(_cPeerConnection->onAddStream != NULL) {
		_cPeerConnection->onAddStream(&event);
	}
}
void PeerConnectionWrapper::OnRemoveStream(rtc::scoped_refptr<MediaStreamInterface> stream) {
	ttLibC_WebrtcEvent event = {};
	event.target = _cPeerConnection;
	// event.stream; // なにか設置したい。
	if(_cPeerConnection->onRemoveStream != NULL) {
		_cPeerConnection->onRemoveStream(&event);
	}
}
void PeerConnectionWrapper::OnDataChannel(rtc::scoped_refptr<DataChannelInterface> data_channel) {
	ttLibC_WebrtcEvent event = {};
	event.target = _cPeerConnection;
	// DataChannelが利用可能になるようになにかしておきたい。
	if(_cPeerConnection->onDataChannel != NULL) {
		_cPeerConnection->onDataChannel(&event);
	}
}
void PeerConnectionWrapper::OnRenegotiationNeeded() {
	ttLibC_WebrtcEvent event = {};
	event.target = _cPeerConnection;
	if(_cPeerConnection->onRenegotiationNeeded != NULL) {
		_cPeerConnection->onRenegotiationNeeded(&event);
	}
}
void PeerConnectionWrapper::OnIceConnectionChange(PeerConnectionInterface::IceConnectionState new_state) {
	ttLibC_WebrtcEvent event = {};
	event.target = _cPeerConnection;
	if(_cPeerConnection->onIceConnectionChange != NULL) {
		_cPeerConnection->onIceConnectionChange(&event);
	}
}
void PeerConnectionWrapper::OnIceGatheringChange(PeerConnectionInterface::IceGatheringState new_state) {
	ttLibC_WebrtcEvent event = {};
	event.target = _cPeerConnection;
	if(_cPeerConnection->onIceGatheringChange != NULL) {
		_cPeerConnection->onIceGatheringChange(&event);
	}
}
void PeerConnectionWrapper::OnIceCandidate(const IceCandidateInterface* candidate) {
	ttLibC_WebrtcEvent event = {};
	event.target = _cPeerConnection;
	// ここでは、iceCandidateの内容を取得して、eventに紐付けておく。
	ttLibC_WebrtcCandidate cCandidate;
	cCandidate.sdpMLineIndex = candidate->sdp_mline_index();
	cCandidate.sdpMid = candidate->sdp_mid().c_str();
	std::string str;
	candidate->ToString(&str);
	cCandidate.value = str.c_str();
	event.candidate = &cCandidate;
	if(_cPeerConnection->onIceCandidate != NULL) {
		_cPeerConnection->onIceCandidate(&event);
	}
}
void PeerConnectionWrapper::OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) {
	ttLibC_WebrtcEvent event = {};
	event.target = _cPeerConnection;
	if(_cPeerConnection->onIceCandidatesRemoved != NULL) {
		_cPeerConnection->onIceCandidatesRemoved(&event);
	}
}
void PeerConnectionWrapper::OnIceConnectionReceivingChange(bool receiving) {
	ttLibC_WebrtcEvent event = {};
	event.target = _cPeerConnection;
	if(_cPeerConnection->onIceConnectionReceivingChange != NULL) {
		_cPeerConnection->onIceConnectionReceivingChange(&event);
	}
}

extern "C" {

bool ttLibC_WebrtcPeerConnection_createOffer(
		ttLibC_WebrtcPeerConnection *conn,
		ttLibC_WebrtcEventFunc func,
		ttLibC_WebrtcConstraint *constraint) {
	if(conn == NULL) {
		return false;
	}
	ttLibC_WebrtcPeerConnection_ *conn_ = (ttLibC_WebrtcPeerConnection_ *)conn;
	return conn_->peerConnectionWrapper->createOffer(func, constraint);
}
bool ttLibC_WebrtcPeerConnection_createAnswer(
		ttLibC_WebrtcPeerConnection *conn,
		ttLibC_WebrtcEventFunc func,
		ttLibC_WebrtcConstraint *constraint) {
	if(conn == NULL) {
		return false;
	}
	ttLibC_WebrtcPeerConnection_ *conn_ = (ttLibC_WebrtcPeerConnection_ *)conn;
	return conn_->peerConnectionWrapper->createAnswer(func, constraint);
}
bool ttLibC_WebrtcPeerConnection_setLocalDescription(
		ttLibC_WebrtcPeerConnection *conn,
		ttLibC_WebrtcSdp *sdp) {
	if(conn == NULL) {
		return false;
	}
	ttLibC_WebrtcPeerConnection_ *conn_ = (ttLibC_WebrtcPeerConnection_ *)conn;
	return conn_->peerConnectionWrapper->setLocalDescription(sdp);
}
bool ttLibC_WebrtcPeerConnection_setRemoteDescription(
		ttLibC_WebrtcPeerConnection *conn,
		ttLibC_WebrtcSdp *sdp) {
	if(conn == NULL) {
		return false;
	}
	ttLibC_WebrtcPeerConnection_ *conn_ = (ttLibC_WebrtcPeerConnection_ *)conn;
	return conn_->peerConnectionWrapper->setRemoteDescription(sdp);
}
bool ttLibC_WebrtcPeerConnection_addIceCandidate(
		ttLibC_WebrtcPeerConnection *conn,
		ttLibC_WebrtcCandidate *candidate) {
	if(conn == NULL) {
		return false;
	}
	ttLibC_WebrtcPeerConnection_ *conn_ = (ttLibC_WebrtcPeerConnection_ *)conn;
	return conn_->peerConnectionWrapper->addIceCandidate(candidate);
}
void ttLibC_WebrtcPeerConnection_close(ttLibC_WebrtcPeerConnection **conn) {
	ttLibC_WebrtcPeerConnection_ *target = (ttLibC_WebrtcPeerConnection_ *)*conn;
	if(target == NULL) {
		return;
	}
	delete target->peerConnectionWrapper;
	ttLibC_free(target);
	*conn = NULL;
}
} /* extern "C" */

#endif
