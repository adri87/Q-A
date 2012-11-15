/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 *
 */

#ifndef DELAH
#define DELAH
#include <stdio.h>
#include <string.h>
#include "String_hash.h"
#include "List_ustring.h"
#include "Alphabet.h"
#include "AbstractAllocator.h"

/* Maximum size of a DELA line */
#define DIC_LINE_SIZE 4096

/* Maximum size of a word (word form or lemma) */
#define DIC_WORD_SIZE (DIC_LINE_SIZE/2)

/* Maximum number of semantic codes (Hum,Conc,z1,...) per line */
#define MAX_SEMANTIC_CODES 100

/* Maximum number of flexional codes (ms,Kf,W,...) per line
 *
 * For languages with rich inflection this number must be considerably high,
 * because words with (almost) no inflection and many homonymic forms may occur.
 * e.g. Russian adjectives have max. 31 forms
 *      (long forms: 6*case * 4*gender/number + 2*animacy in acc.
 *       + 4 short forms + 1 comparative = 31)
 *      German adjectives have 147 forms
 *      ((4*case * 4*gender/number * 3 declension + 1 predicative) * 3 comparation)
 *      but for indeclinable adjectives (like "lila") there is no comparation,
 *      so 49 forms is the maximum.
 */
#define MAX_INFLECTIONAL_CODES 100

/* Maximum number of filters 100 */

#define MAX_FILTERS 100

/* Value returned when a word is not found in a dictionary */
#define NOT_IN_DICTIONARY -1

/*
 * This structure is used to represent an entry of a DELA dictionary.
 * Special characters are supposed to have been unprotected, and comments
 * are ignored. For instance, if we have the following line:
 *
 * M\. Phileas Fogg,Phileas Fogg.N+PR:ms// proper name with "M."
 *
 * the associated entry will contain:
 *
 * inflected =           "M. Phileas Fogg"
 * lemma =               "Phileas Fogg"
 * n_semantic_codes:     2
 * semantic_codes[0] = "N"   semantic_codes[1] = "PR"
 * n_inflectional_codes: 1
 * inflectional_codes[0] = "ms"
 */
struct dela_entry {
	unichar* inflected;
	unichar* lemma;
	/* Number of grammatical and semantic codes.
	 * By convention, the first is supposed to be the
	 * grammatical category of the entry. */
	unsigned char n_semantic_codes;
	unsigned char n_inflectional_codes;
	unsigned char n_filters;
	unichar* semantic_codes[MAX_SEMANTIC_CODES];
	unichar* inflectional_codes[MAX_INFLECTIONAL_CODES];
	unichar* filters[MAX_FILTERS];
};


/**
 * This structure is used to store all the INF codes of an .inf file.
 */
struct INF_codes {
	/* Array containing for each line of the .inf file the reversed list of its
	 * components. For instance, if the first line contains:
	 *
	 * .N+NA+z1:fs,.N+Loc:fs
	 *
	 * codes[0] will contain the following list:
	 *
	 *  ".N+Loc:fs"   -->   ".N+NA+z1:fs"   -->   NULL
	 *
	 */
	struct list_ustring** codes;
	/* Number of lines in the .inf file */
	int N;
};


struct dela_entry* new_dela_entry(const unichar*,const unichar*,const unichar*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct dela_entry* clone_dela_entry(const struct dela_entry*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
int equal(const struct dela_entry*,const struct dela_entry*);
struct dela_entry* tokenize_DELAF_line(const unichar*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct dela_entry* tokenize_DELAF_line_opt(const unichar*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct dela_entry* tokenize_DELAF_line(const unichar*,int,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct dela_entry* tokenize_DELAF_line(const unichar*,int,int,int*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct dela_entry* tokenize_tag_token(const unichar*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct dela_entry* tokenize_DELAS_line(const unichar*,int*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
struct dela_entry* is_strict_DELAS_line(const unichar*,Alphabet*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void get_compressed_line(struct dela_entry*,unichar*,int);
struct list_ustring* tokenize_compressed_info(const unichar*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void uncompress_entry(const unichar*,unichar*,unichar*);
struct INF_codes* load_INF_file(const char*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_INF_codes(struct INF_codes*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
unsigned char* load_BIN_file(const char*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void free_BIN_file(unsigned char*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
void rebuild_dictionary(const unsigned char*,const struct INF_codes*,U_FILE*);
void extract_semantic_codes(const char*,struct string_hash*);
void tokenize_DELA_line_into_3_parts(const unichar*,unichar*,unichar*,unichar*);
void check_DELA_line(const unichar*,U_FILE*,int,int,char*,struct string_hash*,struct string_hash*,
                     struct string_hash*,struct string_hash*,int*,int*,Alphabet*,int,Abstract_allocator prv_alloc=NULL);
int warning_on_code(const unichar*,unichar*,int);
int contains_unprotected_equal_sign(const unichar*);
void replace_unprotected_equal_sign(unichar*,unichar);
void unprotect_equal_signs(unichar*);
void free_dela_entry(struct dela_entry*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
int check_tag_token(const unichar*);
int dic_entry_contain_gram_code(const struct dela_entry*,const unichar*);
int dic_entry_contain_inflectional_code(const struct dela_entry*,const unichar*);
void get_inflection_code(unichar*,char*,unichar*,int*);
void build_tag(struct dela_entry*,const unichar*,unichar*);
int same_semantic_codes(const struct dela_entry*,const struct dela_entry*);
int same_inflectional_codes(const struct dela_entry*,const struct dela_entry*);
int same_codes(const struct dela_entry*,const struct dela_entry*);
void merge_inflectional_codes(struct dela_entry*,const struct dela_entry*,Abstract_allocator prv_alloc=STANDARD_ALLOCATOR);
int one_inflectional_codes_contains_the_other(const unichar*,const unichar*);
int get_inf_code_exact_match(unsigned char* bin,unichar* str);

void debug_print_entry(struct dela_entry*);
void debug_println_entry(struct dela_entry*);

#endif

