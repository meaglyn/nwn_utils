/*
 * Neverwinter Nights ERF Utility
 *
 * Version: 1.1
 * Copyright (C) 2003, Gareth Hughes <roboius@dladventures.net>
 * 
 * Version: 1.2
 * Copyright (C) 2014, Meaglyn <meaglyn.nwn@gmail.com>
 * 
 * Version 1.3
 * Copyright (C) 2014,2016, Phil Auld "Meaglyn" <meaglyn.nwn@gmail.com>
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

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "erf.h"

#define ERF_VERSION     "1.3"
#define ERF_AUTHOR      "Meaglyn <meaglyn.nwn@gmail.com>"
#define ERF_COPYRIGHT   "Original Copyright (C) 2003, Gareth Hughes. Copyright (C) 2014,2016 Meaglyn"

#ifndef BYTE_ORDER
#define LITTLE_ENDIAN   1234
#define BIG_ENDIAN      4321
#define PDP_ENDIAN      3412

#define BYTE_ORDER      LITTLE_ENDIAN
#endif

/*
 * FIXME: Actually take care of endian crap.
 */
#if BYTE_ORDER == LITTLE_ENDIAN
#define CPU_TO_LE32(x) ((u32)(x))
#else
#define CPU_TO_LE32(x) \
    ((u32)((((u32)(x) & (u32)0x000000ff) << 24) | \
           (((u32)(x) & (u32)0x0000ff00) <<  8) | \
           (((u32)(x) & (u32)0x00ff0000) >>  8) | \
           (((u32)(x) & (u32)0xff000000) >> 24) ))
#endif

#define ERF_TYPE        0x0a55f4ce      /* Bonus points for figuring this out */
#define MOD_TYPE        0xffffffff

typedef enum {
    COMMAND_DEFAULT = 0,
    COMMAND_CREATE,
    COMMAND_EXTRACT,
    COMMAND_LIST,
    COMMAND_UPDATE,
} command_t;

static char verbose = 0;
static char showdesc = 0;

/* Format is "Title\nURL\nDescription\0".
 */
static char default_string[] =
    "\n\nCreated by \"erf\", the command-line ERF utility.\n" ERF_COPYRIGHT;

char * description_string = default_string;

/**********************************************************************/

static void fatal_error(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);

    exit(1);
}

static const char *get_extension_from_type(u32 type)
{
    switch (type) {
    case ERF_RESOURCE_TYPE_BMP: return ".bmp";
    case ERF_RESOURCE_TYPE_TGA: return ".tga";
    case ERF_RESOURCE_TYPE_WAV: return ".wav";
    case ERF_RESOURCE_TYPE_PLT: return ".plt";
    case ERF_RESOURCE_TYPE_INI: return ".ini";
    case ERF_RESOURCE_TYPE_BMU: return ".bmu";
    case ERF_RESOURCE_TYPE_TXT: return ".txt";
    case ERF_RESOURCE_TYPE_MDL: return ".mdl";
    case ERF_RESOURCE_TYPE_NSS: return ".nss";
    case ERF_RESOURCE_TYPE_NCS: return ".ncs";
    case ERF_RESOURCE_TYPE_MOD: return ".mod";
    case ERF_RESOURCE_TYPE_ARE: return ".are";
    case ERF_RESOURCE_TYPE_SET: return ".set";
    case ERF_RESOURCE_TYPE_IFO: return ".ifo";
    case ERF_RESOURCE_TYPE_BIC: return ".bic";
    case ERF_RESOURCE_TYPE_WOK: return ".wok";
    case ERF_RESOURCE_TYPE_2DA: return ".2da";
    case ERF_RESOURCE_TYPE_TXI: return ".txi";
    case ERF_RESOURCE_TYPE_GIT: return ".git";
    case ERF_RESOURCE_TYPE_UTI: return ".uti";
    case ERF_RESOURCE_TYPE_UTC: return ".utc";
    case ERF_RESOURCE_TYPE_DLG: return ".dlg";
    case ERF_RESOURCE_TYPE_ITP: return ".itp";
    case ERF_RESOURCE_TYPE_UTT: return ".utt";
    case ERF_RESOURCE_TYPE_DDS: return ".dds";
    case ERF_RESOURCE_TYPE_UTS: return ".uts";
    case ERF_RESOURCE_TYPE_LTR: return ".ltr";
    case ERF_RESOURCE_TYPE_GFF: return ".gff";
    case ERF_RESOURCE_TYPE_FAC: return ".fac";
    case ERF_RESOURCE_TYPE_UTE: return ".ute";
    case ERF_RESOURCE_TYPE_UTD: return ".utd";
    case ERF_RESOURCE_TYPE_UTP: return ".utp";
    case ERF_RESOURCE_TYPE_DFT: return ".dft";
    case ERF_RESOURCE_TYPE_GIC: return ".gic";
    case ERF_RESOURCE_TYPE_GUI: return ".gui";
    case ERF_RESOURCE_TYPE_UTM: return ".utm";
    case ERF_RESOURCE_TYPE_DWK: return ".dwk";
    case ERF_RESOURCE_TYPE_PWK: return ".pwk";
    case ERF_RESOURCE_TYPE_JRL: return ".jrl";
    case ERF_RESOURCE_TYPE_SAV: return ".sav";
    case ERF_RESOURCE_TYPE_UTW: return ".utw";
    case ERF_RESOURCE_TYPE_SSF: return ".ssf";
    case ERF_RESOURCE_TYPE_HAK: return ".hak";
    case ERF_RESOURCE_TYPE_NWM: return ".nwm";
    case ERF_RESOURCE_TYPE_PTM: return ".ptm";
    case ERF_RESOURCE_TYPE_PTT: return ".ptt";
    case ERF_RESOURCE_TYPE_ERF: return ".erf";
    case ERF_RESOURCE_TYPE_BIF: return ".bif";
    case ERF_RESOURCE_TYPE_KEY: return ".key";
    case ERF_RESOURCE_TYPE_UNKNOWN:
    default:
        return ".unk";
    }
}

