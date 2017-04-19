/**
 * @file   net.c
 * @brief  
 * @author taktod
 * @date   2017/04/10
 */

#include "netCommon.h"

#include <string.h>
#include "../allocator.h"

ttLibC_SockaddrIn *ttLibC_SockaddrIn_make() {
	ttLibC_SockaddrIn_ *addr = ttLibC_malloc(sizeof(ttLibC_SockaddrIn_));
	memset(addr, 0, sizeof(ttLibC_SockaddrIn_));
	return (ttLibC_SockaddrIn *)addr;
}
void ttLibC_SockaddrIn_close(ttLibC_SockaddrIn **addr) {
	ttLibC_SockaddrIn_ *target = (ttLibC_SockaddrIn_ *)*addr;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*addr = NULL;
}

void ttLibC_SocketInfo_FD_ZERO(fd_set *set) {
	FD_ZERO(set);
}

void ttLibC_SocketInfo_FD_COPY(fd_set *dst, fd_set *org) {
	memcpy(dst, org, sizeof(fd_set));
}

void ttLibC_SocketInfo_FD_SET(ttLibC_SocketInfo *socket_info, fd_set *set) {
	FD_SET(socket_info->socket, set);
}

bool ttLibC_SocketInfo_FD_ISSET(ttLibC_SocketInfo *socket_info, fd_set *set) {
	return FD_ISSET(socket_info->socket, set);
}

void ttLibC_SocketInfo_FD_CLR(ttLibC_SocketInfo *socket_info, fd_set *set) {
	FD_CLR(socket_info->socket, set);
}

int ttLibC_SocketInfo_updateFDMax(ttLibC_SocketInfo *socket_info, int fd_max) {
	if(fd_max < socket_info->socket) {
		return socket_info->socket;
	}
	return fd_max;
}
