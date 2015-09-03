/*
 * @file   amfUtil.c
 * @brief  util for amf object.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/31
 */

#include "amfUtil.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../log.h"
#include "ioUtil.h"
#include "hexUtil.h"

ttLibC_Amf0Object *ttLibC_Amf0_map(ttLibC_Amf0MapObject *list) {
	uint32_t element_num = 0;
	for(int i = 0;list[i].key != NULL && list[i].amf0_obj != NULL;++ i) {
		++ element_num;
	}
	ttLibC_Amf0Object *obj = malloc(sizeof(ttLibC_Amf0Object));
	if(obj == NULL) {
		// TODO if failed, we need to clear data inside of mapObject.
		ERR_PRINT("failed to alloc memory for map object. need to clear list objects.");
		return NULL;
	}
	ttLibC_Amf0MapObject *map_list = malloc(sizeof(ttLibC_Amf0MapObject) * (element_num + 1));
	if(map_list == NULL) {
		ERR_PRINT("failed to alloc memory for list object. need to clear list objects.");
		free(obj);
		return NULL;
	}
	obj->data_size = 8;
	obj->type = amf0Type_Map;
	obj->object = (void *)map_list;
	int i = 0;
	for(i = 0;list[i].key != NULL && list[i].amf0_obj != NULL;++ i) {
		size_t size = strlen(list[i].key);
		obj->data_size += 2 + size + list[i].amf0_obj->data_size;
		char *key = malloc(size + 1);
		if(key == NULL) {
			ERR_PRINT("failed to allocate key object.");
			// need to clear already allocated key objects.
			free(map_list);
			free(obj);
			return NULL;
		}
		memcpy(key, list[i].key, size);
		key[size] = 0x00;
		map_list[i].key = key;
		map_list[i].amf0_obj = list[i].amf0_obj;
	}
	map_list[i].key = NULL;
	map_list[i].amf0_obj = NULL;
	return (ttLibC_Amf0Object *)obj;
}

ttLibC_Amf0Object *ttLibC_Amf0_object(ttLibC_Amf0MapObject *list) {
	uint32_t element_num = 0;
	for(int i = 0;list[i].key != NULL && list[i].amf0_obj != NULL;++ i) {
		++ element_num;
	}
	ttLibC_Amf0Object *obj = malloc(sizeof(ttLibC_Amf0Object));
	if(obj == NULL) {
		// TODO if failed, we need to clear data inside of mapObject.
		ERR_PRINT("failed to alloc memory for map object. need to clear list objects.");
		return NULL;
	}
	ttLibC_Amf0MapObject *map_list = malloc(sizeof(ttLibC_Amf0MapObject) * (element_num + 1));
	if(map_list == NULL) {
		ERR_PRINT("failed to alloc memory for list object. need to clear list objects.");
		free(obj);
		return NULL;
	}
	obj->data_size = 4;
	obj->type = amf0Type_Object;
	obj->object = (void *)map_list;
	int i = 0;
	for(i = 0;list[i].key != NULL && list[i].amf0_obj != NULL;++ i) {
		size_t size = strlen(list[i].key);
		obj->data_size += 2 + size + list[i].amf0_obj->data_size;
		char *key = malloc(size + 1);
		if(key == NULL) {
			ERR_PRINT("failed to allocate key object.");
			// need to clear already allocated key objects.
			free(map_list);
			free(obj);
			return NULL;
		}
		memcpy(key, list[i].key, size);
		key[size] = 0x00;
		map_list[i].key = key;
		map_list[i].amf0_obj = list[i].amf0_obj;
	}
	map_list[i].key = NULL;
	map_list[i].amf0_obj = NULL;
	return (ttLibC_Amf0Object *)obj;
}

ttLibC_Amf0Object *ttLibC_Amf0_number(double number) {
	ttLibC_Amf0Object *obj = malloc(sizeof(ttLibC_Amf0Object));
	if(obj == NULL) {
		return NULL;
	}
	obj->data_size = 9;
	obj->type = amf0Type_Number;
	uint8_t *buf = (uint8_t *)malloc(8);
	memcpy(buf, &number, 8);
	obj->object = buf;
	return (ttLibC_Amf0Object *)obj;
}

ttLibC_Amf0Object *ttLibC_Amf0_boolean(bool flag) {
	ttLibC_Amf0Object *obj = malloc(sizeof(ttLibC_Amf0Object));
	if(obj == NULL) {
		return NULL;
	}
	obj->data_size = 2;
	obj->type = amf0Type_Boolean;
	uint8_t *buf = (uint8_t *)malloc(1);
	if(flag) {
		*buf = 0x01;
	}
	else {
		*buf = 0x00;
	}
	obj->object = buf;
	return (ttLibC_Amf0Object *)obj;
}

