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

typedef struct {
	ttLibC_Amf0Object inherit_super;
	size_t data_size;
} ttLibC_Util_Amf0Object_;

typedef ttLibC_Util_Amf0Object_ ttLibC_Amf0Object_;

static void Amf0Object_close(ttLibC_Amf0Object_ **amf0_obj) {
	ttLibC_Amf0Object_ *target = *amf0_obj;
	if(target == NULL) {
		return;
	}
	// close the holding object.
	switch(target->inherit_super.type) {
	case amf0Type_Number:
		break;
	case amf0Type_Boolean:
		break;
	case amf0Type_String:
		break;
//	case amf0Type_Object:
//	case amf0Type_MovieClip:
//	case amf0Type_Null:
//	case amf0Type_Undefined:
//	case amf0Type_Reference:
	case amf0Type_Map:
		{
			ttLibC_Amf0MapObject *map_objects = target->inherit_super.object;
			int i = 0;
			while(map_objects[i].key != NULL && map_objects[i].amf0_obj != NULL) {
				if(map_objects[i].key != NULL) {
					free(map_objects[i].key);
				}
				Amf0Object_close((ttLibC_Amf0Object_ **)&map_objects[i].amf0_obj);
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
	free(target->inherit_super.object);
	free(target);
	*amf0_obj = NULL;
}

/**
 * make and reply amf0object
 */
static ttLibC_Amf0Object_ *Amf0Object_make(uint8_t *data, size_t data_size) {
	ttLibC_Amf0Object_ *amf0_obj = malloc(sizeof(ttLibC_Amf0Object_));
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
			amf0_obj->inherit_super.type = amf0Type_Number;
			amf0_obj->inherit_super.object = number;
			amf0_obj->data_size = read_size;
		}
		return amf0_obj;
	case amf0Type_Boolean:
		{
			++ read_size;
			uint8_t *value = malloc(1);
			*value = *(data + read_size);
			++ read_size;
			amf0_obj->inherit_super.type = amf0Type_Boolean;
			amf0_obj->inherit_super.object = value;
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
			amf0_obj->inherit_super.type = amf0Type_String;
			amf0_obj->inherit_super.object = string;
			amf0_obj->data_size = read_size;
		}
		return amf0_obj;
	case amf0Type_Object:
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
				ttLibC_Amf0Object_ *amf0_obj = Amf0Object_make(data + read_size, data_size - read_size);
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
			amf0_obj->data_size = read_size;
			amf0_obj->inherit_super.type = amf0Type_Map;
			amf0_obj->inherit_super.object = map_objects;
			// check the end of data. should be 00 00 09.
			if(*(data + read_size)     != 0x00
			|| *(data + read_size + 1) != 0x00
			|| *(data + read_size + 2) != 0x09) {
				ERR_PRINT("object end is corrupted.");
				Amf0Object_close(&amf0_obj);
				return NULL;
			}
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

bool ttLibC_Amf0Object_read(void *data, size_t data_size, ttLibC_Amf0ObjectReadFunc callback, void *ptr) {
	while(data_size > 0) {
		ttLibC_Amf0Object_ *amf0_obj = Amf0Object_make(data, data_size);
		if(amf0_obj == NULL) {
			return false;
		}
		bool result = callback(ptr, (ttLibC_Amf0Object *)amf0_obj);
		data_size -= amf0_obj->data_size;
		// close used object.
		Amf0Object_close(&amf0_obj);
		if(!result) {
			return false;
		}
	}
	return true;
}

bool ttLibC_Amf0Object_write(ttLibC_Amf0Object *object, ttLibC_AmfObjectWriteFunc callback, void *ptr) {
	return true;
}



