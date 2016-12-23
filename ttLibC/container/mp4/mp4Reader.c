/**
 * @file   mp4Reader.c
 * @brief  mp4 container reader.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/03
 */

#include "mp4Reader.h"
#include <string.h>

#include "../../log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"
#include "../../util/ioUtil.h"
#include "../../util/byteUtil.h"
#include "type/ctts.h"
#include "type/stco.h"
#include "type/stsc.h"
#include "type/stsz.h"
#include "type/stts.h"
#include "type/trun.h"
#include "type/elst.h"

static bool Mp4Reader_closeTrack(void *ptr, void *key, void *item);

ttLibC_Mp4Reader *ttLibC_Mp4Reader_make() {
	ttLibC_Mp4Reader_ *reader = (ttLibC_Mp4Reader_ *)ttLibC_ContainerReader_make(
			containerType_mp4,
			sizeof(ttLibC_Mp4Reader_));
	if(reader == NULL) {
		return NULL;
	}
	reader->error_number = 0;
	reader->tracks = ttLibC_StlMap_make();
	reader->track = NULL;
	reader->tmp_buffer = ttLibC_DynamicBuffer_make();
	reader->mdat_buffer = NULL;
	reader->mdat_start_pos = 0;
	reader->in_reading = false;
	reader->atom = NULL;
	reader->timebase = 0;
	reader->duration = 0;
	reader->callback = NULL;
	reader->mvex = NULL;
	reader->ptr = NULL;
	reader->is_fmp4 = false;
	reader->position = 0;
	return (ttLibC_Mp4Reader *)reader;
}

static bool Mp4Reader_analyzeMvex(ttLibC_Mp4Reader_ *reader) {
	if(reader->mvex == NULL) {
		// 1.get mvex, put it on reader.
		// 2.1st moof, analyzeMvex. reader->mvex will be NULL.
		// 3.every moof, call this.
		return true;
	}
	uint8_t *data = reader->mvex->inherit_super.data;
	size_t data_size = reader->mvex->inherit_super.buffer_size;
	ttLibC_ByteReader *byte_reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	do {
		uint32_t size = ttLibC_ByteReader_bit(byte_reader, 32);
		uint32_t tag = ttLibC_ByteReader_bit(byte_reader, 32);
		switch(tag) {
		case Mp4Type_Mvex:
			break;
		case Mp4Type_Trex:
			{
				ttLibC_ByteReader_skipByte(byte_reader, 4); // version & flags
				uint32_t track_id = ttLibC_ByteReader_bit(byte_reader, 32);
				uint32_t default_sample_description_index = ttLibC_ByteReader_bit(byte_reader, 32);
				uint32_t default_sample_duration = ttLibC_ByteReader_bit(byte_reader, 32);
				uint32_t default_sample_size = ttLibC_ByteReader_bit(byte_reader, 32);
				uint32_t default_sample_flags = ttLibC_ByteReader_bit(byte_reader, 32);
				ttLibC_Mp4Track *track = ttLibC_StlMap_get(reader->tracks, (void *)(long)track_id);
				if(track == NULL) {
					ERR_PRINT("failed to ref track object.");
					return false;
				}
				track->trex_sample_desription_index = default_sample_description_index;
				track->trex_sample_duration         = default_sample_duration;
				track->trex_sample_size             = default_sample_size;
				track->trex_sample_flags            = default_sample_flags;
			}
			break;
		case Mp4Type_Mehd:
		case Mp4Type_Trep:
		default:
			{
				ttLibC_ByteReader_skipByte(byte_reader, size - 8);
			}
			break;
		}
	} while(byte_reader->read_size < data_size);
	ttLibC_ByteReader_close(&byte_reader);
	ttLibC_Mp4Atom_close((ttLibC_Mp4Atom **)&reader->mvex);
	return true;
}

