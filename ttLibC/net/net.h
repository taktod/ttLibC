/**
 * @file   net.h
 * @brief  base definition of network data.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/01/14
 */

#ifndef TTLIBC_NET_NET_H_
#define TTLIBC_NET_NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>

/**
 * definition of socket
 */
typedef struct ttLibC_Net_SocketInfo {
	int socket;
	struct sockaddr_in addr;
	int32_t error_num;
	void *ptr;
} ttLibC_Net_SocketInfo;

typedef ttLibC_Net_SocketInfo ttLibC_SocketInfo;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_NET_H_ */
