/**
 * @file   mpegtsReader.h
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#ifndef TTLIBC_CONTAINER_MPEGTS_MPEGTSREADER_H_
#define TTLIBC_CONTAINER_MPEGTS_MPEGTSREADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mpegtsPacket.h"

#include "type/pat.h"
#include "type/pes.h"
#include "type/pmt.h"
#include "type/sdt.h"

typedef struct {
	ttLibC_MpegtsReader inherit_super;

	// 使い回すpacketオブジェクト
	ttLibC_Pat *pat;
	ttLibC_Pmt *pmt;
	ttLibC_Sdt *sdt;
	ttLibC_Pes *pes[MaxPesTracks]; // ここに必要なpesデータをいれておく。
	// とりあえずトラックは５個まで対応することにする。

	// 処理関連
	ttLibC_Mpegts_Type type;
	size_t target_size; // 読み込むべきデータサイズ
	uint16_t pmt_pid;
	// これにより複数トラックのmpegtsから特定の音声と映像だけ取得ということができる。

	// 一時バッファ関連
	uint8_t *tmp_buffer;
	size_t tmp_buffer_size;
	size_t tmp_buffer_next_pos;
} ttLibC_ContainerReader_MpegtsReader_;

typedef ttLibC_ContainerReader_MpegtsReader_ ttLibC_MpegtsReader_;


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_MPEGTSREADER_H_ */