static u32 get_type_from_extension(const char *ext)
{
    if (ext[0] == '.') {
        ext++;
    }

    if (strncmp(ext, "bmp", 3) == 0) return ERF_RESOURCE_TYPE_BMP;
    if (strncmp(ext, "tga", 3) == 0) return ERF_RESOURCE_TYPE_TGA;
    if (strncmp(ext, "wav", 3) == 0) return ERF_RESOURCE_TYPE_WAV;
    if (strncmp(ext, "plt", 3) == 0) return ERF_RESOURCE_TYPE_PLT;
    if (strncmp(ext, "ini", 3) == 0) return ERF_RESOURCE_TYPE_INI;
    if (strncmp(ext, "bmu", 3) == 0) return ERF_RESOURCE_TYPE_BMU;
    if (strncmp(ext, "txt", 3) == 0) return ERF_RESOURCE_TYPE_TXT;
    if (strncmp(ext, "mdl", 3) == 0) return ERF_RESOURCE_TYPE_MDL;
    if (strncmp(ext, "nss", 3) == 0) return ERF_RESOURCE_TYPE_NSS;
    if (strncmp(ext, "ncs", 3) == 0) return ERF_RESOURCE_TYPE_NCS;
    if (strncmp(ext, "mod", 3) == 0) return ERF_RESOURCE_TYPE_MOD;
    if (strncmp(ext, "are", 3) == 0) return ERF_RESOURCE_TYPE_ARE;
    if (strncmp(ext, "set", 3) == 0) return ERF_RESOURCE_TYPE_SET;
    if (strncmp(ext, "ifo", 3) == 0) return ERF_RESOURCE_TYPE_IFO;
    if (strncmp(ext, "bic", 3) == 0) return ERF_RESOURCE_TYPE_BIC;
    if (strncmp(ext, "wok", 3) == 0) return ERF_RESOURCE_TYPE_WOK;
    if (strncmp(ext, "2da", 3) == 0) return ERF_RESOURCE_TYPE_2DA;
    if (strncmp(ext, "txi", 3) == 0) return ERF_RESOURCE_TYPE_TXI;
    if (strncmp(ext, "git", 3) == 0) return ERF_RESOURCE_TYPE_GIT;
    if (strncmp(ext, "uti", 3) == 0) return ERF_RESOURCE_TYPE_UTI;
    if (strncmp(ext, "utc", 3) == 0) return ERF_RESOURCE_TYPE_UTC;
    if (strncmp(ext, "dlg", 3) == 0) return ERF_RESOURCE_TYPE_DLG;
    if (strncmp(ext, "itp", 3) == 0) return ERF_RESOURCE_TYPE_ITP;
    if (strncmp(ext, "utt", 3) == 0) return ERF_RESOURCE_TYPE_UTT;
    if (strncmp(ext, "dds", 3) == 0) return ERF_RESOURCE_TYPE_DDS;
    if (strncmp(ext, "uts", 3) == 0) return ERF_RESOURCE_TYPE_UTS;
    if (strncmp(ext, "ltr", 3) == 0) return ERF_RESOURCE_TYPE_LTR;
    if (strncmp(ext, "gff", 3) == 0) return ERF_RESOURCE_TYPE_GFF;
    if (strncmp(ext, "fac", 3) == 0) return ERF_RESOURCE_TYPE_FAC;
    if (strncmp(ext, "ute", 3) == 0) return ERF_RESOURCE_TYPE_UTE;
    if (strncmp(ext, "utd", 3) == 0) return ERF_RESOURCE_TYPE_UTD;
    if (strncmp(ext, "utp", 3) == 0) return ERF_RESOURCE_TYPE_UTP;
    if (strncmp(ext, "dft", 3) == 0) return ERF_RESOURCE_TYPE_DFT;
    if (strncmp(ext, "gic", 3) == 0) return ERF_RESOURCE_TYPE_GIC;
    if (strncmp(ext, "gui", 3) == 0) return ERF_RESOURCE_TYPE_GUI;
    if (strncmp(ext, "utm", 3) == 0) return ERF_RESOURCE_TYPE_UTM;
    if (strncmp(ext, "dwk", 3) == 0) return ERF_RESOURCE_TYPE_DWK;
    if (strncmp(ext, "pwk", 3) == 0) return ERF_RESOURCE_TYPE_PWK;
    if (strncmp(ext, "jrl", 3) == 0) return ERF_RESOURCE_TYPE_JRL;
    if (strncmp(ext, "sav", 3) == 0) return ERF_RESOURCE_TYPE_SAV;
    if (strncmp(ext, "utw", 3) == 0) return ERF_RESOURCE_TYPE_UTW;
    if (strncmp(ext, "ssf", 3) == 0) return ERF_RESOURCE_TYPE_SSF;
    if (strncmp(ext, "hak", 3) == 0) return ERF_RESOURCE_TYPE_HAK;
    if (strncmp(ext, "nwm", 3) == 0) return ERF_RESOURCE_TYPE_NWM;
    if (strncmp(ext, "ptm", 3) == 0) return ERF_RESOURCE_TYPE_PTM;
    if (strncmp(ext, "ptt", 3) == 0) return ERF_RESOURCE_TYPE_PTT;
    if (strncmp(ext, "erf", 3) == 0) return ERF_RESOURCE_TYPE_ERF;
    if (strncmp(ext, "bif", 3) == 0) return ERF_RESOURCE_TYPE_BIF;
    if (strncmp(ext, "key", 3) == 0) return ERF_RESOURCE_TYPE_KEY;

    return ERF_RESOURCE_TYPE_UNKNOWN;
}

