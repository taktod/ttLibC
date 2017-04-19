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

void ttLibC_SocketInfo_FD_ZERO(fd_set *set);
void ttLibC_SocketInfo_FD_COPY(fd_set *dst, fd_set *org);
void ttLibC_SocketInfo_FD_SET(ttLibC_SocketInfo *socket_info, fd_set *set);
bool ttLibC_SocketInfo_FD_ISSET(ttLibC_SocketInfo *socket_info, fd_set *set);
void ttLibC_SocketInfo_FD_CLR(ttLibC_SocketInfo *socket_info, fd_set *set);
int ttLibC_SocketInfo_updateFDMax(ttLibC_SocketInfo *socket_info, int fd_max);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_NET_H_ */
