/*
 * @file   mkvReader.c
 * @brief  mkv container reader.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/04
 */

#include "mkvReader.h"
#include <string.h>

#include "../../_log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"
#include "../../util/ioUtil.h"
#include "../../util/byteUtil.h"

static bool MkvReader_closeTrack(void *ptr, void *key, void *item);

ttLibC_MkvReader *ttLibC_MkvReader_make() {
	ttLibC_MkvReader_ *reader = (ttLibC_MkvReader_ *)ttLibC_ContainerReader_make(
			containerType_mkv,
			sizeof(ttLibC_MkvReader_));
	if(reader == NULL) {
		return NULL;
	}
	reader->error_number = 0;
	reader->pts = 0;
	reader->timebase = 1000;
	reader->track = NULL;
	reader->tmp_buffer = ttLibC_DynamicBuffer_make();
	reader->tracks = ttLibC_StlMap_make();
	reader->in_reading = false;
	reader->tag = NULL;
	return (ttLibC_MkvReader *)reader;
}

static bool MkvReader_readTag(
		ttLibC_MkvReader_ *reader,
		ttLibC_MkvReadFunc callback,
		void *ptr) {
	uint8_t *data = ttLibC_DynamicBuffer_refData(reader->tmp_buffer);
	size_t data_size = ttLibC_DynamicBuffer_refSize(reader->tmp_buffer);
	if(data_size == 0) {
		return false;
	}
	ttLibC_ByteReader *byte_reader = ttLibC_ByteReader_make(
			data,
			data_size,
			ByteUtilType_default);
	uint32_t type = ttLibC_ByteReader_ebml(byte_reader, true);
	uint64_t size = ttLibC_ByteReader_ebml(byte_reader, false);
	if(byte_reader->error_number != 0) {
		ttLibC_ByteReader_close(&byte_reader);
		return false;
	}
	switch(type) {
	case MkvType_BlockGroup:
	case MkvType_TrackEntry:
	case MkvType_Segment:
	case MkvType_Info:
	case MkvType_Tracks:
	case MkvType_Cluster:
		{
			if(type == MkvType_TrackEntry) {
				// make new track
				reader->track = ttLibC_malloc(sizeof(ttLibC_MkvTrack));
				memset(reader->track, 0, sizeof(ttLibC_MkvTrack));
				reader->track->track_number = -1;
				reader->track->size = size + byte_reader->read_size;
			}
		}
		/* no break */
	case MkvType_Video:
	case MkvType_Audio:
		{
			ttLibC_MkvTag mkvTag;
			mkvTag.inherit_super.type = type;
			mkvTag.inherit_super.inherit_super.data = NULL;
			mkvTag.inherit_super.inherit_super.data_size = size + byte_reader->read_size;
			mkvTag.inherit_super.inherit_super.buffer_size = size;
			mkvTag.inherit_super.inherit_super.pts = reader->pts;
			mkvTag.inherit_super.inherit_super.timebase = reader->timebase;
			if(callback != NULL) {
				if(!callback(ptr, (ttLibC_Mkv *)&mkvTag)) {
					reader->error_number = 4;
				}
			}
		}
		break;
	case MkvType_DiscardPadding:
	case MkvType_Block:
	case MkvType_SeekPreRoll:
	case MkvType_CodecDelay:
	case MkvType_FlagInterlaced:
	case MkvType_CodecName:
	case MkvType_FrameRate:
	case MkvType_Cues:
	case MkvType_SimpleBlock:
	case MkvType_Timecode:
	case MkvType_Tags:
	case MkvType_BitDepth:
	case MkvType_Channels:
	case MkvType_SamplingFrequency:
	case MkvType_CodecPrivate:
	case MkvType_DisplayWidth:
	case MkvType_DisplayHeight:
	case MkvType_PixelWidth:
	case MkvType_PixelHeight:
	case MkvType_DefaultDuration:
	case MkvType_TrackType:
	case MkvType_CodecID:
	case MkvType_Language:
	case MkvType_FlagLacing:
	case MkvType_TrackUID:
	case MkvType_TrackNumber:
	case MkvType_Duration:
	case MkvType_SegmentUID:
	case MkvType_MuxingApp:
	case MkvType_WritingApp:
	case MkvType_TimecodeScale:
	case MkvType_Void:
	case MkvType_SeekHead:
	case MkvType_EBML:
	case MkvType_CRC32:
	case MkvType_DisplayUnit:
	case MkvType_Colour:
	case MkvType_MaxBlockAdditionID:
	case MkvType_AlphaMode:
		{
			// check the data size.
			if(data_size < size + byte_reader->read_size) {
				// need more.
				ttLibC_ByteReader_close(&byte_reader);
				return false;
			}
			ttLibC_MkvTag *tag = ttLibC_MkvTag_make(
					reader->tag,
					data,
					size + byte_reader->read_size,
					true,
					reader->pts,
					reader->timebase,
					type);
			if(tag == NULL) {
				reader->error_number = 2;
				ttLibC_ByteReader_close(&byte_reader);
				return false;
			}
			switch(type) {
/*			case MkvType_CodecDelay:
				{
					uint32_t delay = ttLibC_ByteReader_bit(byte_reader, size * 8);
//					LOG_PRINT("delay:%d", delay); // should I consider this? do later.
				}
				break;*/
			case MkvType_TimecodeScale:
				{
					uint32_t timescale = ttLibC_ByteReader_bit(byte_reader, size * 8);
					reader->timebase = timescale / 1000;
				}
				break;
			case MkvType_Timecode:
				{
					uint64_t timecode = ttLibC_ByteReader_bit(byte_reader, size * 8);
					reader->pts = timecode;
				}
				break;
			case MkvType_TrackNumber:
				{
					uint32_t trackNumber = ttLibC_ByteReader_bit(byte_reader, size * 8);
					if(reader->track == NULL) {
						ERR_PRINT("got trackNumber ebml outside of trackEntry.");
					}
					else {
						reader->track->track_number = trackNumber;
					}
				}
				break;
			case MkvType_FlagLacing:
				{
					uint32_t flagLacing = ttLibC_ByteReader_bit(byte_reader, size * 8);
					if(reader->track == NULL) {
						ERR_PRINT("got flagLacing ebml outside of trackEntry.");
					}
					else {
						reader->track->is_lacing = flagLacing != 0;
					}
				}
				break;
			case MkvType_PixelWidth:
				{
					uint32_t width = ttLibC_ByteReader_bit(byte_reader, size * 8);
					if(reader->track == NULL) {
						ERR_PRINT("got pixel width ebml outside of trackEntry.");
					}
					else {
						reader->track->width = width;
					}
				}
				break;
			case MkvType_PixelHeight:
				{
					uint32_t height = ttLibC_ByteReader_bit(byte_reader, size * 8);
					if(reader->track == NULL) {
						ERR_PRINT("got pixel height ebml outside of trackEntry.");
					}
					else {
						reader->track->height = height;
					}
				}
				break;
			case MkvType_Channels:
				{
					uint32_t channel_num = ttLibC_ByteReader_bit(byte_reader, size * 8);
					if(reader->track == NULL) {
						ERR_PRINT("got channels ebml outside of trackEntry.");
					}
					else {
						reader->track->channel_num = channel_num;
					}
				}
				break;
			case MkvType_SamplingFrequency:
				{
					uint64_t sample_rate_byte = ttLibC_ByteReader_bit(byte_reader, size * 8);
					if(reader->track == NULL) {
						ERR_PRINT("got sample_rate ebml outside of trackEntry.");
					}
					else {
						if(size == 4) {
							float sample_rate;
							uint32_t sample_rate_byte_32 = sample_rate_byte;
							memcpy(&sample_rate, &sample_rate_byte_32, 4);
							reader->track->sample_rate = (uint32_t)sample_rate;
						}
						else if(size == 8) {
							double sample_rate;
							memcpy(&sample_rate, &sample_rate_byte, 8);
							reader->track->sample_rate = (uint32_t)sample_rate;
						}
						else {
							ERR_PRINT("unexpected size of float tag.");
						}
					}
				}
				break;
			case MkvType_CodecID:
				{
					char codecId[256];
					ttLibC_ByteReader_string(byte_reader, codecId, 256, size);
					if(reader->track == NULL) {
						ERR_PRINT("got codecID ebml outsize of trackEntry.");
					}
					else {
						if(strstr(codecId, "AVC") != 0) {
							reader->track->type = frameType_h264;
						}
						else if(strstr(codecId, "HEVC") != 0) {
							reader->track->type = frameType_h265;
						}
						else if(strcmp(codecId, "V_VP8") == 0) {
							reader->track->type = frameType_vp8;
						}
						else if(strcmp(codecId, "V_VP9") == 0) {
							reader->track->type = frameType_vp9;
						}
						else if(strcmp(codecId, "V_THEORA") == 0) {
							reader->track->type = frameType_theora;
						}
						else if(strcmp(codecId, "V_MJPEG") == 0) {
							reader->track->type = frameType_jpeg;
						}
						else if(strcmp(codecId, "A_OPUS") == 0) {
							reader->track->type = frameType_opus;
						}
						else if(strcmp(codecId, "A_VORBIS") == 0) {
							reader->track->type = frameType_vorbis;
						}
						else if(strstr(codecId, "A_AAC") != 0) {
							reader->track->type = frameType_aac;
						}
						else if(strcmp(codecId, "A_MPEG/L3") == 0) {
							reader->track->type = frameType_mp3;
 						}
						else if(strcmp(codecId, "A_MS/ACM") == 0){
							reader->track->type = frameType_unknown;
						}
						else if(strcmp(codecId, "A_PCM/INT/LIT") == 0) {
							// A_PCM/INT/BIG (for big endian)
							reader->track->type = frameType_pcmS16;
						}
						else {
							ERR_PRINT("unknown codec Id:%s", codecId);
						}
					}
				}
				break;
			case MkvType_CodecPrivate:
				{
					if(reader->track == NULL) {
						ERR_PRINT("got codecPrivate outside of trackentry.");
					}
					else {
						// get data as string.
						uint8_t *private_data = ttLibC_malloc(size + 1);
						if(private_data == NULL) {
							ERR_PRINT("failed to alloc memory for private data.");
						}
						else {
							ttLibC_ByteReader_string(byte_reader, (char *)private_data, size + 1, size);
							if(byte_reader->error_number != 0) {
								ERR_PRINT("got error on byte reader.");
								ttLibC_free(private_data);
							}
							else {
								reader->track->private_data = private_data;
								reader->track->private_data_size = size;
							}
						}
					}
				}
				break;
			case MkvType_BitDepth: // if we deal with pcm, need bitDepth.
			default:
				ttLibC_ByteReader_skipByte(byte_reader, size);
				break;
			}
			reader->tag = tag;
			reader->tag->reader = (ttLibC_MkvReader *)reader;
			if(callback != NULL) {
				if(!callback(ptr, (ttLibC_Mkv *)reader->tag)) {
					reader->error_number = 3;
				}
			}
		}
		break;
	default:
		ERR_PRINT("unknown tag.:%x", type);
		LOG_DUMP(data, data_size, true);
		reader->error_number = 1;
		break;
	}
	if(reader->error_number == 0) {
		reader->error_number = byte_reader->error_number;
	}
	if(reader->error_number == 0) {
		if(reader->track != NULL) {
			reader->track->size -= byte_reader->read_size;
			if(reader->track->size == 0) {
				// if we have old track.
				if(reader->track->track_number == -1) {
					// track is broken. dispose.
					MkvReader_closeTrack(NULL, NULL, reader->track);
				}
				else {
					// put into track map
					ttLibC_StlMap_put(reader->tracks, (void *)(long)reader->track->track_number, reader->track);
					reader->track = NULL;
				}
			}
		}
		ttLibC_DynamicBuffer_markAsRead(reader->tmp_buffer, byte_reader->read_size);
	}
	ttLibC_ByteReader_close(&byte_reader);
	return reader->error_number == 0;
}