static void get_base_name(char dst[16], const char *src)
{
    const char *tmp;
    int ii;

    memset(dst, 0, 16);

    /* Try both types of path separators.
     */
    tmp = strrchr(src, '/');
    if (!tmp) {
        tmp = strrchr(src, '\\');
    }
    if (tmp) {
        tmp++;
    } else {
        tmp = src;
    }

    /* Silenty truncate to 16 characters.
     */
    sprintf(dst, "%.16s", tmp);

    /* Get rid of any extension in the base name.
     */
    tmp = strrchr(dst, '.');
    if (tmp) {
        for (ii = tmp - dst; ii < 16; ii++) {
            dst[ii] = 0;
        }
    }
}

static void get_extension(char dst[4], const char *src)
{
    const char *tmp;
    int ii;

    memset(dst, 0, 4);

    tmp = strrchr(src, '.');
    if (tmp) {
        for (ii = 0; ii < 4; ii++) {
            dst[ii] = tolower(tmp[ii]);
        }
    }
}

static u32 get_type_from_name(const char *name)
{
    char ext[4];
    u32 type;

    get_extension(ext, name);
    type = get_type_from_extension(ext);

    return type;
}

static int is_erf_file(const char *name)
{
    u32 type;

    type = get_type_from_name(name);

    switch (type) {
    case ERF_RESOURCE_TYPE_MOD:
    case ERF_RESOURCE_TYPE_SAV:
    case ERF_RESOURCE_TYPE_HAK:
    case ERF_RESOURCE_TYPE_NWM:
    case ERF_RESOURCE_TYPE_ERF:
        return 1;
    default:
        return 0;
    }
}

static char * get_unquoted_filename(char *file) {
	char * ret;

	if (*file == '"') {
		ret = file + 1;
		if (file[strlen(file) -1] == '"')
			file[strlen(file) -1] = 0;
		return ret;
	}
	return file;
}

/*
 * Used for bsearch(), qsort() calls.  Compare two resource, using their
 * full filenames.
 */
static int compare_resources(const void *a, const void *b)
{
    const struct erf_resource *res[2];
    const char *ext[2];
    int ret;

    res[0] = *(const struct erf_resource **)a;
    res[1] = *(const struct erf_resource **)b;

    /* Compare resource names first.
     */
    ret = strncmp(res[0]->key.name, res[1]->key.name, 16);

    if (ret == 0) {
        /* Compare extension strings second.
         */
        ext[0] = get_extension_from_type(res[0]->key.type);
        ext[1] = get_extension_from_type(res[1]->key.type);

        ret = strncmp(ext[0], ext[1], 4);
    }

    return ret;
}

