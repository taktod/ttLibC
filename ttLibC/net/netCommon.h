/*
 * netCommon.h
 *
 *  Created on: 2017/04/19
 *      Author: taktod
 */

#ifndef TTLIBC_NET_NETCOMMON_H_
#define TTLIBC_NET_NETCOMMON_H_

#include "net.h"

typedef struct ttLibC_Net_SockaddrIn_{
	ttLibC_SockaddrIn inherit_super;
	struct sockaddr_in addr;
} ttLibC_Net_SockaddrIn_;

typedef ttLibC_Net_SockaddrIn_ ttLibC_SockaddrIn_;

#endif /* TTLIBC_NET_NETCOMMON_H_ */
