/**
 * @file   tcpmisc.h
 * @brief  misc for tcp.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/01/08
 */

#ifndef TTLIBC_NET_TCPMISC_H_
#define TTLIBC_NET_TCPMISC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../util/dynamicBufferUtil.h"

#include "tcp.h"

/**
 * clientInfo hidden information.
 */
typedef struct ttLibC_Net_TcpClientInfo_ {
	ttLibC_TcpClientInfo inherit_super;
	ttLibC_DynamicBuffer *write_buffer;
} ttLibC_Net_TcpClientInfo_;

typedef ttLibC_Net_TcpClientInfo_ ttLibC_TcpClientInfo_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_TCPMISC_H_ */