/**********************************************************************/

/*
 * Populate the given erf structure with data from an ERF file.  The
 * name of the file to be opened should be in erf->file.
 */
static void erf_load(struct erf *erf)
{
    FILE *file;
    u32 count, ii;

    file = fopen(erf->name, "rb");
    if (!file) {
        fatal_error("unable to open file '%s'", erf->name);
    }

    if (fread(&erf->header, sizeof(struct erf_header), 1, file) < 1) {
        fatal_error("short read on ERF header");
    }

    /* Don't know if strings are mandatory.
     */
    if (erf->header.string_size) {
        erf->strings = (struct erf_string *) malloc(erf->header.string_size);

        fseek(file, erf->header.string_offset, SEEK_SET);
	if (fread(erf->strings, erf->header.string_size, 1, file)  < 1) {
		    fatal_error("short read on ERF strings");
        }
    }

    count = erf->resource_max = erf->header.entry_count;

    erf->keys = (struct erf_key *) malloc(count * sizeof(struct erf_key));
    erf->data = (struct erf_data *) malloc(count * sizeof(struct erf_data));
    erf->resources = (struct erf_resource **)
        malloc(count * sizeof(struct erf_resource *));

    if (!erf->keys || !erf->data || !erf->resources) {
        fatal_error("out of memory");
    }

    fseek(file, erf->header.key_offset, SEEK_SET);
    if (fread(erf->keys, sizeof(struct erf_key), count, file) < count) {
        fatal_error("short read on ERF key list");
    }
    fseek(file, erf->header.resource_offset, SEEK_SET);
    if (fread(erf->data, sizeof(struct erf_data), count, file) < count) {
        fatal_error("short read on ERF resource list");
    }

    for (ii = 0; ii < count; ii++) {
        struct erf_resource *res;
        u32 len = erf->data[ii].length;

        res = (struct erf_resource *)
            malloc(sizeof(struct erf_resource) + len);
        if (!res) {
            fatal_error("out of memory");
        }

        fseek(file, erf->data[ii].offset, SEEK_SET);
        if (fread(res->buffer, 1, len, file) < len) {
            fatal_error("short read on ERF resource data");
        }

        memcpy(res->key.name, erf->keys[ii].name, 16);
        res->key.index   = erf->keys[ii].index;
        res->key.type    = erf->keys[ii].type;
        res->data.offset = erf->data[ii].offset;
        res->data.length = erf->data[ii].length;

        erf->resources[ii] = res;
    }

    fclose(file);
}

/*
 * Build a resource list from the given files.  Note that the file list
 * must not contain any ERF files.
 */
static struct erf_resource_list *load_files(char **files, int num_files)
{
    struct erf_resource_list *list;
    FILE *file;
    int count, ii;

    list = (struct erf_resource_list *)
        malloc(sizeof(struct erf_resource_list));
    if (!list) {
        fatal_error("out of memory");
    }

    list->next = NULL;
    list->resources = (struct erf_resource **)
        malloc(num_files * sizeof(struct erf_resource *));
    if (!list->resources) {
        fatal_error("out of memory");
    }

    /* Read in all the input files.
     */
    for (ii = 0, count = 0; ii < num_files; ii++) {
        struct erf_resource *res;
        struct stat st;
        u32 type, len;

	files[ii] = get_unquoted_filename(files[ii]);

        type = get_type_from_name(files[ii]);

        if (type == ERF_RESOURCE_TYPE_MOD ||
            type == ERF_RESOURCE_TYPE_SAV ||
            type == ERF_RESOURCE_TYPE_HAK ||
            type == ERF_RESOURCE_TYPE_NWM ||
            type == ERF_RESOURCE_TYPE_ERF) {
		fatal_error("ERF file '%s' handled incorrectly", files[ii]);
        }
        if (type == ERF_RESOURCE_TYPE_UNKNOWN) {
            fprintf(stderr, "skiping non-NWN file '%s'\n", files[ii]);
            continue;
        }

        if (stat(files[ii], &st) < 0) {
            fprintf(stderr, "could not stat file '%s', skipping\n", files[ii]);
            continue;
        }
        if ((st.st_mode & S_IFMT) != S_IFREG) {
            fprintf(stderr, "skipping non-regular file '%s'\n", files[ii]);
            continue;
        }

        len = st.st_size;

        file = fopen(files[ii], "rb");
        if (!file) {
            fprintf(stderr, "could not read file '%s', skipping\n", files[ii]);
            continue;
        }

        res = (struct erf_resource *)
            malloc(sizeof(struct erf_resource) + len);
        if (!res) {
            fatal_error("out of memory");
        }

        if (fread(res->buffer, 1, len, file) < len) {
            fatal_error("short read on data for file '%s'", files[ii]);
        }
        fclose(file);

        if (verbose) {
            printf("Loaded file %s\n", files[ii]);
        }

        get_base_name(res->key.name, files[ii]);
        res->key.index = 0;
        res->key.type = type;
        res->data.offset = 0;
        res->data.length = len;

        list->resources[count++] = res;
    }