ttLibC_Amf0Object *ttLibC_Amf0_string(char *string) {
	ttLibC_Amf0Object *obj = malloc(sizeof(ttLibC_Amf0Object));
	if(obj == NULL) {
		return NULL;
	}
	size_t size = strlen(string);
	obj->data_size = 3 + size;
	obj->type = amf0Type_String;
	uint8_t *buf = (uint8_t *)malloc(size + 1);
	memcpy(buf, string, size);
	buf[size] = 0x00;
	obj->object = buf;
	return (ttLibC_Amf0Object *)obj;
}

/**
 * get the amf0object from amf0object or amf0map.
 * @param amf0_map
 * @param key
 * @return ttLibC_Amf0Object
 */
ttLibC_Amf0Object *ttLibC_Amf0_getElement(ttLibC_Amf0Object *amf0_map, const char *key) {
	switch(amf0_map->type) {
	case amf0Type_Object:
	case amf0Type_Map:
		{
			ttLibC_Amf0MapObject *list = (ttLibC_Amf0MapObject *)amf0_map->object;
			for(int i = 0;list[i].key != NULL && list[i].amf0_obj != NULL;++ i) {
				if(strcmp(list[i].key, key) == 0) {
					return list[i].amf0_obj;
				}
			}
		}
		break;
	default:
		ERR_PRINT("try to get element for neither map nor object.");
		return NULL;
	}
	return NULL;
}

ttLibC_Amf0Object *ttLibC_Amf0_clone(ttLibC_Amf0Object *src) {
	LOG_PRINT("clone is not ready now.");
	return NULL;
}

/**
 * make and reply amf0object
 */
static ttLibC_Amf0Object *Amf0_make(uint8_t *data, size_t data_size) {
	ttLibC_Amf0Object *amf0_obj = malloc(sizeof(ttLibC_Amf0Object));
	if(amf0_obj == NULL) {
		ERR_PRINT("failed to make amf0object.");
		return NULL;
	}
	size_t read_size = 0;
	switch((*data)) {
	case amf0Type_Number:
		{
			++ read_size;
			// 8bit double, endian is bigendian.
			uint8_t *number = malloc(8);
			uint64_t be_val = be_uint64_t(*((uint64_t *)(data + read_size)));
			memcpy(number, &be_val, 8);
			read_size += 8;
			amf0_obj->type = amf0Type_Number;
			amf0_obj->object = number;
			amf0_obj->data_size = read_size;
		}
		return amf0_obj;
	case amf0Type_Boolean:
		{
			++ read_size;
			uint8_t *value = malloc(1);
			*value = *(data + read_size);
			++ read_size;
			amf0_obj->type = amf0Type_Boolean;
			amf0_obj->object = value;
			amf0_obj->data_size = read_size;
		}
		return amf0_obj;
	case amf0Type_String:
		{
			++ read_size;
			// byte for string size.
			uint16_t size = be_uint16_t(*((uint16_t *)(data + read_size)));
			read_size += 2;
			char *string = malloc(size + 1); // + 1 for null byte.
			memcpy(string, data + read_size, size);
			string[size] = 0x00;
			read_size += size;
			amf0_obj->type = amf0Type_String;
			amf0_obj->object = string;
			amf0_obj->data_size = read_size;
		}
		return amf0_obj;
	case amf0Type_Object:
		{
			++ read_size;
			// あとで作る
			// とりあえず保持要素数は、仮に256としておく。(そんなでかいobjectつくらないと思うけど。)
			size_t size = 255;
			ttLibC_Amf0MapObject *map_objects = malloc(sizeof(ttLibC_Amf0MapObject) * (size + 1));
			for(int i = 0;i < size;++ i) {
//				key
				uint16_t key_size = be_uint16_t(*((uint16_t *)(data + read_size)));
				if(key_size == 0) {
					// for the end, set key and object is NULL.
					map_objects[i].key = NULL;
					map_objects[i].amf0_obj = NULL;
					break;
				}
				read_size += 2;
				char *key = malloc(key_size + 1);
				memcpy(key, data + read_size, key_size);
				key[key_size] = 0x00;
				map_objects[i].key = key;
				read_size += key_size;
//				body
				ttLibC_Amf0Object *amf0_obj = Amf0_make(data + read_size, data_size - read_size);
				if(amf0_obj == NULL) {
					ERR_PRINT("failed to analyze object.:%x", *(data + read_size));
					return false;
				}
				map_objects[i].amf0_obj = (ttLibC_Amf0Object *)amf0_obj;
				read_size += amf0_obj->data_size;
			}
			// amf0_obj
			amf0_obj->type = amf0Type_Object;
			amf0_obj->object = map_objects;
			if(*(data + read_size)     != 0x00
			|| *(data + read_size + 1) != 0x00
			|| *(data + read_size + 2) != 0x09) {
				// ここでエラーになるはずだが・・・なんでだろう。
				ERR_PRINT("object end is corrupted.");
				ttLibC_Amf0_close((ttLibC_Amf0Object **)&amf0_obj);
				return NULL;
			}
			read_size += 3;
			amf0_obj->data_size = read_size;
		}
		return amf0_obj;
	case amf0Type_MovieClip:
	case amf0Type_Null:
	case amf0Type_Undefined:
	case amf0Type_Reference:
		free(amf0_obj);
		return NULL;
	case amf0Type_Map:
		{
			++ read_size;
			// get the element size.
			uint32_t size = be_uint32_t(*((uint32_t *)(data + read_size)));
			read_size += 4;
			// data holder.
			ttLibC_Amf0MapObject *map_objects = malloc(sizeof(ttLibC_Amf0MapObject) * (size + 1));
			for(int i = 0;i < size;++ i) {
//				key
				uint16_t key_size = be_uint16_t(*((uint16_t *)(data + read_size)));
				read_size += 2;
				char *key = malloc(key_size + 1);
				memcpy(key, data + read_size, key_size);
				key[key_size] = 0x00;
				map_objects[i].key = key;
				read_size += key_size;
//				body
				ttLibC_Amf0Object *amf0_obj = Amf0_make(data + read_size, data_size - read_size);
				if(amf0_obj == NULL) {
					ERR_PRINT("failed to analyze object.:%x", *(data + read_size));
					return false;
				}
				map_objects[i].amf0_obj = (ttLibC_Amf0Object *)amf0_obj;
				read_size += amf0_obj->data_size;
			}
			// for the end, set key and object is NULL.
			map_objects[size].key = NULL;
			map_objects[size].amf0_obj = NULL;
			// amf0_obj
			amf0_obj->type = amf0Type_Map;
			amf0_obj->object = map_objects;
			// check the end of data. should be 00 00 09.
			if(*(data + read_size)     != 0x00
			|| *(data + read_size + 1) != 0x00
			|| *(data + read_size + 2) != 0x09) {
				ERR_PRINT("object end is corrupted.");
				ttLibC_Amf0_close((ttLibC_Amf0Object **)&amf0_obj);
				return NULL;
			}
			read_size += 3;
			amf0_obj->data_size = read_size;
		}
		return amf0_obj;
	case amf0Type_ObjectEnd:
		// do nothing.
	case amf0Type_Array:
	case amf0Type_Date:
	case amf0Type_LongString:
	case amf0Type_Unsupported:
	case amf0Type_RecordSet:
	case amf0Type_XmlDocument:
	case amf0Type_TypedObject:
	case amf0Type_Amf3Object:
	default:
		free(amf0_obj);
		return NULL;
	}
	free(amf0_obj);
	return NULL;
}

