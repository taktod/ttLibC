/**
 * @file   udp.h
 * @brief  base for udp.
 *
 * this code is under 3-Cuase BSD License.
 *
 * @author taktod
 * @date   2016/01/15
 */

#ifndef TTLIBC_NET_UDP_H_
#define TTLIBC_NET_UDP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "net.h"
#include <stdio.h>
#include <stdbool.h>

/**
 * definition of udp socket information.
 */
typedef ttLibC_SocketInfo ttLibC_UdpSocketInfo;

/**
 * definition for datagram packet.
 */
typedef struct ttLibC_Net_DatagramPacket {
	ttLibC_SocketInfo socket_info;
	void *data;
	size_t data_size;
	size_t buffer_size;
} ttLibC_Net_DatagramPacket;

typedef ttLibC_Net_DatagramPacket ttLibC_DatagramPacket;

/**
 * make udp socket
 * @param port
 * @return ttLibC_UdpSocketInfo object.
 */
ttLibC_UdpSocketInfo TT_ATTRIBUTE_API *ttLibC_UdpSocket_make(uint16_t port);

/**
 * bind udp socket.
 * @param socket_info
 * @return true / false
 */
bool TT_ATTRIBUTE_API ttLibC_UdpSocket_open(ttLibC_UdpSocketInfo *socket_info);

/**
 * write datagram packet for udp socket.
 * @param socket_info
 * @param packet      target datagram packet.
 * @return true / false
 */
bool TT_ATTRIBUTE_API ttLibC_UdpSocket_write(
		ttLibC_UdpSocketInfo *socket_info,
		ttLibC_DatagramPacket *packet);

/**
 * read datagram packet from udp socket.
 * @param socket_info
 * @param packet      target datagram packet, this packet will be update.
 * @return size of recv
 */
int64_t TT_ATTRIBUTE_API ttLibC_UdpSocket_read(
		ttLibC_UdpSocketInfo *socket_info,
		ttLibC_DatagramPacket *packet);

/**
 * close udp socket.
 * @param socket_info
 */
void TT_ATTRIBUTE_API ttLibC_UdpSocket_close(ttLibC_UdpSocketInfo **socket_info);

/**
 * make datagram packet.
 * @param data
 * @param data_size
 * @return ttLibC_DatagramPacket
 */
ttLibC_DatagramPacket TT_ATTRIBUTE_API *ttLibC_DatagramPacket_make(
		void *data,
		size_t data_size);

/**
 * make datagram packet with target address.
 * @param data
 * @param data_size
 * @param target_address ipaddress
 * @param target_port
 * @return ttLibC_DatagramPacket
 */
ttLibC_DatagramPacket TT_ATTRIBUTE_API *ttLibC_DatagramPacket_makeWithTarget(
		void *data,
		size_t data_size,
		const char *target_address,
		int16_t target_port);

/**
 * close datagram packet
  * @param packet
 */
void TT_ATTRIBUTE_API ttLibC_DatagramPacket_close(ttLibC_DatagramPacket **packet);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_UDP_H_ */