    if (count == 0) {
        free(list->resources);
        free(list);
        return NULL;
    }

    list->entry_count = count;
    //printf("Loaded %d files\n", count);

    return list;
}

/*
 * Populate the given erf structure with data from the given list of
 * files.
 */
static struct erf_resource_list *build_resource_list(char **files, u32 num_files)
{
    struct erf erf;
    struct erf_resource_list *head = NULL, *next, *prev;
    u32 last, ii;

    last = 0;

    //printf("build_resource_list called with num_files = %d\n", num_files);
    while (last < num_files) {
        /* Scan the file list to see if we should read in individual
         * files, or if we have to extract resources from an ERF file.
         */
        for (ii = 0; ii < num_files; ii++) {
            if (is_erf_file(files[ii])) {
                break;
            }
        }

        if (ii < num_files) {
            /* For now, this is an error.
             */
            fatal_error("can't handle ERF file '%s' in file list yet...", files[ii]);

            /* We found an ERF file in the list, so we have to extract
             * the resources from it.
             */
            if (last < ii - 1) {
                /* First, load the indivdual files that come before the
                 * ERF in the list.
                 */
                next = head;
                head = load_files(&files[last], ii-last);
                head->next = next;
            }

            /* Load the ERF file.
             */
            erf.name = files[ii];
            erf_load(&erf);

            /* Add the ERF's resources to our list.
             */
            next = head;
            head = (struct erf_resource_list *)
                malloc(sizeof(struct erf_resource_list));
            if (!head) {
                fatal_error("out of memory");
            }
            head->next = next;
            head->resources = erf.resources;
            head->entry_count = erf.header.entry_count;

            /* Free up memory allocated when the ERF file was loaded.
             */
            if (erf.strings) {
                free(erf.strings);
            }
            free(erf.keys);
            free(erf.data);
        } else {
            /* Just load the non-ERF files.
             */
            next = head;
            head = load_files(&files[last], ii-last);
            head->next = next;
        }

        last = ii;
    }

    /* Reverse the list, as entries were pushed on.
     */
    prev = NULL;

    while (head->next) {
        next = head->next;
        head->next = prev;

        prev = head;
        head = next;
    }

    return head;
}

/**********************************************************************/

/*
 * Fill in the given erf's header field with appropriate data.  This
 * should be done for new ERF files only.
 */
static void erf_create_header(struct erf *erf)
{
    time_t t;
    struct tm *tm;

    t = time(NULL);
    tm = localtime(&t);

    if (strncmp(erf->extension, ".erf", 4) == 0) {
        strncpy(erf->header.file_type, "ERF ", 4);
        strncpy(erf->header.version, "V1.0", 4);
        erf->header.type = ERF_TYPE;
    }
    else if (strncmp(erf->extension, ".hak", 4) == 0) {
        strncpy(erf->header.file_type, "HAK ", 4);
        strncpy(erf->header.version, "V1.0", 4);
        erf->header.type = ERF_TYPE;
    }
    else if (strncmp(erf->extension, ".mod", 4) == 0) {
        strncpy(erf->header.file_type, "MOD ", 4);
        strncpy(erf->header.version, "V1.0", 4);
        erf->header.type = MOD_TYPE;
    }
    else if (strncmp(erf->extension, ".nwm", 4) == 0) {
        strncpy(erf->header.file_type, "MOD ", 4);
        strncpy(erf->header.version, "V1.0", 4);
        erf->header.type = MOD_TYPE;
    }
    else if (strncmp(erf->extension, ".sav", 4) == 0) {
        strncpy(erf->header.file_type, "MOD ", 4);
        strncpy(erf->header.version, "V1.0", 4);
        erf->header.type = 0;
    }

    erf->header.string_count = 1;
    //erf->header.string_size = sizeof(struct erf_string) + sizeof(default_string); 
    erf->header.string_size = sizeof(struct erf_string) + strlen(description_string);
    //printf("setting string size to %u (%u + %u)\n", (unsigned int) (sizeof(struct erf_string) + strlen(description_string)),
    //   (unsigned int) sizeof(struct erf_string), (unsigned int) strlen(description_string));


    erf->header.build_year = tm->tm_year;
    erf->header.build_day = tm->tm_yday;
}

