/*
 * netCommon.h
 *
 *  Created on: 2017/04/19
 *      Author: taktod
 */

#ifndef TTLIBC_NET_NETCOMMON_H_
#define TTLIBC_NET_NETCOMMON_H_

#include "net.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

typedef struct ttLibC_Net_SockaddrIn_{
	ttLibC_SockaddrIn inherit_super;
	struct sockaddr_in addr;
} ttLibC_Net_SockaddrIn_;

typedef ttLibC_Net_SockaddrIn_ ttLibC_SockaddrIn_;

typedef struct ttLibC_Net_Fdset_ {
	ttLibC_Fdset inherit_super;
	fd_set fdset;
} ttLibC_Net_Fdset_;

typedef ttLibC_Net_Fdset_ ttLibC_Fdset_;

#endif /* TTLIBC_NET_NETCOMMON_H_ */
