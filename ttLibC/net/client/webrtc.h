/**
 * @file   webrtc.h
 * @brief  support for webrtc client.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/10/08
 */

#ifndef TTLIBC_NET_CLIENT_WEBRTC_H_
#define TTLIBC_NET_CLIENT_WEBRTC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * constraint detail data.
 * if NULL use default.
 */
typedef struct ttLibC_net_client_WebrtcConstraintData {
	const char *minAspectRatio;
	const char *maxAspectRatio;
	const char *maxWidth;
	const char *minWidth;
	const char *maxHeight;
	const char *minHeight;
	const char *maxFrameRate;
	const char *minFrameRate;
	const char *echoCancellation;
	const char *googEchoCancellation;
	const char *googEchoCancellation2;
	const char *googDAEchoCancellation;
	const char *googAutoGainControl;
	const char *googAutoGainControl2;
	const char *googNoiseSuppression;
	const char *googNoiseSuppression2;

	const char *intelligibilityEnhancer; // new
	const char *levelControl; // new

	const char *googHighpassFilter;
	const char *googTypingNoiseDetection;
	const char *googAudioMirroring;
	const char *googNoiseReduction;
	const char *offerToReceiveVideo;
	const char *offerToReceiveAudio;
	const char *voiceActivityDetection;
	const char *iceRestart;
	const char *googUseRtpMux;
	const char *EnableDTLS_SRTP;
	const char *EnableRTPDataChannels;
	const char *googDscp;
	const char *googIPv6;
	const char *googSuspendBelowMinBitrate;
	const char *googCombinedAudioVideoBwe;
	const char *googScreencastMinBitrate;
	const char *googCpuOveruseDetection;
	const char *googPayloadPadding;
//	const char *internalConstraintPrefix;
} ttLibC_net_client_WebrtcConstraintData;
typedef ttLibC_net_client_WebrtcConstraintData ttLibC_WebrtcConstraintData;

/**
 * constraint object.
 */
typedef struct ttLibC_net_client_WebrtcConstraint {
	ttLibC_WebrtcConstraintData mandatory;
	ttLibC_WebrtcConstraintData optional;
} ttLibC_net_client_WebrtcConstraints;
typedef ttLibC_net_client_WebrtcConstraint ttLibC_WebrtcConstraint;

/**
 * ice server information
 */
typedef struct ttLibC_net_client_WebrtcIceServer {
	const char *uri;
	const char *username;
	const char *password;
	// void *iceServer_uris;
} ttLibC_net_client_WebrtcIceServer;
typedef ttLibC_net_client_WebrtcIceServer ttLibC_WebrtcIceServer;

/**
 * config for peerConnection.
 */
typedef struct ttLibC_net_client_WebrtcConfig {
	ttLibC_WebrtcIceServer *iceServers;
	int size; // num of iceServers.
} ttLibC_net_client_WebrtcConfig;
typedef ttLibC_net_client_WebrtcConfig ttLibC_WebrtcConfig;

/**
 * sdp information
 */
typedef struct ttLibC_net_client_WebrtcSdp {
	const char *type;
	const char *value;
} ttLibC_net_client_WebrtcSdp;
typedef ttLibC_net_client_WebrtcSdp ttLibC_WebrtcSdp;

/**
 * candidate information
 */
typedef struct ttLibC_net_client_WebrtcCandidate {
	int sdpMLineIndex;
	const char *sdpMid;
	const char *value;
} ttLibC_net_client_WebrtcCandidate;
typedef ttLibC_net_client_WebrtcCandidate ttLibC_WebrtcCandidate;

/**
 * factory object definition.
 */
typedef struct ttLibC_net_client_WebrtcFactory {
} ttLibC_net_client_WebrtcFactory;
typedef ttLibC_net_client_WebrtcFactory ttLibC_WebrtcFactory;

typedef struct ttLibC_net_client_WebrtcPeerConnection ttLibC_PeerConnection;
typedef struct ttLibC_net_client_WebrtcMediaStream ttLibC_WebrtcMediaStream;

/**
 * event object.
 */
typedef struct ttLibC_net_client_WebrtcEvent {
	ttLibC_WebrtcCandidate *candidate;
	ttLibC_WebrtcSdp *sdp;
	ttLibC_WebrtcMediaStream *stream;
	ttLibC_PeerConnection *target;
} ttLibC_net_client_WebrtcEvent;
typedef ttLibC_net_client_WebrtcEvent ttLibC_WebrtcEvent;