static void erf_update_resources(struct erf *erf, struct erf_resource **resources, u32 count)
{
    if (!erf->resources) {
        /* Just add the given list of resources to the empty ERF.
         */
        erf->resources = resources;
        erf->header.entry_count = erf->resource_max = count;
    } else {
        struct erf_resource *res, **found;
        char *done;
        u32 new_files, ii;

        done = (char *) calloc(1, count);
        if (!done) {
            fatal_error("out of memory");
        }

        new_files = count;

        /* Update any pre-existing ERF resources.
         */
        for (ii = 0; ii < count; ii++) {
            res = resources[ii];

            /* See if the given resource already exists in the ERF.
             */
            found = bsearch(&res, erf->resources, erf->header.entry_count,
                            sizeof(struct erf_resource *), compare_resources);
            if (found) {
                /* If we find a match, update the copy in the ERF.
                 */
                *found = res;
                printf("updating %.16s%.4s\n",
                       res->key.name, get_extension_from_type(res->key.type));
                new_files--;
                done[ii] = 1;
            } else {
                printf("adding %.16s%.4s\n",
                       res->key.name, get_extension_from_type(res->key.type));
            }
        }

        /* Add any new resources to the ERF.
         */
        if (new_files) {
            u32 jj = erf->header.entry_count;

            erf->header.entry_count += new_files;

            /* Reallocate the resource list if required.
             */
            if (erf->resource_max < erf->header.entry_count) {
                erf->resource_max = erf->header.entry_count;
                erf->resources = (struct erf_resource **)
                    realloc(erf->resources, erf->resource_max * sizeof(struct erf_resource *));
                if (!erf->resources) {
                    fatal_error("out of memory");
                }
            }

            for (ii = 0; ii < count; ii++) {
                if (!done[ii]) {
                    erf->resources[jj++] = resources[ii];
                }
            }
        }
    }

    /* Sort the resources.
     */
    qsort(erf->resources, erf->header.entry_count,
          sizeof(struct erf_resource *), compare_resources);
}

static void erf_add_resources(struct erf *erf, struct erf_resource_list *list)
{
    struct erf_string *string;
    u32 count, offset, ii;

    while (list) {
        erf_update_resources(erf, list->resources, list->entry_count);
        list = list->next;
    }

    count = erf->header.entry_count;

    /* Fill in the rest of the header.
     */
    erf_create_header(erf);

    erf->header.string_offset = sizeof(struct erf_header);
    erf->header.key_offset = erf->header.string_offset +
                             erf->header.string_size;
    erf->header.resource_offset = erf->header.key_offset +
                                  count * sizeof(struct erf_key);

    offset = erf->header.resource_offset + count * sizeof(struct erf_data);

    erf->strings = (struct erf_string *) malloc(erf->header.string_size);
    string = erf->strings;
    erf->keys = (struct erf_key *) malloc(count * sizeof(struct erf_key));
    erf->data = (struct erf_data *) malloc(count * sizeof(struct erf_data));

    /* Build the string table.
     * FIXME: Allow the string to be passed in as a command-line option.
     */
    string->language = ERF_LANGUAGE_ENGLISH;
    string->length = erf->header.string_size;
    //memcpy(string->data, default_string, sizeof(default_string)); 
    memcpy(string->data, description_string, strlen(description_string));

    /* Build the key and resource lists.
     */
    for (ii = 0; ii < count; ii++) {
        struct erf_resource *res = erf->resources[ii];

        memcpy(erf->keys[ii].name, res->key.name, 16);
        erf->keys[ii].index  = res->key.index = ii;
        erf->keys[ii].type   = res->key.type;
        erf->data[ii].offset = res->data.offset = offset;
        erf->data[ii].length = res->data.length;

        offset += res->data.length;
    }
}

/**********************************************************************/

/*
 * Create an ERF file from the given erf structure.
 */
static void erf_create(struct erf *erf)
{
    FILE *file;
    u32 count = erf->header.entry_count;
    u32 ii;

    //printf("erf_Create called with count %d\n", count);

    /* Create the output file.
     */
    file = fopen(erf->name, "wb");
    if (!file) {
        fatal_error("unable to open file '%s'", erf->name);
    }

    /* Write out all the data from the erf structure.
     */
    if (fwrite(&erf->header, sizeof(struct erf_header), 1, file) < 1) {
        fatal_error("short write on ERF header");
    }
    if (fwrite(erf->strings, erf->header.string_size, 1, file) < 1) {
        fatal_error("short write on ERF string table");
    }
    if (fwrite(erf->keys, sizeof(struct erf_key), count, file) < count) {
        fatal_error("short write on ERF key list");
    }
    if (fwrite(erf->data, sizeof(struct erf_data), count, file) < count) {
        fatal_error("short write on ERF resource list");
    }

    for (ii = 0; ii < count; ii++) {
        struct erf_resource *res = erf->resources[ii];

	//printf("writing out file %s (%d)\n",  res->key.name, ii);

        if (fwrite(res->buffer, res->data.length, 1, file) < 1) {
		if (ferror(file)) {
            fatal_error("short write on ERF data for file '%.16s%s'",
                        res->key.name, get_extension_from_type(res->key.type));
		} else {
			printf("Write returned < 1 byte for %s (should be %d)\n", res->key.name, res->data.length);
		}
        }
    }

    fclose(file);
}

