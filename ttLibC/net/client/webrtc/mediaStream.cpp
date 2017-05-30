/*
 * @file   mediaStream.cpp
 * @brief  mediaStream object for webrtc.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/10/08
 */

#ifdef __ENABLE_WEBRTC__

#include "mediaStream.h"
#include "../../../allocator.h"
#include "../../../_log.h"
#include "factory.h"
#include <uuid/uuid.h>

using namespace webrtc;
using namespace ttLibC_webrtc;

void VideoFrameListener::OnFrame(const cricket::VideoFrame& frame) {
	if(_startPts == 0) {
		_startPts = frame.timestamp_us();
	}
	_streamWrapper->OnFrame(frame, _id, frame.timestamp_us() - _startPts);
}

void AudioFrameListener::OnData(
		const void *data,
		int bits_per_sample,
		int sample_rate,
		size_t number_of_channels,
		size_t number_of_frames) {
	_streamWrapper->OnData(
			data,
			bits_per_sample,
			sample_rate,
			number_of_channels,
			number_of_frames,
			_id,
			_pts);
	_pts += number_of_frames;
}

MediaStreamWrapper::MediaStreamWrapper(
		FactoryWrapper *factoryWrapper,
		const char *name) {
	_isLocal = true;
	_factoryWrapper = factoryWrapper;
	_nativeStream = factoryWrapper->refNativeFactory()->CreateLocalMediaStream(std::string(name));
	_callback = NULL;
	_ptr = NULL;
	_pcm_frame = NULL;
	_yuv_frame = NULL;
}

MediaStreamWrapper::MediaStreamWrapper(
		FactoryWrapper *factoryWrapper,
		rtc::scoped_refptr<MediaStreamInterface> nativeStream) {
	_isLocal = false;
	_factoryWrapper = factoryWrapper;
	_nativeStream = nativeStream;
	// remoteStreamを作ったら、自動的に全トラックにsinkを追加しておく必要があるか・・・
	// 初めっからsinkを追加しておく方向ですすめようと思う
	// callbackが呼ばれる動作そのものはそれほど重い処理ではないだろう。
	_callback = NULL;
	_ptr = NULL;
	_pcm_frame = NULL;
	_yuv_frame = NULL;
	// ここですべてのtrackを検索して、videoListenerとaudioListenerを追加しなければならない。
	AudioTrackVector audioTracks = nativeStream->GetAudioTracks(); // 音声トラック全部
	AudioTrackVector::iterator audioIter = audioTracks.begin();
	uint32_t id = 0;
	while(audioIter != audioTracks.end()) {
		rtc::scoped_refptr<AudioTrackInterface> track = *audioIter;
		// このtrackについて、listenerをくっつける必要があるぜ。
		track->AddSink(new AudioFrameListener(this, id));
		++ id;
		++ audioIter;
	}
	VideoTrackVector videoTracks = nativeStream->GetVideoTracks(); // 映像トラック全部
	VideoTrackVector::iterator videoIter = videoTracks.begin();
	rtc::VideoSinkWants wants; // これ・・・よくわからないので、調査する必要がある。
	wants.rotation_applied = false;
	wants.black_frames = true;
	id = 0;
	while(videoIter != videoTracks.end()) {
		rtc::scoped_refptr<VideoTrackInterface> track = *videoIter;
		track->AddOrUpdateSink(new VideoFrameListener(this, id), wants);
		++ id;
		++ videoIter;
	}
}

MediaStreamWrapper::~MediaStreamWrapper() {
	ttLibC_PcmS16_close(&_pcm_frame);
	ttLibC_Yuv420_close(&_yuv_frame);
}

bool MediaStreamWrapper::createNewVideoTrack(
		ttLibC_WebrtcConstraint *constraint) {
	// これ・・映像サイズの指定って必要だよね・・・たぶん。
	// こっちはlocalでのみ動作
	if(!_isLocal) {
		ERR_PRINT("new track creation is for local stream only.");
		return false;
	}
	return true;
}

bool MediaStreamWrapper::createNewAudioTrack(
		ttLibC_WebrtcConstraint *constraint) {
	// こっちはchannel数、sampleRateは初めから設定されているものを使わないとだめ
	if(!_isLocal) {
		ERR_PRINT("new track creation is for local stream only.");
		return false;
	}
	return true;
}

bool MediaStreamWrapper::addFrameListener(
		ttLibC_WebrtcMediaStreamFrameFunc callback,
		void *ptr) {
	if(_isLocal) {
		ERR_PRINT("frame listener is for remote stream only.");
		return false;
	}
	_callback = callback;
	_ptr = ptr;
	return true;
}

