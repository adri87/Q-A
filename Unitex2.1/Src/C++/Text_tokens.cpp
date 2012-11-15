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

#include "Text_tokens.h"
#include "Error.h"
#include "Token.h"


struct text_tokens* new_text_tokens(Abstract_allocator prv_alloc) {
struct text_tokens* tmp;
tmp=(struct text_tokens*)malloc_cb(sizeof(struct text_tokens),prv_alloc);
if (tmp==NULL) {
   fatal_alloc_error("new_text_tokens");
}
tmp->N=0;
tmp->SENTENCE_MARKER=-1;
tmp->token=NULL;
return tmp;
}


struct text_tokens* load_text_tokens(const char* nom,int mask_encoding_compatibility_input,Abstract_allocator prv_alloc) {
U_FILE* f;
f=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,nom,U_READ);
if (f==NULL) {
   return NULL;
}
struct text_tokens* res=new_text_tokens(prv_alloc);
u_fscanf(f,"%d\n",&(res->N));
res->token=(unichar**)malloc_cb((res->N)*sizeof(unichar*),prv_alloc);
if (res->token==NULL) {
   fatal_alloc_error("load_text_tokens");
}
unichar tmp[MAX_TAG_LENGTH];
res->SENTENCE_MARKER=-1;
res->SPACE=-1;
res->STOP_MARKER=-1;
int i=0;
int size_gets;
while (EOF!=(size_gets=u_fgets(tmp,MAX_TAG_LENGTH,f))) {
  if (size_gets>0) {
    if (tmp[size_gets-1]=='\n') {
        tmp[size_gets-1]=0;
    }
  }
  res->token[i]=u_strdup(tmp,prv_alloc);
  if (!u_strcmp(tmp,"{S}")) {
     res->SENTENCE_MARKER=i;
  } else if (!u_strcmp(tmp," ")) {
            (res->SPACE)=i;
         }
    else if (!u_strcmp(tmp,"{STOP}")) {
            (res->STOP_MARKER)=i;
         }
  i++;
  if (i>res->N) {
     fatal_error("Inconsistency in file %s between header (%d) and actual number of lines\n"
    		     "Last token loaded=%S\n",nom,res->N,tmp);
  }
}
u_fclose(f);
return res;
}


struct string_hash* load_text_tokens_hash(const char* nom,int mask_encoding_compatibility_input,
                                          int *SENTENCE_MARKER,
                                          int* STOP_MARKER,
                                          int *NUMBER_OF_TEXT_TOKENS,Abstract_allocator /* prv_alloc */) {
U_FILE* f;
f=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,nom,U_READ);
if (f==NULL) {
   return NULL;
}
(*SENTENCE_MARKER)=-1;
u_fscanf(f,"%d\n",NUMBER_OF_TEXT_TOKENS);
*NUMBER_OF_TEXT_TOKENS=(*NUMBER_OF_TEXT_TOKENS);
struct string_hash* res;
res=new_string_hash(*NUMBER_OF_TEXT_TOKENS);
unichar tmp[4096];

int x,i=0;
int size_gets;
while (EOF!=(size_gets=u_fgets(tmp,MAX_TAG_LENGTH,f))) {
  if (size_gets>0) {
    if (tmp[size_gets-1]=='\n') {
        tmp[size_gets-1]=0;
    }
  }
  x=get_value_index(tmp,res);
  if (!u_strcmp(tmp,"{S}")) {
     *SENTENCE_MARKER=x;
  } else if (!u_strcmp(tmp,"{STOP}")) {
     *STOP_MARKER=i;
  }
  i++;
  if (i>*NUMBER_OF_TEXT_TOKENS) {
     fatal_error("Inconsistency in file %s between header (%d) and actual number of lines\n"
    		     "Last token loaded=%S\n",nom,*NUMBER_OF_TEXT_TOKENS,tmp);
  }
}
u_fclose(f);
return res;
}