/*
 * Extract all the data from the given erf structure.
 */
static void erf_extract(struct erf *erf)
{
    FILE *file;
    u32 ii;

    if (showdesc && erf->strings != NULL && erf->strings->data != NULL) {
	    printf("\"%s\"\n", erf->strings->data);
    }
    
    for (ii = 0; ii < erf->header.entry_count; ii++) {
        struct erf_resource *res = erf->resources[ii];
        char name[21] = { 0, };

        sprintf(name, "%.16s%.4s",
                res->key.name,
                get_extension_from_type(res->key.type));

        file = fopen(name, "wb");
        if (!file) {
            fatal_error("unable to open file '%s'\n", name);
        }

        if (verbose) {
            printf("%s\n", name);
        }
        if (fwrite(res->buffer, res->data.length, 1, file) < 1) {
            fatal_error("unable to write file '%s'\n", name);
        }

        fclose(file);
    }
}

/*
 * List the contents of the given erf structure.
 */
static void erf_list(struct erf *erf)
{
    u32 ii;

    if (verbose) {
	
	if (erf->strings != NULL && erf->strings->data != NULL) {
		printf("Desc = \"%s\"\n", erf->strings->data);
	}
        for (ii = 0; ii < erf->header.entry_count; ii++) {
            struct erf_resource *res = erf->resources[ii];

            printf("%03d: %.16s%.4s %8d\n",
                   res->key.index, res->key.name,
                   get_extension_from_type(res->key.type),
                   res->data.length);
        }
    } else {
	    if (showdesc && erf->strings != NULL && erf->strings->data != NULL) {
		    printf("\"%s\"\n", erf->strings->data);
	    }
	    for (ii = 0; ii < erf->header.entry_count; ii++) {
		    struct erf_resource *res = erf->resources[ii];
		    
		    printf("%.16s%.4s\n",
			   res->key.name,
			   get_extension_from_type(res->key.type));
	    }
    }
}

/**********************************************************************/

static void usage(int exitval)
{
    fputs("Neverwinter Nights Encapsulated Resource File (ERF) Utility (version ERF_VERSION) \n"
          ERF_COPYRIGHT "\n"
          "\n"
          "erf can write NWN .erf, .hak, .mod, .nwm and .sav files.  The type is \n"
          "determined from the extension of the archive name.  Exactly one of the main\n"
          "operation mode options below must be specified.\n"
          "\n"
          "Usage: erf [OPTION]... [FILE]...\n"
          "\n"
          "Examples:\n"
          "  erf -c archive.hak foo bar    # Create archive.hak from files foo and bar.\n"
          "  erf -tv archive.hak           # List all files in archive.hak verbosely.\n"
          "  erf -u archive.hak foo        # Update file foo in archive.hak.\n"
          "  erf -x archive.hak            # Extract all files from archive.hak.\n"
	  "  erf -cd $'Lots of lines \\n \\n here \\n'  archive.hak foo bar  # create with description containing newlines.\n"

          "\n"
          "Main operation mode:\n"
          "  -c, --create                  create a new archive\n"
          "  -t, --list                    list the contents of an archive\n"
          "  -u, --update                  update an archive (like import in Toolset)\n"
          "  -x, --extract                 extract files from an archive\n"
          "\n"
          "Informative output:\n"
          "  -h, --help                    print this help, then exit\n"
          "  -v, --verbose                 verbosely list files processed (includes -D)\n"
	  "  -D,                           print the description to stdout\n"
          "      --version                 print erf program version number, then exit\n"
          "\n"
	  "Input options:\n"
	  "  -d, --desc                    Use given string as description instead of default\n"
	  "\n"
          "Report bugs to " ERF_AUTHOR ".\n", stdout);
    exit(exitval);
}

static void version(void)
{
    fputs("erf (NWN ERF Utility) " ERF_VERSION "\n" ERF_COPYRIGHT "\n", stdout);
    exit(0);
}

static void set_command(command_t *command, command_t new_command)
{
    if (*command != COMMAND_DEFAULT) {
        fputs("erf: You may not specify more than one '-ctux' option.\n"
              "Try 'erf --help' for more information.\n", stdout);
        exit(0);
    }
    *command = new_command;
}

