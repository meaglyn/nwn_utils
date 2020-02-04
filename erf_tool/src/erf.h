/*
 * Neverwinter Nights ERF Utility
 * Version: 1.1
 *
 * Copyright (C) 2003, Gareth Hughes <roboius@dladventures.net>
 * Version: 1.2
 *
 * Copyright (C) 2014, Meaglyn <meaglyn.nwn@gmail.com>
 *  	
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __erf_h_
#define __erf_h_

#include <sys/types.h>

#ifdef WIN32
typedef unsigned int u_int32_t;
#endif

typedef u_int32_t u32;

/*
 * Main file header structure.
 */
struct erf_header {
    char file_type[4];
    char version[4];

    u32 string_count;
    u32 string_size;

    u32 entry_count;

    u32 string_offset;
    u32 key_offset;
    u32 resource_offset;

    u32 build_year;
    u32 build_day;
    u32 type;

    u32 __spare[29];
};

struct erf_string {
    u32 language;
    u32 length;
    char data[0];
};

struct erf_key {
    char name[16];
    u32 index;
    u32 type;
};

struct erf_data {
    u32 offset;
    u32 length;
};

struct erf_resource {
    struct erf_key key;
    struct erf_data data;
    char buffer[0];
};

struct erf {
    char *name;
    char extension[4];
    struct erf_header header;
    struct erf_string *strings;
    struct erf_key *keys;
    struct erf_data *data;
    struct erf_resource **resources;
    u32 resource_max;
};

struct erf_resource_list {
    struct erf_resource_list *next;
    struct erf_resource **resources;
    u32 entry_count;
};

enum erf_resource_type {
    ERF_RESOURCE_TYPE_BMP = 0x0001,
    ERF_RESOURCE_TYPE_TGA = 0x0003,
    ERF_RESOURCE_TYPE_WAV = 0x0004,
    ERF_RESOURCE_TYPE_PLT = 0x0006,
    ERF_RESOURCE_TYPE_INI = 0x0007,
    ERF_RESOURCE_TYPE_BMU = 0x0008,
    ERF_RESOURCE_TYPE_TXT = 0x000a,
    ERF_RESOURCE_TYPE_MDL = 0x07d2,
    ERF_RESOURCE_TYPE_NSS = 0x07d9,
    ERF_RESOURCE_TYPE_NCS = 0x07da,
    ERF_RESOURCE_TYPE_MOD = 0x07db,
    ERF_RESOURCE_TYPE_ARE = 0x07dc,
    ERF_RESOURCE_TYPE_SET = 0x07dd,
    ERF_RESOURCE_TYPE_IFO = 0x07de,
    ERF_RESOURCE_TYPE_BIC = 0x07df,
    ERF_RESOURCE_TYPE_WOK = 0x07e0,
    ERF_RESOURCE_TYPE_2DA = 0x07e1,
    ERF_RESOURCE_TYPE_TXI = 0x07e6,
    ERF_RESOURCE_TYPE_GIT = 0x07e7,
    ERF_RESOURCE_TYPE_UTI = 0x07e9,
    ERF_RESOURCE_TYPE_UTC = 0x07eb,
    ERF_RESOURCE_TYPE_DLG = 0x07ed,
    ERF_RESOURCE_TYPE_ITP = 0x07ee,
    ERF_RESOURCE_TYPE_UTT = 0x07f0,
    ERF_RESOURCE_TYPE_DDS = 0x07f1,
    ERF_RESOURCE_TYPE_UTS = 0x07f3,
    ERF_RESOURCE_TYPE_LTR = 0x07f4,
    ERF_RESOURCE_TYPE_GFF = 0x07f5,
    ERF_RESOURCE_TYPE_FAC = 0x07f6,
    ERF_RESOURCE_TYPE_UTE = 0x07f8,
    ERF_RESOURCE_TYPE_UTD = 0x07fa,
    ERF_RESOURCE_TYPE_UTP = 0x07fc,
    ERF_RESOURCE_TYPE_DFT = 0x07fd,
    ERF_RESOURCE_TYPE_GIC = 0x07fe,
    ERF_RESOURCE_TYPE_GUI = 0x07ff,
    ERF_RESOURCE_TYPE_UTM = 0x0803,
    ERF_RESOURCE_TYPE_DWK = 0x0804,
    ERF_RESOURCE_TYPE_PWK = 0x0805,
    ERF_RESOURCE_TYPE_JRL = 0x0808,
    ERF_RESOURCE_TYPE_SAV = 0x0809,
    ERF_RESOURCE_TYPE_UTW = 0x080a,
    ERF_RESOURCE_TYPE_SSF = 0x080c,
    ERF_RESOURCE_TYPE_HAK = 0x080d,
    ERF_RESOURCE_TYPE_NWM = 0x080e,
    ERF_RESOURCE_TYPE_PTM = 0x0811,
    ERF_RESOURCE_TYPE_PTT = 0x0812,
    ERF_RESOURCE_TYPE_BAK = 0x0813,
    ERF_RESOURCE_TYPE_DAT = 0x0814,
    ERF_RESOURCE_TYPE_SHD = 0x0815,
    ERF_RESOURCE_TYPE_XBC = 0x0816,
    ERF_RESOURCE_TYPE_WBM = 0x0817,
    ERF_RESOURCE_TYPE_MTR = 0x0818,
    ERF_RESOURCE_TYPE_KTX = 0x0819,
    ERF_RESOURCE_TYPE_TTF = 0x0820,
    ERF_RESOURCE_TYPE_SQL = 0x0821,
    ERF_RESOURCE_TYPE_TML = 0x0822,
    ERF_RESOURCE_TYPE_SQ3 = 0x0823,
    ERF_RESOURCE_TYPE_IDS = 0x270c,
    ERF_RESOURCE_TYPE_ERF = 0x270d,
    ERF_RESOURCE_TYPE_BIF = 0x270e,
    ERF_RESOURCE_TYPE_KEY = 0x270f,
    ERF_RESOURCE_TYPE_UNKNOWN = 0xffff,
};

/* FIXME: Add proper support for other languages.
 */
enum erf_language_type {
    ERF_LANGUAGE_ENGLISH = 0,
    ERF_LANGUAGE_UNKNOWN = 0xffff,
};

#endif /* __erf_h_ */
