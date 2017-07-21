/*
 * tcpBootstrap.h
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifndef TTLIBC_NET_TETTY2_TCPBOOTSTRAP_H_
#define TTLIBC_NET_TETTY2_TCPBOOTSTRAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../tcp.h"
#include "../../util/tetty2.h"

typedef enum ttLibC_Tetty2_TcpOption{
	Tetty2Option_SO_KEEPALIVE,
	Tetty2Option_SO_REUSEADDR,
	Tetty2Option_TCP_NODELAY
} ttLibC_Tetty2_TcpOption;

ttLibC_Tetty2Bootstrap *ttLibC_TcpBootstrap_make();

bool ttLibC_TcpBootstrap_setOption(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2_TcpOption option);

bool ttLibC_TcpBootstrap_connect(
		ttLibC_Tetty2Bootstrap *bootstrap,
		const char *host,
		int port);

bool ttLibC_TcpBootstrap_bind(
		ttLibC_Tetty2Bootstrap *bootstrap,
		int port);

bool ttLibC_TcpBootstrap_update(
		ttLibC_Tetty2Bootstrap *bootstrap,
		uint32_t wait_interval);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_TETTY2_TCPBOOTSTRAP_H_ */