static int set_description(char * desc) {
	if (desc != NULL) {
		int len = strlen(desc);
		if (len <= 0) {
			return -1;
		}
		description_string = malloc(len +1);
		strncpy(description_string, desc, len);
		//description_string[len] = '\n';
		//description_string[len + 1] = 0;
		description_string[len] = 0;
		printf("Got description \"%s\"\n", description_string);
		return 0;
	}
	return -1;
}


int main(int argc, char *argv[])
{
    struct erf erf = { 0, };
    struct erf_resource_list *list;
    command_t command = COMMAND_DEFAULT;
    char **files = NULL;
    int num_files = 0, ii;
    int have_desc = 0;

    for (ii = 1; ii < argc; ii++) {
        if (argv[ii][0] == '-' && argv[ii][1] != '-') {
            if (strchr(argv[ii], 'v')) {
                verbose = 1;
            }
            if (strchr(argv[ii], 'h')) {
                usage(0);
            }
            if (strchr(argv[ii], 'c')) {
                set_command(&command, COMMAND_CREATE);
            }
            if (strchr(argv[ii], 'x')) {
                set_command(&command, COMMAND_EXTRACT);
            }
            if (strchr(argv[ii], 't')) {
                set_command(&command, COMMAND_LIST);
            }
            if (strchr(argv[ii], 'u')) {
                set_command(&command, COMMAND_UPDATE);
            } 
	    if (strchr(argv[ii], 'D')) {
		    showdesc = 1;
            }
	    if (strchr(argv[ii], 'd')) {
		    if (set_description(argv[ii +1])) {
			    usage(1);
		    } 
		    have_desc = 1;
		    ii++;
            }
	  /* 	 else {
		    fprintf(stderr, "Unknown option \"-%s\"\n", argv[ii]);
		    fprintf(stderr, "Try 'erf --help' for more information.\n");
		    exit(1);
		    //usage(1);
	    }
	    */
        } else if (strcmp(argv[ii], "--create") == 0) {
		set_command(&command, COMMAND_CREATE);
        } else if (strcmp(argv[ii], "--extract") == 0) {
		set_command(&command, COMMAND_EXTRACT);
        } else if (strcmp(argv[ii], "--help") == 0) {
		usage(0);
        } else if (strcmp(argv[ii], "--list") == 0) {
		set_command(&command, COMMAND_LIST);
        } else if (strcmp(argv[ii], "--update") == 0) {
		set_command(&command, COMMAND_UPDATE);
        } else if (strcmp(argv[ii], "--desc") == 0) {
		if (set_description(argv[ii +1])) {
			usage(1);
		} 
		have_desc = 1;
		ii++;
		
        } else if (strcmp(argv[ii], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[ii], "--version") == 0) {
            version();
        } else if (erf.name == NULL) {
            erf.name = argv[ii];
        } else {
		// Check for unexpanded wildcards. This is an error
		if (strchr(argv[ii], '*') || strchr(argv[ii], '?')) {
			files =  &argv[ii];
			num_files = 0;
		} else {
			files = &argv[ii];
			num_files = argc - ii;
		}
		break;
        }
    }

    if (erf.name) {
        if (strlen(erf.name) <= 4) {
            fatal_error("bad ERF name '%s'", erf.name);
        }
        if (!is_erf_file(erf.name)) {
            fatal_error("file '%s' is not an ERF", erf.name);
        }
        get_extension(erf.extension, erf.name);
    }

    if ((command == COMMAND_CREATE || command == COMMAND_UPDATE) &&
        (num_files == 0)) {
        fatal_error("no input files");
    }
    
    if (command != COMMAND_CREATE && command != COMMAND_UPDATE && command != COMMAND_DEFAULT &&  have_desc) {
	    printf("Adding description is only meaningful with create (-c) or update (-u). Ignored\n");
    } 
    if ((command == COMMAND_CREATE || command == COMMAND_UPDATE) && showdesc) {
	    printf("Printing description is only meaningful with list (-t) or extract (-x). Ignored\n");
    }
    //printf("erf called %s num files %d\n", erf.name, num_files);
    


    switch (command) {
    case COMMAND_DEFAULT:
        fputs("erf: You must specify one of the '-ctux' options.\n"
              "Try 'erf --help' for more information.\n", stdout);
        break;
    case COMMAND_CREATE:
        list = build_resource_list(files, num_files);
        erf_add_resources(&erf, list);
        erf_create(&erf);
        break;
    case COMMAND_EXTRACT:
        erf_load(&erf);
        erf_extract(&erf);
        break;
    case COMMAND_LIST:
        erf_load(&erf);
        erf_list(&erf);
        break;
    case COMMAND_UPDATE:
        erf_load(&erf);
        list = build_resource_list(files, num_files);
        erf_add_resources(&erf, list);
        erf_create(&erf);
        break;
    }

    return 0;
}
