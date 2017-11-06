/**
 * @file   mkv.h
 * @brief  mkv(webm) container support.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/04
 */

#ifndef TTLIBC_CONTAINER_MKV_H_
#define TTLIBC_CONTAINER_MKV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "container.h"

/**
 * enum for mkv type.
 */
typedef enum ttLibC_Mkv_Type {
	MkvType_EBML = 0x1A45DFA3,
		MkvType_EBMLVersion = 0x4286,
		MkvType_EBMLReadVersion = 0x42F7,
		MkvType_EBMLMaxIDLength = 0x42F2,
		MkvType_EBMLMaxSizeLength = 0x42F3,
		MkvType_DocType = 0x4282,
		MkvType_DocTypeVersion = 0x4287,
		MkvType_DocTypeReadVersion = 0x4285,
	MkvType_Segment = 0x18538067,
		MkvType_SeekHead = 0x114D9B74,
			MkvType_Seek = 0x4DBB,
				MkvType_SeekID = 0x53AB,
				MkvType_SeekPosition = 0x53AC,
		MkvType_Info = 0x1549A966,
			MkvType_SegmentUID = 0x73A4,
			MkvType_SegmentFilename = 0x7384,
			MkvType_PrevUID = 0x3CB923,
			MkvType_PrevFilename = 0x3C83AB,
			MkvType_NextUID = 0x3EB923,
			MkvType_NextFilename = 0x3E83BB,
			MkvType_SegmentFamily = 0x4444,
			MkvType_ChapterTranslate = 0x6924,
				MkvType_ChapterTranslateEditionUID = 0x69FC,
				MkvType_ChapterTranslateCodec = 0x69BF,
				MkvType_ChapterTranslateID = 0x69A5,
			MkvType_TimecodeScale = 0x2AD7B1,
			MkvType_Duration = 0x4489,
			MkvType_DateUTC = 0x4461,
			MkvType_Title = 0x7BA9,
			MkvType_MuxingApp = 0x4D80,
			MkvType_WritingApp = 0x5741,
		MkvType_Cluster = 0x1F43B675,
			MkvType_Timecode = 0xE7,
			MkvType_Position = 0xA7,
			MkvType_PrevSize = 0xAB,
			MkvType_SimpleBlock = 0xA3,
			MkvType_BlockGroup = 0xA0,
				MkvType_Block = 0xA1,
				MkvType_BlockDuration = 0x9B,
				MkvType_ReferenceBlock = 0xFB,
				MkvType_DiscardPadding = 0x75A2,

				MkvType_BlockAdditions = 0x75A1,
					MkvType_BlockMore = 0xA6,
						MkvType_BlockAddID = 0xEE,
						MkvType_BlockAdditional = 0xA5,
			MkvType_EncryptedBlock = 0xAF, // deplicated
		MkvType_Tracks = 0x1654AE6B,
			MkvType_TrackEntry = 0xAE,
				MkvType_TrackNumber = 0xD7,
				MkvType_TrackUID = 0x73C5,
				MkvType_TrackType = 0x83,
				MkvType_FlagEnabled = 0xb9,
				MkvType_FlagDefault = 0x88,
				MkvType_FlagForced = 0x55aa,
				MkvType_FlagLacing = 0x9C,
				MkvType_MinCache = 0x6de7,
				MkvType_DefaultDuration = 0x23E383,
				MkvType_TrackTimecodeScale = 0x23314f, // depricated
				MkvType_MaxBlockAdditionID = 0x55ee,
				MkvType_Language = 0x22B59C,
				MkvType_CodecID = 0x86,
				MkvType_CodecPrivate = 0x63A2,
				MkvType_CodecName = 0x258688,
				MkvType_CodecDecodeAll = 0xAA,
				MkvType_CodecDelay = 0x56AA,
				MkvType_SeekPreRoll = 0x56BB,
				MkvType_FrameRate = 0x2383E3,
				MkvType_Video = 0xE0,
					MkvType_FlagInterlaced = 0x9a,
					MkvType_AlphaMode = 0x53C0,
					MkvType_PixelWidth = 0xB0,
					MkvType_PixelHeight = 0xBA,
					MkvType_DisplayWidth = 0x54B0,
					MkvType_DisplayHeight = 0x54BA,
					MkvType_DisplayUnit = 0x54B2,
					MkvType_Colour = 0x55B0,
				MkvType_Audio = 0xE1,
					MkvType_SamplingFrequency = 0xB5,
					MkvType_Channels = 0x9F,
					MkvType_BitDepth = 0x6264,
				MkvType_ContentEncodings = 0x6D80,
					MkvType_ContentEncoding = 0x6240,
						MkvType_ContentCompression = 0x5034,
							MkvType_ContentCompAlgo = 0x4254,
							MkvType_ContentCompSettings = 0x4255,
		MkvType_Cues = 0x1c53bb6b,
			MkvType_CuePoint = 0xBB,
				MkvType_CueTime = 0xb3,
				MkvType_CueTrackPositions = 0xb7,
					MkvType_CueTrack = 0xf7,
					MkvType_CueClusterPosition = 0xf1,
					MkvType_CueRelativePosition = 0xf0,
		MkvType_Tags = 0x1254c367,
			MkvType_Tag = 0x7373,
				MkvType_Targets = 0x63c0,
				MkvType_TagTrackUID = 0x63c5,
				MkvType_SimpleTag = 0x67c8,
					MkvType_TagName = 0x45a3,
					MkvType_TagLanguage = 0x447A,
					MkvType_TagString = 0x4487,
	MkvType_Void = 0xEC,
	MkvType_CRC32 = 0xBF,
} ttLibC_Mkv_Type;

