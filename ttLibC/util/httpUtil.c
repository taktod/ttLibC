/*
 * @file   httpUtil.c
 * @brief  library for http access.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/30
 */

#ifdef __ENABLE_FILE__

#include "httpUtil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>

#include "../ttLibC_predef.h"
#include "../_log.h"
#include "../allocator.h"

#define BUF_LEN 256

/**
 * detail data for httpClient
 */
typedef struct {
	ttLibC_HttpClient inherit_super;
	uint8_t *buffer;
} ttLibC_Util_HttpUtil_HttpClient_;

typedef ttLibC_Util_HttpUtil_HttpClient_ ttLibC_HttpClient_;

/*
 * make http client
 * @param buffer_size   download buffer size.
 * @param wait_interval interval for each download.
 * @return http client object.
 */
ttLibC_HttpClient TT_VISIBILITY_DEFAULT *ttLibC_HttpClient_make(
		size_t buffer_size,
		uint32_t wait_interval) {
	ttLibC_HttpClient_ *client = (ttLibC_HttpClient_ *)ttLibC_malloc(sizeof(ttLibC_HttpClient_));
	if(client == NULL) {
		ERR_PRINT("failed to allocate client object.");
		return NULL;
	}
	client->buffer = ttLibC_malloc(buffer_size);
	if(client->buffer == NULL) {
		ERR_PRINT("failed to alloc client buffer.");
		ttLibC_free(client);
		return NULL;
	}
	client->inherit_super.buffer_size = buffer_size;
	sprintf(client->inherit_super.ETag, "");
	client->inherit_super.content_length = 0;
	client->inherit_super.file_length = 0;
	client->inherit_super.wait_interval = wait_interval;
	return (ttLibC_HttpClient *)client;
}

/*
 * get method download.
 * @param client         http client object
 * @param target_address address for download.
 * @param is_binary      true:read as binary false:read as string
 * @param callback       callback for download data.
 * @param ptr            user def data pointer.
 */
void TT_VISIBILITY_DEFAULT ttLibC_HttpClient_get(
		ttLibC_HttpClient *client,
		const char *target_address,
		bool is_binary,
		ttLibC_HttpClientFunc callback,
		void *ptr) {
	ttLibC_HttpClient_getRange(client, target_address, 0, 0, is_binary, callback, ptr);
}

/*
 * get method download.
 * @param client         http client object
 * @param target_address address for download.
 * @param range_start    begin point for download. if 0, ignore
 * @param range_length   download size for download. if 0, ignore
 * @param is_binary      true:read as binary false:read as string
 * @param callback       callback for download data.
 * @param ptr            user def data pointer.
 */