static bool Mp4Reader_readAtom(
		ttLibC_Mp4Reader_ *reader,
		ttLibC_Mp4ReadFunc callback,
		void *ptr) {
	uint8_t *data = ttLibC_DynamicBuffer_refData(reader->tmp_buffer);
	size_t data_size = ttLibC_DynamicBuffer_refSize(reader->tmp_buffer);
	if(data_size == 0) {
		return false;
	}
	ttLibC_ByteReader *byte_reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	uint32_t move_size = 8;
	uint32_t size = ttLibC_ByteReader_bit(byte_reader, 32);
	uint32_t tag  = ttLibC_ByteReader_bit(byte_reader, 32);
	if(size == 0) {
		ERR_PRINT("invalid mp4 data. atom size is 0.");
		ttLibC_ByteReader_close(&byte_reader);
		return false;
	}
	if(tag != Mp4Type_Mdat && data_size < size) {
		ttLibC_ByteReader_close(&byte_reader);
		return false;
	}
	switch(tag) {
	case Mp4Type_Sidx:
	case Mp4Type_Styp:
	case Mp4Type_Ftyp:
	case Mp4Type_Free:
	case Mp4Type_Mdat:
	case Mp4Type_Traf:
		{
			switch(tag) {
			case Mp4Type_Mdat:
				// mdat could be too big. so do callback with incomplete data.
				// only for mdat.
				{
					reader->mdat_start_pos = reader->position;
					if(data_size < size) {
						ttLibC_ByteReader_close(&byte_reader);
						ttLibC_Mp4Atom *mp4Atom = ttLibC_Mp4Atom_make(
								reader->atom,
								data,
								data_size,
								true,
								0,
								reader->timebase,
								tag);
						if(mp4Atom != NULL) {
							mp4Atom->position = reader->position;
							mp4Atom->inherit_super.is_complete = false;
							mp4Atom->reader = (ttLibC_Mp4Reader *)reader;
							reader->atom = mp4Atom;
							if(callback != NULL) {
								if(!callback(ptr, (ttLibC_Mp4 *)mp4Atom)) {
									reader->error_number = 1;
								}
							}
						}
						return false;
					}
					if(reader->tracks->size == 0) {
						if(reader->mdat_buffer == NULL) {
							reader->mdat_buffer = ttLibC_DynamicBuffer_make();
						}
						ttLibC_DynamicBuffer_append(reader->mdat_buffer, data, data_size);
					}
				}
				break;
			case Mp4Type_Traf:
				{
					// analyze traf at once. for the case of tfhd is not 1st.
					ttLibC_Mp4Track *track = NULL; // target track
					uint64_t decode_time_duration = 0; // from tfdt
					// trun will contain the data of traf, for analyzing trun, we need to check the data in trafAtom.(because, traf can hold multiple trunAtoms.)
					ttLibC_Mp4 *trun = ttLibC_Trun_make(data + 8, size - 8);
					do {
						uint32_t in_size = ttLibC_ByteReader_bit(byte_reader, 32);
						uint32_t in_tag  = ttLibC_ByteReader_bit(byte_reader, 32);
						uint8_t  version = ttLibC_ByteReader_bit(byte_reader, 8);
						uint32_t flags   = ttLibC_ByteReader_bit(byte_reader, 24);
						switch(in_tag) {
						case Mp4Type_Tfhd:
							{
								// get track object by track_id
								uint32_t track_id = ttLibC_ByteReader_bit(byte_reader, 32);
								track = ttLibC_StlMap_get(reader->tracks, (void *)(long)track_id);
								if(decode_time_duration == 0) {
									decode_time_duration = track->decode_time_duration;
								}
								// default information
								track->tfhd_base_data_offset = 0;
								track->tfhd_sample_desription_index = 0;
								track->tfhd_sample_duration = 0;
								track->tfhd_sample_flags = 0;
								track->tfhd_sample_size = 0;
								if((flags & 0x000001) != 0) {
									track->tfhd_base_data_offset = ttLibC_ByteReader_bit(byte_reader, 64);
								}
								if((flags & 0x000002) != 0) {
									track->tfhd_sample_desription_index = ttLibC_ByteReader_bit(byte_reader, 32);
								}
								if((flags & 0x000008) != 0) {
									track->tfhd_sample_duration = ttLibC_ByteReader_bit(byte_reader, 32);
								}
								if((flags & 0x000010) != 0) {
									track->tfhd_sample_size = ttLibC_ByteReader_bit(byte_reader, 32);
								}
								if((flags & 0x000020) != 0) {
									track->tfhd_sample_flags = ttLibC_ByteReader_bit(byte_reader, 32);
								}
								if((flags & 0x010000) != 0) {
									// I don't know how to deal this. later.
									ERR_PRINT("unexpected flags for tfhd.");
									reader->error_number = 10;
								}
							}
							break;
						case Mp4Type_Tfdt:
							{
								if(version == 0) {
									decode_time_duration = ttLibC_ByteReader_bit(byte_reader, 32);
								}
								else {
									decode_time_duration = ttLibC_ByteReader_bit(byte_reader, 64);
								}
							}
							break;
						case Mp4Type_Trun:
							{
								ttLibC_ByteReader_skipByte(byte_reader, in_size - 12);
							}
							break;
						default:
							LOG_PRINT("sz:%x tag:%x", in_size, in_tag);
							reader->error_number = 6;
							ttLibC_ByteReader_skipByte(byte_reader, in_size - 12); // CHECK is this corrent?
							break;
						}
					} while(byte_reader->read_size < size);
					// byte reading is done.
					if(track == NULL || trun == NULL) {
						ttLibC_Mp4Atom_close((ttLibC_Mp4Atom **)&trun);
						reader->error_number = 6;
					}
					else {
						ttLibC_Mp4Atom_close((ttLibC_Mp4Atom **)&track->trun);
						track->trun = trun;
						track->decode_time_duration = decode_time_duration;
						ttLibC_Trun_setTrack(track->trun, track);
					}
				}
				break;
			default:
				break;
			}
			reader->position += size;
			move_size = size;
		}
		break;
	case Mp4Type_Edts:
	case Mp4Type_Moof:
	case Mp4Type_Moov:
	case Mp4Type_Trak:
	case Mp4Type_Mdia:
	case Mp4Type_Minf:
	case Mp4Type_Stbl:
		{
			switch(tag) {
			case Mp4Type_Trak:
				if(reader->track != NULL) {
					ERR_PRINT("prev track object is remaining, something wrong.");
					Mp4Reader_closeTrack(NULL, NULL, reader->track);
					reader->track = NULL;
				}
				// make new track object.
				reader->track = ttLibC_malloc(sizeof(ttLibC_Mp4Track));
				if(reader->track != NULL) {
					memset(reader->track, 0, sizeof(ttLibC_Mp4Track));
					reader->track->track_number = -1;
					reader->track->size = size;
				}
				break;
			case Mp4Type_Moof:
				{
					reader->moof_position = reader->position;
					if(!Mp4Reader_analyzeMvex(reader)) {
						reader->error_number = 5;
						return false;
					}
				}
				break;
			default:
				break;
			}
		}
		reader->position += 8;
		move_size = 8;
		break;
	case Mp4Type_Elst:
	case Mp4Type_Udta:
	case Mp4Type_Mvhd:
	case Mp4Type_Iods:
	case Mp4Type_Tkhd:
	case Mp4Type_Mdhd:
	case Mp4Type_Hdlr:
	case Mp4Type_Vmhd:
	case Mp4Type_Smhd:
	case Mp4Type_Dinf:
	case Mp4Type_Stsd:
	case Mp4Type_Stts:
	case Mp4Type_Stss:
	case Mp4Type_Stsc:
	case Mp4Type_Stsz:
	case Mp4Type_Stco:
	case Mp4Type_Mvex:
	case Mp4Type_Mfhd: // for html5 mediaSource, sequence number = 0 is works.
	case Mp4Type_Ctts:
		{
			uint8_t version = ttLibC_ByteReader_bit(byte_reader, 8);
			uint32_t flags = ttLibC_ByteReader_bit(byte_reader, 24);
			switch(tag) {
			case Mp4Type_Elst:
				{
					if(reader->track == NULL) {
						reader->error_number = 1;
					}
					else {
						reader->track->elst = ttLibC_Elst_make(data, size, reader->track->timebase);
						if(reader->track->elst == NULL) {
							reader->error_number = 1;
						}
					}
/*					uint32_t count;
					uint64_t segmentDuration;
					uint64_t mediaTime;
					uint16_t mediaRateInt;
					uint16_t mediaRateFraction;
					count = ttLibC_ByteReader_bit(byte_reader, 32);
					if(version == 0) {
						segmentDuration = ttLibC_ByteReader_bit(byte_reader, 32);
						mediaTime = ttLibC_ByteReader_bit(byte_reader, 32);
					}
					else {
						ERR_PRINT("warn:64bit for elst.");
						segmentDuration = ttLibC_ByteReader_bit(byte_reader, 64);
						mediaTime = ttLibC_ByteReader_bit(byte_reader, 64);
					}
					mediaRateInt = ttLibC_ByteReader_bit(byte_reader, 16);
					mediaRateFraction = ttLibC_ByteReader_bit(byte_reader, 16);
					if(mediaRateInt != 1 && mediaRateFraction != 0) {
						ERR_PRINT("warn:rate is out of my thought. need to check.");
					}
					if(reader->track != NULL) {
						reader->track->elst_mediatime = (uint32_t)mediaTime;
					}
					*/
				}
				break;
			case Mp4Type_Mvhd:
				{
					uint32_t timebase;
					uint64_t duration;
					if(version == 0) {
						ttLibC_ByteReader_bit(byte_reader, 32);
						ttLibC_ByteReader_bit(byte_reader, 32);
						timebase = ttLibC_ByteReader_bit(byte_reader, 32);
						duration = ttLibC_ByteReader_bit(byte_reader, 32);
					}
					else {
						ttLibC_ByteReader_bit(byte_reader, 64);
						ttLibC_ByteReader_bit(byte_reader, 64);
						timebase = ttLibC_ByteReader_bit(byte_reader, 32);
						duration = ttLibC_ByteReader_bit(byte_reader, 64);
					}
					reader->timebase = timebase;
					reader->duration = duration;
				}
				break;
			case Mp4Type_Iods:
				/*
				 * extra tag information?
				 * from MP4Box
				 * 0x10:tag type(MP4_IOD_Tag?)
				 * 0x07:size
				 * 0x004F:ObjectDescripterID = 1
				 * 0xFF:ODProfileLevel
				 * 0xFF:SceneProfileLevel
				 * 0x29:AudioProfileLevel
				 * 0xFE:VideoProfileLevel
				 * 0xFF:graphicProfileLevel
				 */
				break;
			case Mp4Type_Tkhd:
				{
					uint64_t duration;
					uint32_t track_id;
					uint32_t width, height;
					if(version == 0) {
						ttLibC_ByteReader_bit(byte_reader, 32);
						ttLibC_ByteReader_bit(byte_reader, 32);
						track_id = ttLibC_ByteReader_bit(byte_reader, 32);
						ttLibC_ByteReader_bit(byte_reader, 32);
						duration = ttLibC_ByteReader_bit(byte_reader, 32);
					}
					else {
						ttLibC_ByteReader_bit(byte_reader, 64);
						ttLibC_ByteReader_bit(byte_reader, 64);
						track_id = ttLibC_ByteReader_bit(byte_reader, 32);
						ttLibC_ByteReader_bit(byte_reader, 32);
						duration = ttLibC_ByteReader_bit(byte_reader, 64);
					}
					ttLibC_ByteReader_skipByte(byte_reader, 52);
					width = ttLibC_ByteReader_bit(byte_reader, 16);
					ttLibC_ByteReader_bit(byte_reader, 16);
					height = ttLibC_ByteReader_bit(byte_reader, 16);
					ttLibC_ByteReader_bit(byte_reader, 16);
					if(reader->track != NULL) {
						reader->track->track_number = track_id;
						reader->track->width = width;
						reader->track->height = height;
					}
				}
				break;
			case Mp4Type_Mdhd:
				{
					uint32_t timebase;
					uint64_t duration;
					if(version == 0) {
						ttLibC_ByteReader_bit(byte_reader, 32);
						ttLibC_ByteReader_bit(byte_reader, 32);
						timebase = ttLibC_ByteReader_bit(byte_reader, 32);
						duration = ttLibC_ByteReader_bit(byte_reader, 32);
					}
					else {
						ttLibC_ByteReader_bit(byte_reader, 64);
						ttLibC_ByteReader_bit(byte_reader, 64);
						timebase = ttLibC_ByteReader_bit(byte_reader, 32);
						duration = ttLibC_ByteReader_bit(byte_reader, 64);
					}
					if(reader->track != NULL) {
						reader->track->duration = duration;
						reader->track->timebase = timebase;
					}
				}
				break;
			case Mp4Type_Hdlr:
				{
					uint32_t type;
					ttLibC_ByteReader_skipByte(byte_reader, 4);
					type = ttLibC_ByteReader_bit(byte_reader, 32);
					switch(type) {
					case 'vide':
						if(reader->track != NULL) {
							reader->track->is_video = true;
						}
						break;
					case 'soun':
						if(reader->track != NULL) {
							reader->track->is_audio = true;
						}
						break;
					default:
						LOG_PRINT("unknown type of media, subtitle?");
						break;
					}
				}
				break;
			case Mp4Type_Stsd:
				{
					uint32_t entry_count = ttLibC_ByteReader_bit(byte_reader, 32);
					if(entry_count != 1) {
						LOG_PRINT("only 1 is expected.");
					}
					uint32_t in_size = ttLibC_ByteReader_bit(byte_reader, 32);
					uint32_t in_tag = ttLibC_ByteReader_bit(byte_reader, 32);
					switch(in_tag) {
					case 'avc1':
						reader->track->frame_type = frameType_h264;
						break;
					case 'hev1':
						reader->track->frame_type = frameType_h265;
						break;
					case 'mp4v':
					case 'mp4a':
						if(reader->track->is_video) {
							ttLibC_ByteReader_skipByte(byte_reader, 24);
							uint16_t width  = ttLibC_ByteReader_bit(byte_reader, 16);
							uint16_t height = ttLibC_ByteReader_bit(byte_reader, 16);
							ttLibC_ByteReader_skipByte(byte_reader, 4);
							ttLibC_ByteReader_skipByte(byte_reader, 4);
							ttLibC_ByteReader_skipByte(byte_reader, 4);
							ttLibC_ByteReader_skipByte(byte_reader, 2);
							ttLibC_ByteReader_skipByte(byte_reader, 32);
							ttLibC_ByteReader_skipByte(byte_reader, 2);
							ttLibC_ByteReader_skipByte(byte_reader, 2);
						}
						else if(reader->track->is_audio) {
							// need to check esTag to understand frame type.
							// for audio need to get sampleRate and channelNum
							ttLibC_ByteReader_skipByte(byte_reader, 16);
							uint32_t channel_num = ttLibC_ByteReader_bit(byte_reader, 16);
							ttLibC_ByteReader_skipByte(byte_reader, 6);
							uint32_t sample_rate = ttLibC_ByteReader_bit(byte_reader, 16);
							ttLibC_ByteReader_skipByte(byte_reader, 2);
							if(reader->track != NULL) {
								reader->track->channel_num = channel_num;
								reader->track->sample_rate = sample_rate;
							}
						}
						in_size = ttLibC_ByteReader_bit(byte_reader, 32);
						in_tag = ttLibC_ByteReader_bit(byte_reader, 32);
						if(in_tag == 'esds') {
							ttLibC_ByteReader_skipByte(byte_reader, 4);
							while(true) {
								// ---
								uint32_t esTagType = ttLibC_ByteReader_bit(byte_reader, 8);
								// 3:esTag 4:decoderConfig 5:DecoderSpecific 6:SlConfig
								// if first bit is on, continue to next byte. for size.
								uint32_t esTagSize = 0;
								bool is_continue;
								do {
									is_continue = ttLibC_ByteReader_bit(byte_reader, 1) == 0x01;
									esTagSize = esTagSize << 7 | ttLibC_ByteReader_bit(byte_reader, 7);
								} while(is_continue);
								if(esTagType == 3) { // esTag
									ttLibC_ByteReader_skipByte(byte_reader, 3);
								}
								else if(esTagType == 4) { // decoderConfig
									uint8_t frameType = ttLibC_ByteReader_bit(byte_reader, 8);
									/* list.
									 * 0x01:SystemV1
									 * 0x02:SystemV2
									 * 0x20:Mpeg4Video
									 * 0x21:Mpeg4AvcSps
									 * 0x22:Mpeg4AvcPps
									 * 0x40:Mpeg4Audio(aac?) *
									 * 0x60:Mpeg2SimpleVideo
									 * 0x61:Mpeg2MainVideo
									 * 0x62:Mpeg2SnrVideo
									 * 0x63:Mpeg2SpecialVideo
									 * 0x64:Mpeg2HighVideo
									 * 0x65:Mpeg2_422Video
									 * 0x66:Mpeg4AdtsMain
									 * 0x67:Mpeg4AdtsLowComplexity
									 * 0x68:Mpeg4AdtsScalableSamplingRate
									 * 0x69:Mpeg2Adts
									 * 0x6A:Mpeg1Video
									 * 0x6B:Mpeg1Adts *
									 * 0x6C:JpegVideo *
									 * 0xC0:PrivateAudio
									 * 0xD0:PrivateVideo
									 * 0xE0:PcmLeAudio16Bit
									 * 0xE1:VorbisAudio x
									 * 0xE2:DolbyV3Ac3Audio
									 * 0xE3:AlowAudio
									 * 0xE4:MulawAudio
									 * 0xE5:AdpcmAudio
									 * 0xE6:PcBinEndianAudio16Bit
									 * 0xF0:YCbCr420Video
									 * 0xF1:H264Video
									 * 0xF2:H263Video
									 * 0xF3:H261Video
									 */
									switch(frameType) {
									case 0x40:
										reader->track->frame_type = frameType_aac;
										break;
									case 0x6B:
										reader->track->frame_type = frameType_mp3;
										break;
									case 0x6C:
										reader->track->frame_type = frameType_jpeg;
										break;
									case 0xDD:
										reader->track->frame_type = frameType_vorbis;
										break;
									default:
										break;
									}
									// for now, aac or vorbis is the only one to analyze more.
									if(reader->track->frame_type != frameType_aac
									&& reader->track->frame_type != frameType_vorbis) {
										// other codec, stop to analyze.
										break;
									}
									ttLibC_ByteReader_skipByte(byte_reader, 12);
								}
								else if(esTagType == 5) { // decoderSpecific
									uint8_t *buf = data + byte_reader->read_size;
									switch(reader->track->frame_type) {
									case frameType_aac:
										memcpy(&reader->track->dsi_info, buf, esTagSize);
										break;
									case frameType_vorbis:
										{
											reader->track->private_data = ttLibC_DynamicBuffer_make();
											ttLibC_DynamicBuffer_append(reader->track->private_data, buf, esTagSize); // コピっとく。
										}
										break;
									default:
										break;
									}
									break; // something I need is done.
								}
								else if(esTagType == 6) { // slConfig
									break;
								}
								else {
									break;
								}
							}
						}
						break;
					default:
						ERR_PRINT("unknown tag for stsd:%x", in_tag);
						break;
					}
				}
				break;
			case Mp4Type_Stts:
				{
					reader->track->stts = ttLibC_Stts_make(data, size, reader->track->timebase);
					if(reader->track->stts == NULL) {
						reader->error_number = 1;
					}
				}
				break;
			case Mp4Type_Ctts:
				{
					reader->track->ctts = ttLibC_Ctts_make(data, size, reader->track->timebase);
					if(reader->track->ctts == NULL) {
						reader->error_number = 1;
					}
				}
				break;
//			case Mp4Type_Stss:
			case Mp4Type_Stsc:
				{
					reader->track->stsc = ttLibC_Stsc_make(data, size, reader->timebase);
					if(reader->track->stsc == NULL) {
						reader->error_number = 1;
					}
				}
				break;
			case Mp4Type_Stsz:
				{
					reader->track->stsz = ttLibC_Stsz_make(data, size, reader->timebase);
					if(reader->track->stsz == NULL) {
						reader->error_number = 1;
					}
				}
				break;
			case Mp4Type_Stco:
				{
					reader->track->stco = ttLibC_Stco_make(data, size, reader->timebase);
					if(reader->track->stco == NULL) {
						reader->error_number = 1;
					}
				}
				break;
			case Mp4Type_Mvex:
				{
					reader->is_fmp4 = true;
					reader->mvex = (ttLibC_Mp4 *)ttLibC_Mp4Atom_make(
							NULL,
							data,
							size,
							false,
							0,
							reader->timebase,
							tag);
					if(reader->mvex == NULL) {
						reader->error_number = 1;
					}
					// analyze this data is on the case of first moof detect.
				}
				break;
			default:
				break;
			}
		}
		reader->position += size;
		move_size = size;
		break;
	default:
		{
			uint32_t name = be_uint32_t(tag);
			char buf[5] = {0};
			memcpy(buf, &name, 4);
			LOG_PRINT("unknown Tag:%s", buf);
		}
		reader->error_number = 1;
		ttLibC_ByteReader_close(&byte_reader);
		return false;
	}
	if(reader->track != NULL) {
		reader->track->size -= move_size;
	}
	// call callback.
	ttLibC_Mp4Atom *mp4Atom = ttLibC_Mp4Atom_make(
			reader->atom,
			data,
			size,
			true,
			0,
			reader->timebase,
			tag);
	if(mp4Atom != NULL) {
		mp4Atom->position = reader->position - size;
		mp4Atom->inherit_super.is_complete = true;
		mp4Atom->reader = (ttLibC_Mp4Reader *)reader;
		reader->atom = mp4Atom;
		if(callback != NULL) {
			if(!callback(ptr, (ttLibC_Mp4 *)mp4Atom)) {
				reader->error_number = 1;
			}
		}
	}
	if(reader->track != NULL) {
		// trak is done. put track into stlmap.
		if(reader->track->size == 0) {
			if(reader->track->track_number == -1) {
				ERR_PRINT("track_number is invalid. mp4 is broken.");
				Mp4Reader_closeTrack(NULL, NULL, reader->track);
			}
			else {
				ttLibC_StlMap_put(reader->tracks, (void *)(long)reader->track->track_number, reader->track);
			}
			reader->track = NULL;
		}
	}
	ttLibC_DynamicBuffer_markAsRead(reader->tmp_buffer, move_size);
	ttLibC_ByteReader_close(&byte_reader);
	return true;
}

