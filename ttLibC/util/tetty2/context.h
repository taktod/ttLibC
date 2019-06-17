/*
 * context.h
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifndef TTLIBC_UTIL_TETTY2_CONTEXT_H_
#define TTLIBC_UTIL_TETTY2_CONTEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../ttLibC_predef.h" 
#include "../tetty2.h"
#include "../stlListUtil.h"
#include <stdio.h>
#include <stdbool.h>

typedef enum ttLibC_Tetty2ContextCommand {
	Tetty2Command_channelActive,
	Tetty2Command_channelInactive,
	Tetty2Command_channelRead,
	Tetty2Command_close,
	Tetty2Command_write,
	Tetty2Command_flush,
	Tetty2Command_exceptionCaught,
	Tetty2Command_userEventTriggered,
} ttLibC_Tetty2ContextCommand;

typedef struct ttLibC_Util_Tetty2Context_ {
	ttLibC_Tetty2Context inherit_super;
	ttLibC_Tetty2ContextCommand command;
	void *data;
	size_t data_size;
	int32_t error_no;
} ttLibC_Util_Tetty2Context_;

typedef ttLibC_Util_Tetty2Context_ ttLibC_Tetty2Context_;

tetty2_errornum TT_ATTRIBUTE_INNER ttLibC_Tetty2Context_channelActive_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info);

tetty2_errornum TT_ATTRIBUTE_INNER ttLibC_Tetty2Context_channelInactive_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info);

tetty2_errornum TT_ATTRIBUTE_INNER ttLibC_Tetty2Context_channelRead_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info,
		void *data,
		size_t data_size);

tetty2_errornum TT_ATTRIBUTE_INNER ttLibC_Tetty2Context_write_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info,
		void *data,
		size_t data_size);

tetty2_errornum TT_ATTRIBUTE_INNER ttLibC_Tetty2Context_flush_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info);

tetty2_errornum TT_ATTRIBUTE_INNER ttLibC_Tetty2Context_close_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info);

// no def for this function?
tetty2_errornum TT_ATTRIBUTE_INNER ttLibC_Tetty2Context_exceptionCaught(
		ttLibC_Tetty2Bootstrap *bootstrap,
		tetty2_errornum error_num);

tetty2_errornum TT_ATTRIBUTE_INNER ttLibC_Tetty2Context_userEventTriggered_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info,
		void *data,
		size_t data_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_TETTY2_CONTEXT_H_ */
