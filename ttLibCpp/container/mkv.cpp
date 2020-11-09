/**
 * 
 */

#include "mkv.hpp"

#include <iostream>
#include <map>
#include <queue>
#include <ttLibC/frame/video/h264.h>
#include <ttLibC/frame/video/h265.h>
#include <ttLibC/frame/audio/mp3.h>
#include <ttLibC/frame/audio/aac2.h>
#include <ttLibC/util/ioUtil.h>
#include <ttLibC/util/hexUtil.h>

#include "../util/dynamicBufferUtil.hpp"
#include "../util/byteUtil.hpp"

using namespace std;

namespace ttLibCpp {

using dataCallback = function<bool(void *data, size_t dataSize)>;

class MkvTrack {
public:
  MkvTrack(MkvWriter *writer, uint32_t id) : isReady(false), initDone(false), id(id), writer(writer) {}
  virtual ~MkvTrack(){}
  virtual void writeInit(dataCallback callback) = 0;
  virtual bool append(ttLibC_Frame *frame, dataCallback callback) = 0;
  virtual bool makeSegmentHeader(ttLibC_Frame *frame, dataCallback callback) = 0;

  bool isReady;
protected:
  MkvWriter *writer;
  bool initDone;
  uint32_t id;
  queue<vector<uint8_t>> queue;
};

class H264Track : public MkvTrack {
public:
  H264Track(MkvWriter *writer, uint32_t id) : MkvTrack(writer, id), configData(nullptr) {}
  ~H264Track(){ttLibC_H264_close(&configData);}
  void writeInit(dataCallback callback) {
    // trackEntry
    //   trackNumber
    //   trackUID
    //   codecID
    //   trackType
    //   video
    //     pixelWidth
    //     pixelHeight
    //   codecPrivate
    uint8_t buf[256], innerBuf[256];
    size_t in_size = 0;
    unique_ptr<DynamicBuffer> trackEntryBuffer(DynamicBuffer::create());
    unique_ptr<ByteConnector> connector(ByteConnector::create(buf, 256, ByteUtilType_default));
    unique_ptr<ByteConnector> innerConnector(nullptr);
    connector->ebml(MkvType_TrackNumber, true)
             ->ebml(1, false)
             ->bit(id, 8);
    connector->ebml(MkvType_TrackUID, true)
             ->ebml(1, false)
             ->bit(id, 8);
    connector->ebml(MkvType_CodecID, true)
             ->ebml(15, false)
             ->string("V_MPEG4/ISO/AVC");
    connector->ebml(MkvType_TrackType, true)
             ->ebml(1, false)
             ->bit(1, 8);

    innerConnector.reset(ByteConnector::create(innerBuf, 256, ByteUtilType_default));
    innerConnector->ebml(MkvType_PixelWidth, true)
                  ->ebml(2, false)
                  ->bit(configData->inherit_super.width, 16);
    innerConnector->ebml(MkvType_PixelHeight, true)
                  ->ebml(2, false)
                  ->bit(configData->inherit_super.height, 16);
    connector->ebml(MkvType_Video, true)
             ->ebml(innerConnector->writeSize, false)
             ->string(std::string((const char *)innerBuf, innerConnector->writeSize));

    in_size = ttLibC_H264_readAvccTag(configData, innerBuf, 256);
    connector->ebml(MkvType_CodecPrivate, true)
             ->ebml(in_size, false);
    trackEntryBuffer->append(buf, connector->writeSize);
    trackEntryBuffer->append(innerBuf, in_size);

    connector.reset(ByteConnector::create(buf, 256, ByteUtilType_default));
    connector->ebml(MkvType_TrackEntry, true)
             ->ebml(trackEntryBuffer->refSize(), false);
    callback(buf, connector->writeSize);
    callback(trackEntryBuffer->refData(), trackEntryBuffer->refSize());

    initDone = true;
  }
  bool append(ttLibC_Frame *frame, dataCallback callback) {
    if(frame->type != frameType_h264) {
      cout << "not h264. corrupt." << endl;
      return false;
    }
    auto h264 = reinterpret_cast<ttLibC_H264 *>(frame);
    if(h264->type == H264Type_configData) {
      configData = ttLibC_H264_clone(configData, h264);
      isReady = true;
      return true;
    }
    bool result = true;
    uint8_t buf[256];
    unique_ptr<ByteConnector> header(ByteConnector::create(buf, 256, ByteUtilType_default));
    header->ebml(MkvType_SimpleBlock, true)
          ->ebml(4 + frame->buffer_size, false)
          ->ebml(frame->id, false)
          ->bit(frame->pts - writer->pts, 16);
    switch(h264->type) {
    case H264Type_slice:
      header->bit(0x00, 8);
      break;
    case H264Type_sliceIDR:
      header->bit(0x80, 8);
      break;
    default:
      return true;
    }
    // convert annexB(00 00 00 01) -> sizeNal(00 00 si ze)
    uint8_t *h264_data = (uint8_t *)frame->data;
    size_t h264_data_size = frame->buffer_size;
    ttLibC_H264_NalInfo nal_info;
    while(ttLibC_H264_getNalInfo(&nal_info, h264_data, h264_data_size)) {
      uint32_t nal_size = nal_info.nal_size - nal_info.data_pos;
      uint32_t be_nal_size = be_uint32_t(nal_size);
      uint32_t *u32 = (uint32_t *)h264_data;
      *u32 = be_nal_size;
      h264_data += nal_info.nal_size;
      h264_data_size -= nal_info.nal_size;
    }
    if(!initDone) {
      // stack on queue to use later.
      queue.push(vector<uint8_t>(buf, buf + header->writeSize));
      queue.push(vector<uint8_t>((uint8_t *)frame->data, (uint8_t *)frame->data + frame->buffer_size));
    }
    else {
      // pop all data in queue.
      while(queue.size() > 0) {
        vector<uint8_t> data = queue.front();
        if(result) {
          result = callback(data.data(), data.size());
        }
        queue.pop();
      }
      // callback current info.
      if(result) {
        result = callback(buf, header->writeSize);
      }
      if(result) {
        result = callback(frame->data, frame->buffer_size);
      }
    }
    return result;
  }
  bool makeSegmentHeader(ttLibC_Frame *frame, dataCallback callback) {
    if(!initDone) {
      return true;
    }
    // currently split with timestamp only.
    if(frame->pts > writer->pts + writer->unitDuration) {
      writer->pts = frame->pts;
      uint8_t buf[256];
      unique_ptr<ByteConnector> connector(ByteConnector::create(buf, 256, ByteUtilType_default));
      connector->ebml(MkvType_Cluster, true)
               ->bit(0x01FFFFFF, 32)
               ->bit(0xFFFFFFFF, 32);
      connector->ebml(MkvType_Timecode, true)
               ->ebml(8, false)
               ->bit((writer->pts >> 32), 32)
               ->bit(writer->pts, 32);
      return callback(buf, connector->writeSize);
    }
    return true;
  }
private:
  ttLibC_H264 *configData;
};

class H265Track : public MkvTrack {
public:
  H265Track(MkvWriter *writer, uint32_t id) : MkvTrack(writer, id), configData(nullptr) {}
  ~H265Track(){ttLibC_H265_close(&configData);}
  void writeInit(dataCallback callback) {
    // trackEntry
    //   trackNumber
    //   trackUID
    //   codecID
    //   trackType
    //   video
    //     pixelWidth
    //     pixelHeight
    //   codecPrivate
    uint8_t buf[256], innerBuf[256];
    size_t in_size = 0;
    unique_ptr<DynamicBuffer> trackEntryBuffer(DynamicBuffer::create());
    unique_ptr<ByteConnector> connector(ByteConnector::create(buf, 256, ByteUtilType_default));
    unique_ptr<ByteConnector> innerConnector(nullptr);
    connector->ebml(MkvType_TrackNumber, true)
             ->ebml(1, false)
             ->bit(id, 8);
    connector->ebml(MkvType_TrackUID, true)
             ->ebml(1, false)
             ->bit(id, 8);
    connector->ebml(MkvType_CodecID, true)
             ->ebml(16, false)
             ->string("V_MPEGH/ISO/HEVC");
    connector->ebml(MkvType_TrackType, true)
             ->ebml(1, false)
             ->bit(1, 8);

    innerConnector.reset(ByteConnector::create(innerBuf, 256, ByteUtilType_default));
    innerConnector->ebml(MkvType_PixelWidth, true)
                  ->ebml(2, false)
                  ->bit(configData->inherit_super.width, 16);
    innerConnector->ebml(MkvType_PixelHeight, true)
                  ->ebml(2, false)
                  ->bit(configData->inherit_super.height, 16);
    connector->ebml(MkvType_Video, true)
             ->ebml(innerConnector->writeSize, false)
             ->string(std::string((const char *)innerBuf, innerConnector->writeSize));

    in_size = ttLibC_H265_readHvccTag(configData, innerBuf, 256);
    connector->ebml(MkvType_CodecPrivate, true)
             ->ebml(in_size, false);
    trackEntryBuffer->append(buf, connector->writeSize);
    trackEntryBuffer->append(innerBuf, in_size);

    connector.reset(ByteConnector::create(buf, 256, ByteUtilType_default));
    connector->ebml(MkvType_TrackEntry, true)
             ->ebml(trackEntryBuffer->refSize(), false);
    callback(buf, connector->writeSize);
    callback(trackEntryBuffer->refData(), trackEntryBuffer->refSize());

    initDone = true;
  }
  bool append(ttLibC_Frame *frame, dataCallback callback) {
    if(frame->type != frameType_h265) {
      cout << "not h265. corrupt." << endl;
      return false;
    }
    auto h265 = reinterpret_cast<ttLibC_H265 *>(frame);
    if(h265->type == H265Type_configData) {
      configData = ttLibC_H265_clone(configData, h265);
      isReady = true;
      return true;
    }
    bool result = true;
    uint8_t buf[256];
    unique_ptr<ByteConnector> header(ByteConnector::create(buf, 256, ByteUtilType_default));
    header->ebml(MkvType_SimpleBlock, true)
          ->ebml(4 + frame->buffer_size, false)
          ->ebml(frame->id, false)
          ->bit(frame->pts - writer->pts, 16);
    switch(h265->type) {
    case H265Type_slice:
      header->bit(0x00, 8);
      break;
    case H265Type_sliceIDR:
      header->bit(0x80, 8);
      break;
    default:
      return true;
    }
    // annexB(00 00 00 01) -> sizeNal(00 00 si ze)
    uint8_t *h265_data = (uint8_t *)frame->data;
    size_t h265_data_size = frame->buffer_size;
    ttLibC_H265_NalInfo nal_info;
    while(ttLibC_H265_getNalInfo(&nal_info, h265_data, h265_data_size)) {
      uint32_t nal_size = nal_info.nal_size - nal_info.data_pos;
      uint32_t be_nal_size = be_uint32_t(nal_size);
      uint32_t *u32 = (uint32_t *)h265_data;
      *u32 = be_nal_size;
      h265_data += nal_info.nal_size;
      h265_data_size -= nal_info.nal_size;
    }
    if(!initDone) {
      queue.push(vector<uint8_t>(buf, buf + header->writeSize));
      queue.push(vector<uint8_t>((uint8_t *)frame->data, (uint8_t *)frame->data + frame->buffer_size));
    }
    else {
      while(queue.size() > 0) {
        vector<uint8_t> data = queue.front();
        if(result) {
          result = callback(data.data(), data.size());
        }
        queue.pop();
      }
      if(result) {
        result = callback(buf, header->writeSize);
      }
      if(result) {
        result = callback(frame->data, frame->buffer_size);
      }
    }
    return result;
  }
  bool makeSegmentHeader(ttLibC_Frame *frame, dataCallback callback) {
    if(!initDone) {
      return true;
    }
    // TODO writer->mode
    if(frame->pts > writer->pts + writer->unitDuration) {
      writer->pts = frame->pts;
      uint8_t buf[256];
      unique_ptr<ByteConnector> connector(ByteConnector::create(buf, 256, ByteUtilType_default));
      connector->ebml(MkvType_Cluster, true)
               ->bit(0x01FFFFFF, 32)
               ->bit(0xFFFFFFFF, 32);
      connector->ebml(MkvType_Timecode, true)
               ->ebml(8, false)
               ->bit((writer->pts >> 32), 32)
               ->bit(writer->pts, 32);
      return callback(buf, connector->writeSize);
    }
    return true;
  }
private:
  ttLibC_H265 *configData;
};

class AacTrack : public MkvTrack {
public:
  AacTrack(MkvWriter *writer, uint32_t id) : MkvTrack(writer, id), asiInfo(nullptr) {}
  ~AacTrack() {ttLibC_Aac2_close(&asiInfo);}
  void writeInit(dataCallback callback) {
    // trackEntry
    //   trackNumber
    //   trackUID
    //   codecID
    //   trackType
    //   audio
    //     samplingFrequency
    //     channels
    //   codecPrivate
    uint8_t buf[256], innerBuf[256];
    size_t in_size;
    unique_ptr<DynamicBuffer> trackEntryBuffer(DynamicBuffer::create());
    unique_ptr<ByteConnector> connector(ByteConnector::create(buf, 256, ByteUtilType_default));
    unique_ptr<ByteConnector> innerConnector(nullptr);
    connector->ebml(MkvType_TrackNumber, true)
             ->ebml(1, false)
             ->bit(id, 8);
    connector->ebml(MkvType_TrackUID, true)
             ->ebml(1, false)
             ->bit(id, 8);
    connector->ebml(MkvType_CodecID, true)
             ->ebml(5, false)
             ->string("A_AAC");
    connector->ebml(MkvType_TrackType, true)
             ->ebml(1, false)
             ->bit(2, 8);

    innerConnector.reset(ByteConnector::create(innerBuf, 256, ByteUtilType_default));
    float sr = sampleRate;
    innerConnector->ebml(MkvType_SamplingFrequency, true)
                  ->ebml(4, false)
                  ->bit(*(uint32_t *)&sr, 32);
    innerConnector->ebml(MkvType_Channels, true)
                  ->ebml(1, false)
                  ->bit(channelNum, 8);
    connector->ebml(MkvType_Audio, true)
             ->ebml(innerConnector->writeSize, false)
             ->string(std::string((const char *)innerBuf, innerConnector->writeSize));

    in_size = ttLibC_Aac2_makeAsiHeader(asiInfo, innerBuf, 256);
    connector->ebml(MkvType_CodecPrivate, true)
             ->ebml(in_size, false);
    trackEntryBuffer->append(buf, connector->writeSize);
    trackEntryBuffer->append(innerBuf, in_size);

    connector.reset(ByteConnector::create(buf, 256, ByteUtilType_default));
    connector->ebml(MkvType_TrackEntry, true)
             ->ebml(trackEntryBuffer->refSize(), false);
    callback(buf, connector->writeSize);
    callback(trackEntryBuffer->refData(), trackEntryBuffer->refSize());

    initDone = true;
  }
  bool append(ttLibC_Frame *frame, dataCallback callback) {
    if(frame->type != frameType_aac2) {
      cout << "not aac. corrupt." << endl;
      return false;
    }
    auto aac = reinterpret_cast<ttLibC_Aac2 *>(frame);
    if(asiInfo == nullptr) {
      asiInfo = ttLibC_Aac2_clone(asiInfo, aac);
    }
    if(aac->type == Aac2Type_asi) {
      return true;
    }
    channelNum = aac->inherit_super.channel_num;
    sampleRate = aac->inherit_super.sample_rate;
    isReady = true;
    bool result = true;
    uint8_t buf[256];
    unique_ptr<ByteConnector> header(ByteConnector::create(buf, 256, ByteUtilType_default));
    header->ebml(MkvType_SimpleBlock, true)
          ->ebml(4 + frame->buffer_size, false)
          ->ebml(frame->id, false)
          ->bit(frame->pts - writer->pts, 16)
          ->bit(0x80, 8);
    if(!initDone) {
      queue.push(vector<uint8_t>(buf, buf + header->writeSize));
      queue.push(vector<uint8_t>((uint8_t *)frame->data, (uint8_t *)frame->data + frame->buffer_size));
    }
    else {
      while(queue.size() > 0) {
        vector<uint8_t> data = queue.front();
        if(result) {
          result = callback(data.data(), data.size());
        }
        queue.pop();
      }
      if(result) {
        result = callback(buf, header->writeSize);
      }
      if(result) {
        result = callback(frame->data, frame->buffer_size);
      }
    }
    return result;
  }
  bool makeSegmentHeader(ttLibC_Frame *frame, dataCallback callback) {
    if(!initDone) {
      return true;
    }
    if(frame->pts > writer->pts + writer->unitDuration) {
      writer->pts = frame->pts;
      uint8_t buf[256];
      unique_ptr<ByteConnector> connector(ByteConnector::create(buf, 256, ByteUtilType_default));
      connector->ebml(MkvType_Cluster, true)
               ->bit(0x01FFFFFF, 32)
               ->bit(0xFFFFFFFF, 32);
      connector->ebml(MkvType_Timecode, true)
               ->ebml(8, false)
               ->bit((writer->pts >> 32), 32)
               ->bit(writer->pts, 32);
      return callback(buf, connector->writeSize);
    }
    return true;
  }
private:
  ttLibC_Aac2 *asiInfo;