bool MediaStreamWrapper::addFrame(ttLibC_Frame *frame) {
	if(!_isLocal) {
		ERR_PRINT("add frame is for local stream only.");
		return false;
	}
	return true;
}

void MediaStreamWrapper::OnFrame(
		const cricket::VideoFrame& frame,
		uint32_t id,
		uint64_t pts) {
//	printf("v:%llu, %llu\n", pts, pts / 1000);
	if(_callback == NULL) {
		return;
	}
	// 初めに取得したときのtimestampのズレから推測するしか方法がないとしておくか・・・
	// 音声は先行してデータがくることがあるべし。
	// 映像はあとから始まるけど、はじまった時間が0になる。
	// それなりな時間になった。まぁいいだろう。
//	rtc::scoped_refptr<webrtc::VideoFrameBuffer>& buf = frame.video_frame_buffer();
	ttLibC_Yuv420 *yuv420 = ttLibC_Yuv420_make(
			_yuv_frame,
			Yuv420Type_planar,
			(uint32_t)frame.width(),
			(uint32_t)frame.height(),
			NULL,
			0,
			(void *)frame.video_frame_buffer()->DataY(),
			frame.video_frame_buffer()->StrideY(),
			(void *)frame.video_frame_buffer()->DataU(),
			frame.video_frame_buffer()->StrideU(),
			(void *)frame.video_frame_buffer()->DataV(),
			frame.video_frame_buffer()->StrideV(),
			true,
			pts,
			1000000);
	if(yuv420 == NULL) {
		ERR_PRINT("failed to make yuv420 object.");
		return;
	}
	yuv420->inherit_super.inherit_super.id = id;
	_yuv_frame = yuv420;
	_callback(_ptr, NULL, (ttLibC_Frame *)_yuv_frame);
}

void MediaStreamWrapper::OnData(
		const void *data,
		int bits_per_sample,
		int sample_rate,
		size_t number_of_channels,
		size_t number_of_frames,
		uint32_t id,
		uint64_t pts) {
	if(_callback == NULL) {
		return;
	}
	switch(bits_per_sample) {
	case 16:
		{
			ttLibC_PcmS16 *pcm = ttLibC_PcmS16_make(_pcm_frame,
					PcmS16Type_littleEndian,
					sample_rate,
					number_of_frames,
					number_of_channels,
					NULL,
					0,
					(void *)data,
					number_of_frames * 2,
					NULL,
					0,
					true,
					pts,
					sample_rate);
			if(pcm == NULL) {
				ERR_PRINT("failed to make pcmS16 object.");
				return;
			}
			pcm->inherit_super.inherit_super.id = id;
			_pcm_frame = pcm;
			// calbackを応答する。
			_callback(_ptr, NULL, (ttLibC_Frame *)_pcm_frame);
		}
		break;
	default:
		ERR_PRINT("unexpected sample_rate.");
		break;
	}
}

extern "C" {

bool ttLibC_WebrtcMediaStream_createNewVideoTrack(
		ttLibC_WebrtcMediaStream *stream,
		ttLibC_WebrtcConstraint *constraint) {
	if(stream == NULL) {
		return false;
	}
	ttLibC_WebrtcMediaStream_ *stream_ = (ttLibC_WebrtcMediaStream_ *)stream;
	return stream_->streamWrapper->createNewVideoTrack(constraint);
}

bool ttLibC_WebrtcMediaStream_createNewAudioTrack(
		ttLibC_WebrtcMediaStream *stream,
		ttLibC_WebrtcConstraint *constraint) {
	if(stream == NULL) {
		return false;
	}
	ttLibC_WebrtcMediaStream_ *stream_ = (ttLibC_WebrtcMediaStream_ *)stream;
	return stream_->streamWrapper->createNewAudioTrack(constraint);
}

bool ttLibC_WebrtcMediaStream_addFrameListener(
		ttLibC_WebrtcMediaStream *stream,
		ttLibC_WebrtcMediaStreamFrameFunc callback,
		void *ptr) {
	if(stream == NULL) {
		return false;
	}
	ttLibC_WebrtcMediaStream_ *stream_ = (ttLibC_WebrtcMediaStream_ *)stream;
	return stream_->streamWrapper->addFrameListener(callback, ptr);
}

// indexはframe.idでなんとかすることにする。
bool ttLibC_WebrtcMediaStream_addFrame(
		ttLibC_WebrtcMediaStream *stream,
		ttLibC_Frame *frame) {
	if(stream == NULL) {
		return false;
	}
	ttLibC_WebrtcMediaStream_ *stream_ = (ttLibC_WebrtcMediaStream_ *)stream;
	return stream_->streamWrapper->addFrame(frame);
}

}

#endif