void free_text_tokens(struct text_tokens* tok,Abstract_allocator prv_alloc) {
for (int i=0;i<tok->N;i++) {
   free_cb(tok->token[i],prv_alloc);
}
free_cb(tok->token,prv_alloc);
free_cb(tok,prv_alloc);
}




void explorer_token_tree(int pos,const unichar* sequence,const Alphabet* alph,struct string_hash_tree_node* n,struct list_int** l,Abstract_allocator prv_alloc) {
if (sequence[pos]=='\0') {
   // if we are at the end of the sequence
   if (n->value_index!=-1) {
      // if the sequence is a text token, we add its number to the list
      (*l)=sorted_insert(n->value_index,*l,prv_alloc);
   }
   return;
}
struct string_hash_tree_transition* trans=n->trans;
while (trans!=NULL) {
  if (is_equal_or_uppercase(sequence[pos],trans->letter,alph)) {
     // if we can follow the transition
     explorer_token_tree(pos+1,sequence,alph,trans->node,l,prv_alloc);
  }
  trans=trans->next;
}
}



struct list_int* get_token_list_for_sequence(const unichar* sequence,const Alphabet* alph,
                                                  struct string_hash* hash,Abstract_allocator prv_alloc) {
struct list_int* l=NULL;
explorer_token_tree(0,sequence,alph,hash->root,&l,prv_alloc);
return l;
}



int get_token_number(const unichar* s,struct text_tokens* tok) {
for (int i=0;i<tok->N;i++) {
    if (!u_strcmp(tok->token[i],s)) return i;
}
return -1;
}

unichar  *get_text_token(int token_number ,struct text_tokens* tok) {
           return(tok->token[token_number]);
}


//
// return 1 if s is a digit sequence, 0 else
//
int is_a_digit_token(const unichar* s) {
int i=0;
while (s[i]!='\0') {
   if (s[i]<'0' || s[i]>'9') {
      return 0;
   }
   i++;
}
return 1;
}


//
// Scans text tokens to extract semantic codes contained in tags like {le,.DET:ms}
//
void extract_semantic_codes_from_tokens(const struct string_hash* tok,
                                        struct string_hash* semantic_codes,Abstract_allocator prv_alloc) {
for (int i=0;i<tok->size;i++) {
    if (tok->value[i][0]=='{' && u_strcmp(tok->value[i],"{S}")
                            && u_strcmp(tok->value[i],"{STOP}")) {
       struct dela_entry* temp=tokenize_tag_token(tok->value[i],prv_alloc);
       for (int j=0;j<temp->n_semantic_codes;j++) {
          get_value_index(temp->semantic_codes[j],semantic_codes);
       }
       free_dela_entry(temp,prv_alloc);
    }
}
}



/**
 * Scans text tokens to extract semantic codes contained in .inf
 * of morphological mode dictionaries.
 */
void extract_semantic_codes_from_morpho_dics(const struct INF_codes** array,int N,
                                        struct string_hash* semantic_codes,Abstract_allocator prv_alloc) {
unichar line[2048];
/* Foo line for generating a dela_entry */
u_strcpy(line,"a,a");
for (int i=0;i<N;i++) {
   if (array[i]==NULL) {
      continue;
   }
   for (int j=0;j<array[i]->N;j++) {
      struct list_ustring* codes=array[i]->codes[j];
      while (codes!=NULL) {
         u_strcpy(line+3,codes->string);
         //u_printf("code=_%S_\n",codes->string);
         struct dela_entry* entry=tokenize_DELAF_line(line,prv_alloc);
         if (entry!=NULL) {
            /* There could be an error due to an old dictionary with,
             * for instance, duplicate semantic codes */
            for (int k=0;k<entry->n_semantic_codes;k++) {
               get_value_index(entry->semantic_codes[k],semantic_codes);
            }
            free_dela_entry(entry,prv_alloc);
         }
         codes=codes->next;
      }
   }
}
}

//---------------------------------------------------------------------------