bool ttLibC_Mp4Reader_read(
		ttLibC_Mp4Reader *reader,
		void *data,
		size_t data_size,
		ttLibC_Mp4ReadFunc callback,
		void *ptr) {
	ttLibC_Mp4Reader_ *reader_ = (ttLibC_Mp4Reader_ *)reader;
	ttLibC_DynamicBuffer_append(reader_->tmp_buffer, data, data_size);
	if(reader_->in_reading) {
		return true;
	}
	reader_->in_reading = true;
	while(Mp4Reader_readAtom(reader_, callback, ptr)) {
		if(reader_->error_number != 0) {
			break;
		}
	}
	ttLibC_DynamicBuffer_clear(reader_->tmp_buffer);
	reader_->in_reading = false;
	return reader_->error_number == 0;
}

static bool Mp4Reader_closeTrack(void *ptr, void *key, void *item) {
	(void)ptr;
	(void)key;
	ttLibC_Mp4Track *track = (ttLibC_Mp4Track *)item;
	if(track != NULL) {
		ttLibC_Frame_close(&track->frame);
		ttLibC_DynamicBuffer_close(&track->private_data);
		ttLibC_Mp4Atom_close((ttLibC_Mp4Atom **)&track->stsc);
		ttLibC_Mp4Atom_close((ttLibC_Mp4Atom **)&track->stts);
		ttLibC_Mp4Atom_close((ttLibC_Mp4Atom **)&track->stsz);
		ttLibC_Mp4Atom_close((ttLibC_Mp4Atom **)&track->stco);
		ttLibC_Mp4Atom_close((ttLibC_Mp4Atom **)&track->ctts);
		ttLibC_Mp4Atom_close((ttLibC_Mp4Atom **)&track->trun);
		ttLibC_Mp4Atom_close((ttLibC_Mp4Atom **)&track->elst);
		ttLibC_free(track);
	}
	return true;
}

void ttLibC_Mp4Reader_close(ttLibC_Mp4Reader **reader) {
	ttLibC_Mp4Reader_ *target = (ttLibC_Mp4Reader_ *)*reader;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mp4) {
		ERR_PRINT("this reader is not mp4 reader.");
		return;
	}
	ttLibC_StlMap_forEach(target->tracks, Mp4Reader_closeTrack, NULL);
	ttLibC_StlMap_close(&target->tracks);
	if(target->track != NULL) {
		Mp4Reader_closeTrack(NULL, NULL, target->track);
		target->track = NULL;
	}
	ttLibC_Mp4Atom_close((ttLibC_Mp4Atom **)&target->mvex);
	ttLibC_Mp4Atom_close(&target->atom);
	ttLibC_DynamicBuffer_close(&target->tmp_buffer);
	ttLibC_DynamicBuffer_close(&target->mdat_buffer);
	ttLibC_free(target);
	*reader = NULL;
}
