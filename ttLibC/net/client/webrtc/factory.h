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
#include <webrtc/api/mediastreaminterface.h>
#include <webrtc/api/mediaconstraintsinterface.h>
#include <map>

namespace ttLibC_webrtc {

/**
 * constraint object
 * ttLibC_WebrtcConstraint -> MediaConstraintsInterfaceの仲だちさせる
 */
class WebrtcConstraint : public webrtc::MediaConstraintsInterface {
public:
	WebrtcConstraint() {};
	virtual ~WebrtcConstraint() {};

	virtual const Constraints& GetMandatory() const {
		return _mandatory;
	}
	virtual const Constraints& GetOptional() const {
		return _optional;
	}
	template<class T>
	void AddMandatory(const std::string &key, const T &value) {
		_mandatory.push_back(Constraint(key, rtc::ToString<T>(value)));
	}

	template<class T>
	void SetMandatory(const std::string &key, const T &value) {
		std::string value_str;
		if(_mandatory.FindFirst(key, &value_str)) {
			for(Constraints::iterator iter = _mandatory.begin();iter != _mandatory.end(); ++iter) {
				if(iter->key == key) {
					_mandatory.erase(iter);
					break;
				}
			}
		}
		_mandatory.push_back(Constraint(key, rtc::ToString<T>(value)));
	}

	template<class T>
	void AddOptional(const std::string &key, const T &value) {
		_optional.push_back(Constraint(key, rtc::ToString<T>(value)));
	}
	// ttLibC_WebrtcConstraintの内容で更新する。
	void update(ttLibC_WebrtcConstraint *constraint);
private:
	void analyze(Constraints& target, ttLibC_net_client_WebrtcConstraintData *data);
	template<class T>
	void add(Constraints& target, const std::string& key, const T &value) {
		target.push_back(Constraint(key, rtc::ToString<T>(value)));
	}
	Constraints _mandatory;
	Constraints _optional;
};

class FactoryWrapper {
public:
	FactoryWrapper();
	~FactoryWrapper();
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> refNativeFactory();
	void registMediaStream(
			ttLibC_WebrtcMediaStream *stream,
			bool isLocal);
	ttLibC_WebrtcMediaStream *getLocalStream(std::string name) {
		return _localStreams[name];
	};
	ttLibC_WebrtcMediaStream *getRemoteStream(std::string name) {
		return _remoteStreams[name];
	};
private:
	rtc::Thread _signalingThread;
	rtc::Thread _workerThread;
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _nativeFactory;
	// ここでstring -> mediaStreamのmapを保持しておく。
	std::map<std::string, ttLibC_WebrtcMediaStream *> _localStreams;
	std::map<std::string, ttLibC_WebrtcMediaStream *> _remoteStreams;
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

extern "C" {

ttLibC_WebrtcMediaStream *ttLibC_WebrtcFactory_createRemoteStream(
		ttLibC_webrtc::FactoryWrapper *factoryWrapper,
		rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);

}

#endif /* TTLIBC_NET_CLIENT_WEBRTC_FACTORY_H_ */