/**
 * webrtc callback func definition.
 */
typedef bool (* ttLibC_WebrtcEventFunc)(ttLibC_WebrtcEvent *event);

/**
 * connection object definition.
 */
typedef struct ttLibC_net_client_WebrtcPeerConnection {
	ttLibC_WebrtcEventFunc onSignalingChange;
	ttLibC_WebrtcEventFunc onAddStream;
	ttLibC_WebrtcEventFunc onRemoveStream;
	ttLibC_WebrtcEventFunc onDataChannel;
	ttLibC_WebrtcEventFunc onRenegotiationNeeded;
	ttLibC_WebrtcEventFunc onIceConnectionChange;
	ttLibC_WebrtcEventFunc onIceGatheringChange;
	ttLibC_WebrtcEventFunc onIceCandidate;
	ttLibC_WebrtcEventFunc onIceCandidatesRemoved;
	ttLibC_WebrtcEventFunc onIceConnectionReceivingChange;
	void *ptr; // used def pointer.
	uint64_t id; // user def id number. you can put any.
} ttLibC_net_client_WebrtcPeerConnection;
typedef ttLibC_net_client_WebrtcPeerConnection ttLibC_WebrtcPeerConnection;

/**
 * for media stream.(do later.)
 */
typedef struct ttLibC_net_client_WebrtcMediaStream {
} ttLibC_net_client_WebrtcMediaStream;
typedef ttLibC_net_client_WebrtcMediaStream ttLibC_WebrtcMediaStream;

// methods
/**
 * initialize ssl usage.
 */
void ttLibC_WebrtcFactory_initSSL();

/**
 * clean ssl usage.
 */
void ttLibC_WebrtcFactory_cleanSSL();

/**
 * generate webrtc factory.
 */
ttLibC_WebrtcFactory *ttLibC_WebrtcFactory_make();

/**
 * close webrtc factory
 * @param factory
 */
void ttLibC_WebrtcFactory_close(ttLibC_WebrtcFactory **factory);

/**
 * make new peerConnection from factory.
 * @param factory
 * @param config (iceServer information and so on...)
 */
ttLibC_WebrtcPeerConnection *ttLibC_WebrtcFactory_createPeerConnection(
		ttLibC_WebrtcFactory *factory,
		ttLibC_WebrtcConfig *config);

/**
 * create offer for peer.
 * @param conn       target peer connection
 * @param func       callback after offer. you can get sdp information.
 * @param constraint constraint information.
 */
bool ttLibC_WebrtcPeerConnection_createOffer(
		ttLibC_WebrtcPeerConnection *conn,
		ttLibC_WebrtcEventFunc func,
		ttLibC_WebrtcConstraint *constraint);
/**
 * create answer for peer
 * @param conn       target peer connection
 * @param func       callback after answer. you can get sdp information.
 * @param constraint constraint information.
 * @note you have to set remoteSdp before answering, or failed.
 */
bool ttLibC_WebrtcPeerConnection_createAnswer(
		ttLibC_WebrtcPeerConnection *conn,
		ttLibC_WebrtcEventFunc func,
		ttLibC_WebrtcConstraint *constraint);

/**
 * set local sdp on peer
 * @param conn target peer connection
 * @param sdp  sdp information
 */
bool ttLibC_WebrtcPeerConnection_setLocalDescription(
		ttLibC_WebrtcPeerConnection *conn,
		ttLibC_WebrtcSdp *sdp);

/**
 * set remote sdp on peer
 * @param conn target peer connection
 * @param sdp  sdp information
 */
bool ttLibC_WebrtcPeerConnection_setRemoteDescription(
		ttLibC_WebrtcPeerConnection *conn,
		ttLibC_WebrtcSdp *sdp);

/**
 * add ice candidate information.
 * @param conn      target peer connection
 * @param candidate candidate information
 */
bool ttLibC_WebrtcPeerConnection_addIceCandidate(
		ttLibC_WebrtcPeerConnection *conn,
		ttLibC_WebrtcCandidate *candidate);

/**
 * close peer connection.
 */
void ttLibC_WebrtcPeerConnection_close(ttLibC_WebrtcPeerConnection **conn);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_WEBRTC_H_ */
