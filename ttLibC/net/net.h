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

#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdint.h>

typedef void* ttLibC_Net_SockaddrIn;
typedef ttLibC_Net_SockaddrIn ttLibC_SockaddrIn;

ttLibC_SockaddrIn *ttLibC_SockaddrIn_make();
void ttLibC_SockaddrIn_close(ttLibC_SockaddrIn **addr);

typedef void* ttLibC_Net_Fdset;
typedef ttLibC_Net_Fdset ttLibC_Fdset;

ttLibC_Fdset *ttLibC_Fdset_make();
void ttLibC_Fdset_close(ttLibC_Fdset **fdset);

/**
 * definition of socket
 */
typedef struct ttLibC_Net_SocketInfo {
	int socket;
	ttLibC_SockaddrIn *addr;
	int32_t error_num;
	void *ptr;
} ttLibC_Net_SocketInfo;

typedef ttLibC_Net_SocketInfo ttLibC_SocketInfo;

void ttLibC_Fdset_FD_ZERO(ttLibC_Fdset *set);
void ttLibC_Fdset_FD_COPY(ttLibC_Fdset *dst, ttLibC_Fdset *org);
void ttLibC_Fdset_FD_SET(ttLibC_SocketInfo *socket_info, ttLibC_Fdset *set);
bool ttLibC_Fdset_FD_ISSET(ttLibC_SocketInfo *socket_info, ttLibC_Fdset *set);
void ttLibC_Fdset_FD_CLR(ttLibC_SocketInfo *socket_info, ttLibC_Fdset *set);
int ttLibC_Fdset_updateFDMax(ttLibC_SocketInfo *socket_info, int fd_max);
int ttLibC_Fdset_select(
		int fd_max,
		ttLibC_Fdset *readset,
		ttLibC_Fdset *writeset,
		ttLibC_Fdset *exceptset,
		uint64_t wait_interval);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_NET_H_ */
