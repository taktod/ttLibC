/**
 * @file   factory.h
 * @brief  factory for webrtc objects.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/10/08
 */

#ifndef TTLIBC_NET_CLIENT_WEBRTC_FACTORY_H_
#define TTLIBC_NET_CLIENT_WEBRTC_FACTORY_H_

#include "../webrtc.h"
#include <webrtc/base/thread.h>
#include <webrtc/base/scoped_ref_ptr.h>
#include <webrtc/api/peerconnectioninterface.h>

namespace ttLibC_webrtc {

class FactoryWrapper {
public:
	FactoryWrapper();
	~FactoryWrapper();
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> refNativeFactory();
private:
	rtc::Thread _signalingThread;
	rtc::Thread _workerThread;
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _nativeFactory;
};

}

typedef struct ttLibC_net_client_WebrtcFactory_ {
	ttLibC_WebrtcFactory inherit_super;
	// scoped refptrのクラスを直接構造体に持たせるとおかしくなる模様。
	//  delete 演算子になにかやってる？
	// とりあえずclassでwrapすることで回避した。
	ttLibC_webrtc::FactoryWrapper *factoryWrapper;
} ttLibC_net_client_WebrtcFactory_;
typedef ttLibC_net_client_WebrtcFactory_ ttLibC_WebrtcFactory_;

#endif /* TTLIBC_NET_CLIENT_WEBRTC_FACTORY_H_ */
