/*
 * switch_config.h
 *
 *  Created on: 19 feb. 2020
 *      Author: Jan
 */

#ifndef MAIN_SWITCH_CONFIG_H_
#define MAIN_SWITCH_CONFIG_H_

#define VERSION			"v3.1.0"

#define STARTPAUSE		10

#define RESP200 		"200 OK"
#define RESP400 		"400 Bad Request"
#define RESP408 		"408 Request Timeout"
#define RESP413 		"413 Payload Too Large"
#define RESP499 		"499 Unknown Error"
#define RESP500 		"500 Internal Server Error"

#define TYPE_JSON		"application/json"

typedef unsigned char   bool;
#define false			0
#define true			1

typedef unsigned char       uint8;
typedef unsigned char       u8;
typedef signed char         sint8;
typedef signed char         int8;
typedef signed char         s8;
typedef unsigned short      uint16;
typedef unsigned short      u16;
typedef signed short        sint16;
typedef signed short        s16;
typedef unsigned int        uint32;
typedef unsigned int        u_int;
typedef unsigned int        u32;
typedef signed int          sint32;
typedef signed int          s32;
typedef int                 int32;
typedef signed long long    sint64;
typedef unsigned long long  uint64;
typedef unsigned long long  u64;
typedef float               real32;
typedef double              real64;

#endif /* MAIN_SWITCH_CONFIG_H_ */
