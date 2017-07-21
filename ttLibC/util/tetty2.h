/*
 * @file   tetty2.h
 * @brief  framework like netty.
 *
 * this code is under 3-Clause BSD License.
 *
 * @author taktod
 * @date   2017/07/21
 */
#ifndef TTLIBC_UTIL_TETTY2_H_
#define TTLIBC_UTIL_TETTY2_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// error information number to use inside tetty2
typedef int32_t tetty2_errornum;

// hold any information
typedef struct ttLibC_Util_Tetty2Info {
	void *bootstrap_ptr;
	void *ptr;
} ttLibC_Util_Tetty2Info;
typedef struct ttLibC_Util_Tetty2Info ttLibC_Tetty2Info;

// definition ref
struct ttLibC_Util_Tetty2Context;
typedef struct ttLibC_Util_Tetty2Context ttLibC_Tetty2Context;

struct ttLibC_Util_Tetty2Bootstrap;
typedef struct ttLibC_Util_Tetty2Bootstrap ttLibC_Tetty2Bootstrap;

struct ttLibC_Util_Tetty2ChannelHandler;
typedef struct ttLibC_Util_Tetty2ChannelHandler ttLibC_Tetty2ChannelHandler;

struct ttLibC_Util_Tetty2Promise;
typedef struct ttLibC_Util_Tetty2Promise ttLibC_Tetty2Promise;

// bootstrap
typedef struct ttLibC_Util_Tetty2Bootstrap {
	tetty2_errornum error_number;
} ttLibC_Util_Tetty2Bootstrap;

typedef tetty2_errornum (*ttLibC_Tetty2_ContextFunc)(ttLibC_Tetty2Context *ctx);
typedef tetty2_errornum (*ttLibC_Tetty2_DataFunc)(ttLibC_Tetty2Context *ctx, void *data, size_t data_size);
typedef void (*ttLibC_Tetty2_ExceptionFunc)(ttLibC_Tetty2Context *ctx, tetty2_errornum error_num);

typedef struct ttLibC_Util_Tetty2ChannelHandler {
	ttLibC_Tetty2_ContextFunc   channelActive;
	ttLibC_Tetty2_ContextFunc   channelInactive;
	ttLibC_Tetty2_DataFunc      channelRead;
	ttLibC_Tetty2_ContextFunc   close;
	ttLibC_Tetty2_DataFunc      write;
	ttLibC_Tetty2_ContextFunc   flush;
	ttLibC_Tetty2_ExceptionFunc exceptionCaught;
	ttLibC_Tetty2_DataFunc      userEventTriggered;
} ttLibC_Util_Tetty2ChannelHandler;

/**
 * close bootstrap
 * @param bootstrap bootstrap object.
 */
void ttLibC_Tetty2Bootstrap_close(ttLibC_Tetty2Bootstrap **bootstrap);

/**
 * add channel handler on pipeline.
 * @param bootstrap
 * @param channel_handler
 */
void ttLibC_Tetty2Bootstrap_pipeline_addLast(
		ttLibC_Tetty2Bootstrap *bootstrap,
		void *channel_handler);

/**
 * remove channel handler on pipeline
 * @param bootstrap
 * @param channel_handler
 */
void ttLibC_Tetty2Bootstrap_pipeline_remove(
		ttLibC_Tetty2Bootstrap *bootstrap,
		void *channel_handler);

/**
 * fire userEventTriggered on pipeline.
 * @param bootstrap
 * @param data
 * @param data_size
 * @return error number 0:noerror
 */
tetty2_errornum ttLibC_Tetty2Bootstrap_userEventTriggered(
		ttLibC_Tetty2Bootstrap *bootstrap,
		void *data,
		size_t data_size);

/**
 * fire to write message to bootstrap
 * @param bootstrap
 * @param data
 * @param data_size
 */
tetty2_errornum ttLibC_Tetty2Bootstrap_write(
		ttLibC_Tetty2Bootstrap *bootstrap,
		void *data,
		size_t data_size);

/**
 * flush write buffer to bootstrap
 * @param bootstrap
 */
tetty2_errornum ttLibC_Tetty2Bootstrap_flush(ttLibC_Tetty2Bootstrap *bootstrap);

/**
 * write and flush message
 * @param bootstrap
 * @param data
 * @param data_size
 */
tetty2_errornum ttLibC_Tetty2Bootstrap_writeAndFlush(
		ttLibC_Tetty2Bootstrap *bootstrap,
		void *data,
		size_t data_size);

/**
 * make promise
 * @param bootstrap
 */
ttLibC_Tetty2Promise *ttLibC_Tetty2Bootstrap_makePromise(ttLibC_Tetty2Bootstrap *bootstrap);

// context
typedef struct ttLibC_Util_Tetty2Context {
	ttLibC_Tetty2Bootstrap *bootstrap;
	ttLibC_Tetty2ChannelHandler *channel_handler;
	ttLibC_Tetty2Info *tetty_info;
} ttLibC_Util_Tetty2Context;

tetty2_errornum ttLibC_Tetty2Context_channel_write(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size);

tetty2_errornum ttLibC_Tetty2Context_channel_flush(ttLibC_Tetty2Context *ctx);

tetty2_errornum ttLibC_Tetty2Context_channel_writeAndFlush(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size);

tetty2_errornum ttLibC_Tetty2Context_close(ttLibC_Tetty2Context *ctx);

tetty2_errornum ttLibC_Tetty2Context_super_channelActive(ttLibC_Tetty2Context *ctx);
tetty2_errornum ttLibC_Tetty2Context_super_channelInactive(ttLibC_Tetty2Context *ctx);
tetty2_errornum ttLibC_Tetty2Context_super_channelRead(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size);
tetty2_errornum ttLibC_Tetty2Context_super_close(ttLibC_Tetty2Context *ctx);
tetty2_errornum ttLibC_Tetty2Context_super_write(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size);
tetty2_errornum ttLibC_Tetty2Context_super_flush(ttLibC_Tetty2Context *ctx);
tetty2_errornum ttLibC_Tetty2Context_super_exceptionCaught(
		ttLibC_Tetty2Context *ctx,
		tetty2_errornum error_no);
tetty2_errornum ttLibC_Tetty2Context_super_userEventTriggered(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size);

// promise
typedef struct ttLibC_Util_Tetty2Promise {
	ttLibC_Tetty2Bootstrap *bootstrap;
	bool is_done;
	bool is_success;
	void *return_val;
} ttLibC_Util_Tetty2Promise;

typedef void (* ttLibC_Tetty2PromiseListener)(void *ptr, ttLibC_Tetty2Promise *promise);

void ttLibC_Tetty2Promise_addEventListener(
		ttLibC_Tetty2Promise *promise,
		ttLibC_Tetty2PromiseListener listener,
		void *ptr);

void ttLibC_Tetty2Promise_setSuccess(
		ttLibC_Tetty2Promise *promise,
		void *data);
void ttLibC_Tetty2Promise_setFailure(
		ttLibC_Tetty2Promise *promise,
		void *data);
void ttLibC_Tetty2Promise_close(ttLibC_Tetty2Promise **promise);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_TETTY2_H_ */