bool ttLibC_Amf0_read(void *data, size_t data_size, ttLibC_Amf0ObjectReadFunc callback, void *ptr) {
	while(data_size > 0) {
		ttLibC_Amf0Object *amf0_obj = Amf0_make(data, data_size);
		if(amf0_obj == NULL) {
			ERR_PRINT("failed to get object.");
			return false;
		}
		bool result = callback(ptr, (ttLibC_Amf0Object *)amf0_obj);
		data_size -= amf0_obj->data_size;
		// close used object.
		ttLibC_Amf0_close((ttLibC_Amf0Object **)&amf0_obj);
		if(!result) {
			return false;
		}
	}
	return true;
}

static bool Amf0_write(ttLibC_Amf0Object *amf0_obj, ttLibC_AmfObjectWriteFunc callback, void *ptr) {
	switch(amf0_obj->type) {
	case amf0Type_Number:
		{
			uint8_t buf[9];
			buf[0] = amf0Type_Number;
			uint64_t num = be_uint64_t(*((uint64_t *)amf0_obj->object));
			memcpy(buf + 1, &num, 8);
			if(!callback(ptr, buf, 9)) {
				return false;
			}
		}
		return true;
	case amf0Type_Boolean:
		{
			uint8_t buf[2];
			buf[0] = amf0Type_Boolean;
			buf[1] = *((uint8_t *)amf0_obj->object);
			if(!callback(ptr, buf, 2)) {
				return false;
			}
		}
		break;
	case amf0Type_String:
		{
			uint8_t buf[3];
			buf[0] = amf0Type_String;
			uint16_t str_size = be_uint16_t(strlen((char *)amf0_obj->object));
			memcpy(buf + 1, &str_size, 2);
			if(!callback(ptr, buf, 3)) {
				return false;
			}
			str_size = be_uint16_t(str_size);
			if(!callback(ptr, amf0_obj->object, str_size)) {
				return false;
			}
		}
		break;
	case amf0Type_Object:
		{
			uint8_t buf[3];
			buf[0] = amf0Type_Object;
			if(!callback(ptr, buf, 1)) { // type
				return false;
			}
			ttLibC_Amf0MapObject *lists = (ttLibC_Amf0MapObject *)amf0_obj->object;
			for(int i = 0;lists[i].key != NULL && lists[i].amf0_obj != NULL;++ i) {
				uint16_t key_size = be_uint16_t(strlen(lists[i].key));
				memcpy(buf, &key_size, 2);
				if(!callback(ptr, buf, 2)) { // keysize
					return false;
				}
				key_size = be_uint16_t(key_size);
				if(!callback(ptr, lists[i].key, key_size)) { // key
					return false;
				}
				// あとは、objectを書き込めばOK
				if(!Amf0_write(lists[i].amf0_obj, callback, ptr)) { // amfObject
					return false;
				}
			}
			buf[0] = 0x00;
			buf[1] = 0x00;
			buf[2] = amf0Type_ObjectEnd;
			if(!callback(ptr, buf, 3)) { // write object end.
				return false;
			}
		}
		break;
//	case amf0Type_MovieClip:
//	case amf0Type_Null:
//	case amf0Type_Undefined:
//	case amf0Type_Reference:
	case amf0Type_Map:
		{
			uint8_t buf[5];
			buf[0] = amf0Type_Map;
			uint32_t element_num = 0;
			ttLibC_Amf0MapObject *lists = (ttLibC_Amf0MapObject *)amf0_obj->object;
			for(int i = 0;lists[i].key != NULL && lists[i].amf0_obj != NULL;++ i) {
				++ element_num;
			}
			element_num = be_uint32_t(element_num);
			memcpy(buf + 1, &element_num, 4);
			if(!callback(ptr, buf, 5)) { // type and elementNum
				return false;
			}
			for(int i = 0;lists[i].key != NULL && lists[i].amf0_obj != NULL;++ i) {
				uint16_t key_size = be_uint16_t(strlen(lists[i].key));
				memcpy(buf, &key_size, 2);
				if(!callback(ptr, buf, 2)) { // keysize
					return false;
				}
				key_size = be_uint16_t(key_size);
				if(!callback(ptr, lists[i].key, key_size)) { // key
					return false;
				}
				// あとは、objectを書き込めばOK
				if(!Amf0_write(lists[i].amf0_obj, callback, ptr)) { // amfObject
					return false;
				}
			}
			buf[0] = 0x00;
			buf[1] = 0x00;
			buf[2] = amf0Type_ObjectEnd;
			if(!callback(ptr, buf, 3)) { // write object end.
				return false;
			}
		}
		break;
//	case amf0Type_ObjectEnd:
//	case amf0Type_Array:
//	case amf0Type_Date:
//	case amf0Type_LongString:
//	case amf0Type_Unsupported:
//	case amf0Type_RecordSet:
//	case amf0Type_XmlDocument:
//	case amf0Type_TypedObject:
//	case amf0Type_Amf3Object:
	default:
		break;
	}
	return true;
}