bool ttLibC_MkvReader_read(
		ttLibC_MkvReader *reader,
		void *data,
		size_t data_size,
		ttLibC_MkvReadFunc callback,
		void *ptr) {
	ttLibC_MkvReader_ *reader_ = (ttLibC_MkvReader_ *)reader;
	ttLibC_DynamicBuffer_append(reader_->tmp_buffer, data, data_size);
	if(reader_->in_reading) {
		return true;
	}
	reader_->in_reading = true;
	while(MkvReader_readTag(reader_, callback, ptr)) {
		;
	}
	ttLibC_DynamicBuffer_clear(reader_->tmp_buffer);
	reader_->in_reading = false;
	return reader_->error_number == 0;
}

static bool MkvReader_closeTrack(void *ptr, void *key, void *item) {
	(void)ptr;
	(void)key;
	ttLibC_MkvTrack *track = (ttLibC_MkvTrack *)item;
	if(track != NULL) {
		ttLibC_Frame_close(&track->frame);
		if(track->private_data) {
			ttLibC_free(track->private_data);
			track->private_data = NULL;
		}
		ttLibC_free(track);
	}
	return true;
}

void ttLibC_MkvReader_close(ttLibC_MkvReader **reader) {
	ttLibC_MkvReader_ *target = (ttLibC_MkvReader_ *)*reader;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mkv) {
		ERR_PRINT("this reader is not mkv reader.");
		return;
	}
	ttLibC_StlMap_forEach(target->tracks, MkvReader_closeTrack, NULL);
	ttLibC_StlMap_close(&target->tracks);
	if(target->track != NULL) {
		MkvReader_closeTrack(NULL, NULL, target->track);
		target->track = NULL;
	}
	ttLibC_MkvTag_close(&target->tag);
	ttLibC_DynamicBuffer_close(&target->tmp_buffer);
	ttLibC_free(target);
	*reader = NULL;
}