void TT_VISIBILITY_DEFAULT ttLibC_HttpClient_getRange(
		ttLibC_HttpClient *client,
		const char *target_address,
		size_t range_start,
		size_t range_length,
		bool is_binary,
		ttLibC_HttpClientFunc callback,
		void *ptr) {
	if(client == NULL) {
		return;
	}
	if(target_address == NULL || strlen(target_address) == 0) {
		return;
	}
	ttLibC_HttpClient_ *client_ = (ttLibC_HttpClient_ *)client;
	int32_t sock = 0;
	FILE *fp;
	struct hostent *servhost = NULL;
	struct sockaddr_in server;
	char buf[BUF_LEN];
	char host[BUF_LEN];
	char path[BUF_LEN];
	uint16_t port = 80;
	char host_path[BUF_LEN];
	// analyze http address.
	if(strlen(target_address) > BUF_LEN - 1) {
		ERR_PRINT("target address is too long.");
		return;
	}
	if(strstr(target_address, "http://")
	&& sscanf(target_address, "http://%s", host_path)
	&& strcmp(target_address, "http://")) {
		char *p;

		p = strchr(host_path, '/');
		if(p != NULL) {
			strcpy(path, p);
			*p = '\0';
			strcpy(host, host_path);
		}
		else {
			strcpy(host, host_path);
		}

		p = strchr(host, ':');
		if(p != NULL) {
			port = atoi(p + 1);
			if(port <= 0) {
				port = 80;
			}
			*p = '\0';
		}
	}
	else {
		ERR_PRINT("target_address (url) is invalid. %s", target_address);
		return;
	}

	servhost = gethostbyname(host);
	if(servhost == NULL) {
		ERR_PRINT("failed to get ip address from [%s]", host);
		return;
	}
	memset(&server, 0, sizeof(server));

	memcpy(&server.sin_addr, servhost->h_addr_list[0], servhost->h_length);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		ERR_PRINT("fail to make socket.");
		return;
	}
	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) == -1) {
		ERR_PRINT("failed to connect.");
	}
	else {
		fp = fdopen(sock, "r+");
		if(fp == NULL) {
			ERR_PRINT("failed to fdopen.");
		}
		else {
			setvbuf(fp, NULL, _IONBF, 0); // off the buffering.
			// send request header.
			fprintf(fp, "GET %s HTTP/1.0\r\n", path);
			fprintf(fp, "Host: %s:%d\r\n", host, port);
			if(range_start != 0) {
				if(range_length != 0) {
					fprintf(fp, "Range: bytes=%ld-%ld\r\n", range_start, range_start + range_length - 1);
				}
				else {
					fprintf(fp, "Range: bytes=%ld-\r\n", range_start);
				}
			}
			else {
				if(range_length != 0) {
					fprintf(fp, "Range: bytes=0-%ld\r\n", range_length - 1);
				}
			}
			fprintf(fp, "\r\n");

			char buf[BUF_LEN];
			char tag[BUF_LEN];
			char data[BUF_LEN];
			int count = 0;
			bool is_body = false;
			sprintf(client->ETag, "");
			client->content_length = 0;
			client->file_length = 0;
			int dummy1, dummy2;
			// get response
			while(1) {
				if(!is_body) {
					if(fgets(buf, sizeof(buf), fp) == NULL) {
						break;
					}
					if(strlen(buf) == 2) {
						// now ready to get body content.
						is_body = true;
					}
					if(strstr(buf, "HTTP") != NULL) {
						float dummy = 0;
						sscanf(buf, "HTTP/%f %d", &dummy, &client->status_code);
					}
					if(strstr(buf, "ETag") != NULL) {
						sscanf(buf, "ETag: \"%s", client->ETag);
						int length = strlen(client->ETag);
						client->ETag[length - 1] = 0x00;
					}
					else if(strstr(buf, "Content-Length") != NULL) {
						sscanf(buf, "Content-Length: %ld", &client->content_length);
						if(client->file_length == 0) {
							client->file_length = client->content_length;
						}
					}
					else if(strstr(buf, "Content-Range") != NULL) {
						sscanf(buf, "Content-Range: bytes %d-%d/%ld", &dummy1, &dummy2, &client->file_length);
					}
				}
				else {
					if(client->status_code < 200 || client->status_code >= 300) {
						return;
					}
					// body content.
					size_t read_size = 0;
					if(is_binary) {
						if((read_size = fread(client_->buffer, 1, client->buffer_size, fp)) == 0) {
							break;
						}
						callback(ptr, client, client_->buffer, read_size);
					}
					else {
						if(fgets((char *)client_->buffer, client->buffer_size, fp) == NULL) {
							break;
						}
						callback(ptr, client, client_->buffer, strlen((char *)client_->buffer));
					}
					if(client->wait_interval != 0) {
						struct timespec ts;
						ts.tv_sec = (client->wait_interval / 1000);
						ts.tv_nsec = (client->wait_interval % 1000) * 1000000;
						nanosleep(&ts, NULL);
					}
				}
			}
			fclose(fp);
		}
	}
	close(sock);
}

/*
 * close http client
 * @param client
 */
void TT_VISIBILITY_DEFAULT ttLibC_HttpClient_close(ttLibC_HttpClient **client) {
	ttLibC_HttpClient_ *target = (ttLibC_HttpClient_ *)*client;
	if(target == NULL) {
		return;
	}
	if(target->buffer) {
		ttLibC_free(target->buffer);
		target->buffer = NULL;
	}
	ttLibC_free(target);
	*client = NULL;
}

#endif