bool ttLibC_Amf0_write(ttLibC_Amf0Object *amf0_obj, ttLibC_AmfObjectWriteFunc callback, void *ptr) {
	if(!Amf0_write(amf0_obj, callback, ptr)) {
		return false;
	}
	return true;
}

void ttLibC_Amf0_close(ttLibC_Amf0Object **amf0_obj) {
	ttLibC_Amf0Object *target = *amf0_obj;
	if(target == NULL) {
		return;
	}
	// close the holding object.
	switch(target->type) {
	case amf0Type_Number:
		break;
	case amf0Type_Boolean:
		break;
	case amf0Type_String:
		break;
//	case amf0Type_MovieClip:
//	case amf0Type_Null:
//	case amf0Type_Undefined:
//	case amf0Type_Reference:
	case amf0Type_Object:
	case amf0Type_Map:
		{
			ttLibC_Amf0MapObject *map_objects = target->object;
			int i = 0;
			while(map_objects[i].key != NULL && map_objects[i].amf0_obj != NULL) {
				if(map_objects[i].key != NULL) {
					free(map_objects[i].key);
				}
				ttLibC_Amf0_close((ttLibC_Amf0Object **)&map_objects[i].amf0_obj);
				++ i;
			}
		}
		break;
//	case amf0Type_ObjectEnd:
//	case amf0Type_Array:
//	case amf0Type_Date:
//	case amf0Type_LongString:
//	case amf0Type_Unsupported:
//	case amf0Type_RecordSet:
//	case amf0Type_XmlDocument:
//	case amf0Type_TypedObject:
//	case amf0Type_Amf3Object:
	default:
		break;
	}
	free(target->object);
	free(target);
	*amf0_obj = NULL;
}