/**
 * definition of mkv object.
 */
typedef struct ttLibC_Container_Mkv {
	ttLibC_Container inherit_super;
	ttLibC_Mkv_Type type;
} ttLibC_Container_Mkv;

typedef ttLibC_Container_Mkv ttLibC_Mkv;

/**
 * get frame object from mkv object.
 * @param mkv
 * @param callback
 * @param ptr
 */
bool ttLibC_Mkv_getFrame(
		ttLibC_Mkv *mkv,
		ttLibC_getFrameFunc callback,
		void *ptr);

// -------------------------------------------------------------- //
// reader

/**
 * mkv reader definition
 */
typedef struct ttLibC_ContainerReader_MkvReader {
	ttLibC_ContainerReader inherit_super;
} ttLibC_ContainerReader_MkvReader;

typedef ttLibC_ContainerReader_MkvReader ttLibC_MkvReader;

/**
 * callback for mkv object reading.
 * @param ptr user def pointer.
 * @param mkv read mkv object.
 * @return true:continue to work. false:stop
 */
typedef bool (* ttLibC_MkvReadFunc)(void *ptr, ttLibC_Mkv *mkv);

/**
 * make mkv reader object.
 */
ttLibC_MkvReader *ttLibC_MkvReader_make();

/**
 * read mkv object.
 * @param reader
 * @param data
 * @param data_size
 * @param callback
 * @param ptr
 * @return true:success false:error
 */
bool ttLibC_MkvReader_read(
		ttLibC_MkvReader *reader,
		void *data,
		size_t data_size,
		ttLibC_MkvReadFunc callback,
		void *ptr);

/**
 * close mkv reader
 * @param reader
 */
void ttLibC_MkvReader_close(ttLibC_MkvReader **reader);

// -------------------------------------------------------------- //
// writer

typedef ttLibC_ContainerWriter ttLibC_MkvWriter;

ttLibC_MkvWriter *ttLibC_MkvWriter_make(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num);

ttLibC_MkvWriter *ttLibC_MkvWriter_make_ex(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num,
		uint32_t unit_duration);

bool ttLibC_MkvWriter_write(
		ttLibC_MkvWriter *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

void ttLibC_MkvWriter_close(ttLibC_MkvWriter **writer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MKV_H_ */
