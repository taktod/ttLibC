/**
 * @file   amfUtil.h
 * @brief  util for amf object.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/31
 */

#ifndef TTLIBC_UTIL_AMFUTIL_H_
#define TTLIBC_UTIL_AMFUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include <stdio.h>
#include <stdbool.h>

typedef bool (* ttLibC_AmfObjectWriteFunc)(void *ptr, void *amf_data, size_t amf_data_size);

/**
 * enum def for amf0 object type.
 */
typedef enum ttLibC_Amf0_Type {
	amf0Type_Number      = 0x00, // 00 8byte double bits.
	amf0Type_Boolean     = 0x01, // 01 01 true 01 00 false
	amf0Type_String      = 0x02, // 02 si ze data
	amf0Type_Object      = 0x03, // 03 ([2byte(size) data type data] x num) 00 00 09(eof)
	amf0Type_MovieClip   = 0x04, // unsupported.
	amf0Type_Null        = 0x05, // 05
	amf0Type_Undefined   = 0x06, // 06
	amf0Type_Reference   = 0x07, // unsupported
	amf0Type_Map         = 0x08, // 08 ([4byte(num)(big endian)] ([2byte(size) key] [amf0Type data] x num) 00 00 09)
	amf0Type_ObjectEnd   = 0x09, // 09
	amf0Type_Array       = 0x0A, // 0A amf0data x num 00 00 09
	amf0Type_Date        = 0x0B, // 0B 8byte(double uniztime) 2byte timezone?
	amf0Type_LongString  = 0x0C, // 0C _s _i _z _e data
	amf0Type_Unsupported = 0x0D, // 0D
	amf0Type_RecordSet   = 0x0E, // 0E unsupported
	amf0Type_XmlDocument = 0x0F, // 0F
	amf0Type_TypedObject = 0x10, // 10
	amf0Type_Amf3Object  = 0x11, // 11
} ttLibC_Amf0_Type;

/**
 * def for amf0 object.
 */
typedef struct ttLibC_Util_Amf0Object {
	ttLibC_Amf0_Type type;
	void *object;
	size_t data_size;
} ttLibC_Util_Amf0Object;

typedef ttLibC_Util_Amf0Object ttLibC_Amf0Object;

/**
 * def for amf0 map object.
 */
typedef struct ttLibC_Util_Amf0MapObject {
	char *key;
	ttLibC_Amf0Object *amf0_obj;
} ttLibC_Util_Amf0MapObject;

typedef ttLibC_Util_Amf0MapObject ttLibC_Amf0MapObject;

typedef bool (* ttLibC_Amf0ObjectReadFunc)(void *ptr, ttLibC_Amf0Object *amf0_obj);

/**
 * make amf0 number object.
 * @param number
 * @return ttLibC_Amf0Object
 */
ttLibC_Amf0Object TT_ATTRIBUTE_API *ttLibC_Amf0_number(double number);

/**
 * make amf0 boolean object.
 * @param flag
 * @return ttLibC_Amf0Object
 */
ttLibC_Amf0Object TT_ATTRIBUTE_API *ttLibC_Amf0_boolean(bool flag);

/**
 * make amf0 string object.
 * @param string
 * @return ttLibC_Amf0Object
 */
ttLibC_Amf0Object TT_ATTRIBUTE_API *ttLibC_Amf0_string(const char *string);

/**
 * make amf0 map object.
 * @param list amf0Object key -> obj maplist.
 * @return ttLibC_Amf0Object
 */
ttLibC_Amf0Object TT_ATTRIBUTE_API *ttLibC_Amf0_map(ttLibC_Amf0MapObject *list);

/**
 * make amf0 null object.
 * @return ttLibC_Amf0Object
 */
ttLibC_Amf0Object TT_ATTRIBUTE_API *ttLibC_Amf0_null();

/**
 * make amf0 object object.
 * @param list amf0Object key -> obj maplist.
 * @return ttLibC_Amf0Object
 */
ttLibC_Amf0Object TT_ATTRIBUTE_API *ttLibC_Amf0_object(ttLibC_Amf0MapObject *list);

/**
 * get the amf0object from amf0object or amf0map.
 * @param amf0_map
 * @param key
 * @return ttLibC_Amf0Object
 */
ttLibC_Amf0Object TT_ATTRIBUTE_API *ttLibC_Amf0_getElement(ttLibC_Amf0Object *amf0_map, const char *key);

/**
 * make amf0 clone.
 * not yet.
 * @param number
 * @return ttLibC_Amf0Object
 */
ttLibC_Amf0Object TT_ATTRIBUTE_API *ttLibC_Amf0_clone(ttLibC_Amf0Object *src);

/**
 * read data from binary stream.
 * @param data      binary data
 * @param data_size data size
 * @param callback  callback func, which will call when found amf0object.
 * @param ptr       user def value pointer.
 * @return true:success false:abort.
 */
bool TT_ATTRIBUTE_API ttLibC_Amf0_read(void *data, size_t data_size, ttLibC_Amf0ObjectReadFunc callback, void *ptr);

/**
 * write amf0Object as binary data.
 * @param object   target amf0object.
 * @param callback callback func, which will call when need to write binary.
 * @param ptr      user def value pointer.
 * @return true:success false:abort.
 */
bool TT_ATTRIBUTE_API ttLibC_Amf0_write(ttLibC_Amf0Object *object, ttLibC_AmfObjectWriteFunc callback, void *ptr);

/**
 * close amf0Object
 * @param amf0_obj
 */
void TT_ATTRIBUTE_API ttLibC_Amf0_close(ttLibC_Amf0Object **amf0_obj);

typedef enum ttLibC_Amf3_Type {
	amf3Type_Undefined    = 0x00,
	amf3Type_Null         = 0x01,
	amf3Type_False        = 0x02,
	amf3Type_True         = 0x03,
	amf3Type_Integer      = 0x04,
	amf3Type_Double       = 0x05,
	amf3Type_String       = 0x06,
	amf3Type_XmlDoc       = 0x07,
	amf3Type_Date         = 0x08,
	amf3Type_Array        = 0x09,
	amf3Type_Object       = 0x0A,
	amf3Type_Xml          = 0x0B,
	amf3Type_BinaryArray  = 0x0C,
	amf3Type_VectorInt    = 0x0D,
	amf3Type_VectorUint   = 0x0E,
	amf3Type_VectorDouble = 0x0F,
	amf3Type_VectorObject = 0x10,
	amf3Type_Dictionary   = 0x11,
} ttLibC_Amf3_Type;

typedef struct ttLibC_Util_Amf3Object {
	ttLibC_Amf3_Type type;
} ttLibC_Util_Amf3Object;

typedef ttLibC_Util_Amf3Object ttLibC_Amf3Object;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_AMFUTIL_H_ */