  uint32_t channelNum;
  uint32_t sampleRate;
};

class Mp3Track : public MkvTrack {
public:
  Mp3Track(MkvWriter *writer, uint32_t id) : MkvTrack(writer, id){}
  ~Mp3Track() {}
  void writeInit(dataCallback callback) {
    // trackEntry
    //   trackNumber
    //   trackUID
    //   codecID
    //   trackType
    //   audio
    //     samplingFrequency
    //     channels
    uint8_t buf[256], innerBuf[256];
    unique_ptr<DynamicBuffer> trackEntryBuffer(DynamicBuffer::create());
    unique_ptr<ByteConnector> connector(ByteConnector::create(buf, 256, ByteUtilType_default));
    unique_ptr<ByteConnector> innerConnector(nullptr);
    connector->ebml(MkvType_TrackNumber, true)
             ->ebml(1, false)
             ->bit(id, 8);
    connector->ebml(MkvType_TrackUID, true)
             ->ebml(1, false)
             ->bit(id, 8);
    connector->ebml(MkvType_CodecID, true)
             ->ebml(9, false)
             ->string("A_MPEG/L3");
    connector->ebml(MkvType_TrackType, true)
             ->ebml(1, false)
             ->bit(2, 8);

    innerConnector.reset(ByteConnector::create(innerBuf, 256, ByteUtilType_default));
    float sr = sampleRate;
    innerConnector->ebml(MkvType_SamplingFrequency, true)
                  ->ebml(4, false)
                  ->bit(*(uint32_t *)&sr, 32);
    innerConnector->ebml(MkvType_Channels, true)
                  ->ebml(1, false)
                  ->bit(channelNum, 8);
    connector->ebml(MkvType_Audio, true)
             ->ebml(innerConnector->writeSize, false)
             ->string(std::string((const char *)innerBuf, innerConnector->writeSize));
    trackEntryBuffer->append(buf, connector->writeSize);
    connector.reset(ByteConnector::create(buf, 256, ByteUtilType_default));
    connector->ebml(MkvType_TrackEntry, true)
             ->ebml(trackEntryBuffer->refSize(), false);
    callback(buf, connector->writeSize);
    callback(trackEntryBuffer->refData(), trackEntryBuffer->refSize());

    initDone = true;
  }
  bool append(ttLibC_Frame *frame, dataCallback callback) {
    if(frame->type != frameType_mp3) {
      cout << "not mp3. corrupt." << endl;
      return false;
    }
    auto audio = reinterpret_cast<ttLibC_Audio *>(frame);
    channelNum = audio->channel_num;
    sampleRate = audio->sample_rate;
    isReady = true;
    bool result = true;
    uint8_t buf[256];
    unique_ptr<ByteConnector> header(ByteConnector::create(buf, 256, ByteUtilType_default));
    header->ebml(MkvType_SimpleBlock, true)
          ->ebml(4 + frame->buffer_size, false)
          ->ebml(frame->id, false)
          ->bit(frame->pts - writer->pts, 16)
          ->bit(0x80, 8);
    if(!initDone) {
      queue.push(vector<uint8_t>(buf, buf + header->writeSize));
      queue.push(vector<uint8_t>((uint8_t *)frame->data, (uint8_t *)frame->data + frame->buffer_size));
    }
    else {
      while(queue.size() > 0) {
        vector<uint8_t> data = queue.front();
        if(result) {
          result = callback(data.data(), data.size());
        }
        queue.pop();
      }
      if(result) {
        result = callback(buf, header->writeSize);
      }
      if(result) {
        result = callback(frame->data, frame->buffer_size);
      }
    }
    return result;
  }
  bool makeSegmentHeader(ttLibC_Frame *frame, dataCallback callback) {
    if(!initDone) {
      return true;
    }
    if(frame->pts > writer->pts + writer->unitDuration) {
      writer->pts = frame->pts;
      uint8_t buf[256];
      unique_ptr<ByteConnector> connector(ByteConnector::create(buf, 256, ByteUtilType_default));
      connector->ebml(MkvType_Cluster, true)
               ->bit(0x01FFFFFF, 32)
               ->bit(0xFFFFFFFF, 32);
      connector->ebml(MkvType_Timecode, true)
               ->ebml(8, false)
               ->bit((writer->pts >> 32), 32)
               ->bit(writer->pts, 32);
      return callback(buf, connector->writeSize);
    }
    return true;
  }
private:
  uint32_t channelNum;
  uint32_t sampleRate;
};

class MkvWriter_ : public MkvWriter {
public:
  MkvWriter_(vector<ttLibC_Frame_Type> &types) : MkvWriter(), isHeaderWritten(false) {
    uint32_t id = 0;
    for(auto type : types) {
      id ++;
      switch(type) {
      case frameType_h264:
        tracks.insert(pair<const uint32_t, MkvTrack *>(id, new H264Track(this, id)));
        break;
      case frameType_h265:
        tracks.insert(pair<const uint32_t, MkvTrack *>(id, new H265Track(this, id)));
        break;
      case frameType_aac2:
        tracks.insert(pair<const uint32_t, MkvTrack *>(id, new AacTrack(this, id)));
        break;
      case frameType_mp3:
        tracks.insert(pair<const uint32_t, MkvTrack *>(id, new Mp3Track(this, id)));
        break;
      default:
        break;
      }
    }
  }
  ~MkvWriter_() {
    for(auto trackPair : tracks) {
      delete trackPair.second;
    }
  }
  bool write(ttLibC_Frame *frame, dataCallback callback) {
    bool result = true;
    if(!isHeaderWritten) {
      bool ready = true;
      for(auto trackPair : tracks) {
        if(!trackPair.second->isReady) {
          ready = false;
        }
      }
      if(ready) {
        unique_ptr<DynamicBuffer> buffer(DynamicBuffer::create());
        uint8_t buf[256];
        size_t in_size;
        // EBML
    		in_size = ttLibC_HexUtil_makeBuffer("1A 45 DF A3 A3 42 86 81 01 42 F7 81 01 42 F2 81 04 42 F3 81 08 42 82 88 6D 61 74 72 6F 73 6B 61 42 87 81 04 42 85 81 02", buf, 256);
        buffer->append(buf, in_size);
        // Segment
      	in_size = ttLibC_HexUtil_makeBuffer("18 53 80 67 01 FF FF FF FF FF FF FF", buf, 256);
	      buffer->append(buf, in_size);
        // Info
      	in_size = ttLibC_HexUtil_makeBuffer("15 49 A9 66 99 2A D7 B1 83 0F 42 40 4D 80 86 74 74 4C 69 62 43 57 41 86 74 74 4C 69 62 43", buf, 256);
	      buffer->append(buf, in_size);
        if(!callback(buffer->refData(), buffer->refSize())) {
          return false;
        }

        // tracks
        unique_ptr<DynamicBuffer> tracksBuffer(DynamicBuffer::create());
        for(uint32_t i = 0, size = tracks.size(); i < size; ++ i) {
          tracks[i + 1]->writeInit([&](void *data, size_t dataSize){
            tracksBuffer->append(reinterpret_cast<uint8_t *>(data), dataSize);
            return true;
          });
        }
        unique_ptr<ByteConnector> connector(ByteConnector::create(buf, 256, ByteUtilType_default));
        connector->ebml(MkvType_Tracks, true)
                 ->ebml(tracksBuffer->refSize(), false);
        if(!callback(buf, connector->writeSize)) {
          return false;
        }
        if(!callback(tracksBuffer->refData(), tracksBuffer->refSize())) {
          return false;
        }

        // segment entry.
        in_size = ttLibC_HexUtil_makeBuffer("1F 43 B6 75 01 FF FF FF FF FF FF FF E7 81 00", buf, 256);
        if(!callback(buf, in_size)) {
          return false;
        }
        isHeaderWritten = true;
      }
    }
    uint64_t originalPts = frame->pts;
    uint32_t originalTimebase = frame->timebase;
    frame->pts = (uint64_t)(1.0 * frame->pts * timebase / frame->timebase);
    frame->timebase = timebase;
    if(result) {
      result = tracks[1]->makeSegmentHeader(frame, callback);
    }
    if(result) {
      result = doTrack(frame->id, [&](MkvTrack *track){
        return track->append(frame, callback);
      });
    }
    frame->pts = originalPts;
    frame->timebase = originalTimebase;
    return result;
  }
private:
  bool doTrack(uint32_t id, std::function<bool(MkvTrack *track)> callback) {
    if(tracks[id] != nullptr) {
      return callback(tracks[id]);
    }
    return false;
  }
  map<uint32_t, MkvTrack *>tracks;
  bool isHeaderWritten;
};

MkvWriter::MkvWriter() : pts(0), timebase(1000), unitDuration(5000), mode(0) {}
MkvWriter::~MkvWriter() {}
MkvWriter *MkvWriter::create(vector<ttLibC_Frame_Type> types) {
  return new MkvWriter_(types);
}
MkvWriter *MkvWriter::create(ttLibC_Frame_Type videoType, ttLibC_Frame_Type audioType) {
  std::vector<ttLibC_Frame_Type> vec(videoType, audioType);
  return new MkvWriter_(vec);
}

}

