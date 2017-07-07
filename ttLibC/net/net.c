/**
 * @file   net.c
 * @brief  
 * @author taktod
 * @date   2017/04/10
 */

#include "netCommon.h"

#include <string.h>
#include "../ttLibC_predef.h"
#include "../allocator.h"

ttLibC_SockaddrIn TT_VISIBILITY_DEFAULT *ttLibC_SockaddrIn_make() {
	ttLibC_SockaddrIn_ *addr = ttLibC_malloc(sizeof(ttLibC_SockaddrIn_));
	if(addr == NULL) {
		return NULL;
	}
	memset(addr, 0, sizeof(ttLibC_SockaddrIn_));
	return (ttLibC_SockaddrIn *)addr;
}
void TT_VISIBILITY_DEFAULT ttLibC_SockaddrIn_close(ttLibC_SockaddrIn **addr) {
	ttLibC_SockaddrIn_ *target = (ttLibC_SockaddrIn_ *)*addr;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*addr = NULL;
}

ttLibC_Fdset TT_VISIBILITY_DEFAULT *ttLibC_Fdset_make() {
	ttLibC_Fdset_ *fdset = ttLibC_malloc(sizeof(ttLibC_Fdset_));
	if(fdset == NULL) {
		return NULL;
	}
	memset(fdset, 0, sizeof(ttLibC_Fdset_));
	return (ttLibC_Fdset *)fdset;
}

void TT_VISIBILITY_DEFAULT ttLibC_Fdset_close(ttLibC_Fdset **fdset) {
	ttLibC_Fdset_ *target = (ttLibC_Fdset_ *)*fdset;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*fdset = NULL;
}

void TT_VISIBILITY_DEFAULT ttLibC_Fdset_FD_ZERO(ttLibC_Fdset *set) {
	ttLibC_Fdset_ *fdset = (ttLibC_Fdset_ *)set;
	FD_ZERO(&fdset->fdset);
}

void TT_VISIBILITY_DEFAULT ttLibC_Fdset_FD_COPY(ttLibC_Fdset *dst, ttLibC_Fdset *org) {
	ttLibC_Fdset_ *dstset = (ttLibC_Fdset_ *)dst;
	ttLibC_Fdset_ *orgset = (ttLibC_Fdset_ *)org;
	memcpy(&dstset->fdset, &orgset->fdset, sizeof(fd_set));
}

void TT_VISIBILITY_DEFAULT ttLibC_Fdset_FD_SET(ttLibC_SocketInfo *socket_info, ttLibC_Fdset *set) {
	ttLibC_Fdset_ *fdset = (ttLibC_Fdset_ *)set;
	FD_SET(socket_info->socket, &fdset->fdset);
}

bool TT_VISIBILITY_DEFAULT ttLibC_Fdset_FD_ISSET(ttLibC_SocketInfo *socket_info, ttLibC_Fdset *set) {
	ttLibC_Fdset_ *fdset = (ttLibC_Fdset_ *)set;
	return FD_ISSET(socket_info->socket, &fdset->fdset);
}

void TT_VISIBILITY_DEFAULT ttLibC_Fdset_FD_CLR(ttLibC_SocketInfo *socket_info, ttLibC_Fdset *set) {
	ttLibC_Fdset_ *fdset = (ttLibC_Fdset_ *)set;
	FD_CLR(socket_info->socket, &fdset->fdset);
}

int TT_VISIBILITY_DEFAULT ttLibC_Fdset_updateFDMax(ttLibC_SocketInfo *socket_info, int fd_max) {
	if(fd_max < socket_info->socket) {
		return socket_info->socket;
	}
	return fd_max;
}

int TT_VISIBILITY_DEFAULT ttLibC_Fdset_select(
		int fd_max,
		ttLibC_Fdset *readset,
		ttLibC_Fdset *writeset,
		ttLibC_Fdset *exceptset,
		uint64_t wait_interval) {
	fd_set *read   = NULL;
	fd_set *write  = NULL;
	fd_set *except = NULL;
	if(readset != NULL) {
		read = &((ttLibC_Fdset_ *)readset)->fdset;
	}
	if(writeset != NULL) {
		write = &((ttLibC_Fdset_ *)writeset)->fdset;
	}
	if(exceptset != NULL) {
		except = &((ttLibC_Fdset_ *)exceptset)->fdset;
	}
	struct timeval timeout;
	timeout.tv_sec = wait_interval / 1000000;
	timeout.tv_usec = wait_interval % 1000000;
	return select(fd_max, read, write, except, &timeout);
}
