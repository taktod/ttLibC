/**
 * @file   udp.c
 * @brief  base for udp.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/01/15
 */

#ifdef __ENABLE_SOCKET__

#include "udp.h"
#include "netCommon.h"
#include "../allocator.h"
#include "../_log.h"
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

ttLibC_UdpSocketInfo *ttLibC_UdpSocket_make(uint16_t port) {
	ttLibC_UdpSocketInfo *socket_info = (ttLibC_UdpSocketInfo *)ttLibC_malloc(sizeof(ttLibC_UdpSocketInfo));
	if(socket_info == NULL) {
		ERR_PRINT("failed to allocator ttLibC_UdpSocketInfo.");
		return NULL;
	}
	memset(socket_info, 0, sizeof(ttLibC_UdpSocketInfo));
	socket_info->socket = -1;
	socket_info->addr = ttLibC_SockaddrIn_make();
	ttLibC_SockaddrIn_ *addr = (ttLibC_SockaddrIn_ *)socket_info->addr;
	addr->addr.sin_family = AF_INET;
	addr->addr.sin_port = htons(port);
	addr->addr.sin_addr.s_addr = INADDR_ANY;
	return socket_info;
}

bool ttLibC_UdpSocket_open(ttLibC_UdpSocketInfo *socket_info) {
	if(socket_info == NULL) {
		return false;
	}
	socket_info->socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_info->socket == -1) {
		ERR_PRINT("failed to open socket.");
		return false;
	}
	ttLibC_SockaddrIn_ *addr = (ttLibC_SockaddrIn_ *)socket_info->addr;
	// bind
	if(bind(
			socket_info->socket,
			(struct sockaddr *)&addr->addr,
			sizeof(addr->addr)) == -1) {
		ERR_PRINT("failed to bind.");
		return false;
	}
	// do bind no listen.
	return true;
}

bool ttLibC_UdpSocket_write(
		ttLibC_UdpSocketInfo *socket_info,
		ttLibC_DatagramPacket *packet) {
	// sendto with datagramPacket.
	if(socket_info == NULL) {
		return false;
	}
	if(packet == NULL) {
		return false;
	}
	ttLibC_SockaddrIn_ *addr = (ttLibC_SockaddrIn_ *)packet->socket_info.addr;
	int64_t size = sendto(
			socket_info->socket,
			packet->data,
			packet->data_size,
			0,
			(struct sockaddr *)&addr->addr,
			sizeof(addr->addr));
	if(size == -1) {
		return false;
	}
	if(size == (int64_t)packet->data_size) {
		return true;
	}
	else {
		return false;
	}
}

int64_t ttLibC_UdpSocket_read(
		ttLibC_UdpSocketInfo *socket_info,
		ttLibC_DatagramPacket *packet) {
	// recvfrom
	if(socket_info == NULL) {
		return -1;
	}
	if(packet == NULL) {
		return -1;
	}
	// update datagramPacket information.
	socklen_t sockaddr_in_size = sizeof(struct sockaddr_in);
	ttLibC_SockaddrIn_ *addr = (ttLibC_SockaddrIn_ *)packet->socket_info.addr;
	int64_t recv_size = recvfrom(
			socket_info->socket,
			packet->data,
			packet->data_size,
			0,
			(struct sockaddr *)&addr->addr,
			(socklen_t *)&sockaddr_in_size);
	if(recv_size >= 0) {
		packet->buffer_size = recv_size;
	}
	return recv_size;
}

void ttLibC_UdpSocket_close(ttLibC_UdpSocketInfo **socket_info) {
	ttLibC_UdpSocketInfo *target = *socket_info;
	if(target == NULL) {
		return;
	}
	if(target->socket != -1) {
		close(target->socket);
	}
	ttLibC_SockaddrIn_close(&target->addr);
	ttLibC_free(target);
	*socket_info = NULL;
}

ttLibC_DatagramPacket *ttLibC_DatagramPacket_make(
		void *data,
		size_t data_size) {
	return ttLibC_DatagramPacket_makeWithTarget(
			data, data_size, "127.0.0.1", 0);
}

ttLibC_DatagramPacket *ttLibC_DatagramPacket_makeWithTarget(
		void *data,
		size_t data_size,
		const char *target_address,
		int16_t target_port) {
	ttLibC_DatagramPacket *packet = (ttLibC_DatagramPacket *)ttLibC_malloc(sizeof(ttLibC_DatagramPacket));
	if(packet == NULL) {
		return NULL;
	}
	packet->socket_info.addr = ttLibC_SockaddrIn_make();
	ttLibC_SockaddrIn_ *addr = (ttLibC_SockaddrIn_ *)packet->socket_info.addr;
	addr->addr.sin_family = AF_INET;
	addr->addr.sin_port = htons(target_port);
	addr->addr.sin_addr.s_addr = inet_addr(target_address);
	packet->data = data;
	packet->data_size = data_size;
	packet->buffer_size = data_size;
	return packet;
}

void ttLibC_DatagramPacket_close(ttLibC_DatagramPacket **packet) {
	ttLibC_DatagramPacket *target = *packet;
	if(target == NULL) {
		return;
	}
	ttLibC_SockaddrIn_close(&target->socket_info.addr);
	ttLibC_free(target);
	*packet = NULL;
}

#endif

