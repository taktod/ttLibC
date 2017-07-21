/*
 * bootstrap.h
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifndef TTLIBC_UTIL_TETTY2_BOOTSTRAP_H_
#define TTLIBC_UTIL_TETTY2_BOOTSTRAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../tetty2.h"
#include "../stlListUtil.h"
#include "context.h"

typedef tetty2_errornum (*ttLibC_Tetty2_EventFunc)(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Context *ctx);

typedef struct ttLibC_Util_Tetty2Bootstrap_ {
	ttLibC_Tetty2Bootstrap inherit_super;
	uint32_t project_id; // unique id or project.
	ttLibC_StlList *pipeline;
	ttLibC_Tetty2Info tetty_info;
	ttLibC_Tetty2_EventFunc write_event;
	ttLibC_Tetty2_EventFunc close_event;
	ttLibC_Tetty2_EventFunc flush_event;
} ttLibC_Utill_Tetty2Bootstrap_;

typedef ttLibC_Utill_Tetty2Bootstrap_ ttLibC_Tetty2Bootstrap_;

ttLibC_Tetty2Bootstrap *ttLibC_Tetty2Bootstrap_make(size_t bootstrap_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_TETTY2_BOOTSTRAP_H_ */
