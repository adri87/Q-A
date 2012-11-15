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

#include "ParsingInfo.h"
#include "Error.h"

size_t get_prefered_allocator_item_size_for_nb_variable(int nbvar)
{
    return AroundAllocAlign(sizeof(struct parsing_info)) + 
           AroundAllocAlign(get_expected_variable_backup_size_in_byte_for_nb_variable(nbvar)) +
           AroundAllocAlign(sizeof(unichar)*(SIZE_RESERVE_NB_UNICHAR_STACK_INSAMEALLOC+1));
}

size_t get_prefered_allocator_item_size_for_variable(Variables* v)
{
    return AroundAllocAlign(sizeof(struct parsing_info)) + 
           AroundAllocAlign(get_variable_backup_size_in_byte(v)) +
           AroundAllocAlign(sizeof(unichar)*(SIZE_RESERVE_NB_UNICHAR_STACK_INSAMEALLOC+1));
}

void update_parsing_info_stack(struct parsing_info*list,const unichar* new_stack_value)
{
   if (list->stack_must_be_free) {
       free(list->stack);
   }
   list->stack_must_be_free=0;
   list->stack = NULL;
   if (new_stack_value == NULL) {
       return;
   }
   list->stack = list->stack_internal_reserved_space;

   int stack_internal_reserved_space_size = list->stack_internal_reserved_space_size;
   unichar* stack_internal_reserved_space=list->stack_internal_reserved_space;
   int pos=0;
   for (;;)
   {
       unichar c=*(new_stack_value+pos);
       *((stack_internal_reserved_space)+pos)=c;
       if (c=='\0')
           return ;

       pos++;
       if (pos==stack_internal_reserved_space_size)
       {
           list->stack = u_strdup(new_stack_value);
           list->stack_must_be_free=1;
           return;
       }
   }
}

/**
 * Allocates, initializes and returns a new parsing info structure.
 */
struct parsing_info* new_parsing_info(int pos,int pos_in_token,int state,int stack_pointer,unichar* stack,
                                      Variables* v,OutputVariables* output_var,struct dela_entry* dic_entry,
                                      struct dic_variable* v2,
                                      int left_ctx_shift,int left_ctx_base,unichar* jamo,int pos_int_jamo,
                                      Abstract_allocator prv_alloc_recycle) {
struct parsing_info* info;
unsigned char*buf;
buf=(unsigned char*)malloc_cb(get_prefered_allocator_item_size_for_variable(v),prv_alloc_recycle);
if (buf==NULL) {
   fatal_alloc_error("new_parsing_info");
}
info=(struct parsing_info*)buf;
info->input_variable_backup = (int*)(buf + AroundAllocAlign(sizeof(struct parsing_info)));
info->stack_internal_reserved_space = (unichar*)(buf +
                                          AroundAllocAlign(sizeof(struct parsing_info)) +
                                          AroundAllocAlign(get_variable_backup_size_in_byte(v)));
info->stack_internal_reserved_space_size = SIZE_RESERVE_NB_UNICHAR_STACK_INSAMEALLOC;
/*
info=(struct parsing_info*)malloc_cb(sizeof(struct parsing_info),prv_alloc_recycle);
if (info==NULL) {
   fatal_alloc_error("new_parsing_info");
}*/

info->input_variable_backup_must_be_free=0;
info->stack_must_be_free=0;


info->position=pos;
info->pos_in_token=pos_in_token;
info->state_number=state;
info->next=NULL;
info->stack_pointer=stack_pointer;
update_parsing_info_stack(info,stack);
//info->input_variable_backup=create_variable_backup(v,prv_alloc_recycle);
//info->input_variable_backup=create_variable_backup(v,prv_alloc_recycle);
init_variable_backup(info->input_variable_backup,v);
info->output_variable_backup=create_output_variable_backup(output_var);
info->variable_backup_size=0;
if (v!=NULL)
  if (v->variable_index!=NULL)
      info->variable_backup_size=v->variable_index->size;
info->dic_entry=clone_dela_entry(dic_entry);
info->dic_variable_backup=clone_dic_variable_list(v2);
info->left_ctx_shift=left_ctx_shift;
info->left_ctx_base=left_ctx_base;
info->jamo=jamo;
info->pos_in_jamo=pos_int_jamo;
return info;
}


/**
 * Frees the whole memory associated to the given information list.
 */
void free_parsing_info(struct parsing_info* list,Abstract_allocator prv_alloc_recycle) {
struct parsing_info* tmp;
while (list!=NULL) {
   tmp=list->next;
   if (list->input_variable_backup_must_be_free) {
       free_variable_backup(list->input_variable_backup,prv_alloc_recycle);
   }
   if (list->stack_must_be_free) {
       free(list->stack);
   }
   free_output_variable_backup(list->output_variable_backup);
   clear_dic_variable_list(&(list->dic_variable_backup));
   free_dela_entry(list->dic_entry);
   /* No free on list->jamo because it was only a pointer on the global jamo tag array */
   free_cb(list,prv_alloc_recycle);
   list=tmp;
}
}


/**
 * Inserts an element in the given information list only if there is no element
 * with same end position of match.
 */
struct parsing_info* insert_if_absent(int pos,int pos_in_token,int state,struct parsing_info* list,int stack_pointer,
                                      unichar* stack,Variables* v,OutputVariables* output_var,
                                      struct dic_variable* v2,
                                      int left_ctx_shift,int left_ctx_base,unichar* jamo,int pos_in_jamo,
                                      Abstract_allocator prv_alloc_recycle) {
if (list==NULL) return new_parsing_info(pos,pos_in_token,state,stack_pointer,stack,v,output_var,NULL,v2,
                                        left_ctx_shift,left_ctx_base,jamo,pos_in_jamo,prv_alloc_recycle);
if (list->position==pos && list->pos_in_token==pos_in_token && list->state_number==state
	&& list->jamo==jamo /* We can because we only work on pointers on unique elements */
	&& list->pos_in_jamo==pos_in_jamo) {
   list->stack_pointer=stack_pointer;
   /* We update the stack value */
   update_parsing_info_stack(list,stack);

   int v_variable_index_size=0;
   if (v!=NULL)
     if (v->variable_index!=NULL)
         v_variable_index_size=v->variable_index->size;

   if (list->variable_backup_size == v_variable_index_size) {
      update_variable_backup(list->input_variable_backup,v);
   }
   else {
      free_variable_backup(list->input_variable_backup,prv_alloc_recycle);
      list->input_variable_backup=create_variable_backup(v,prv_alloc_recycle);
      list->variable_backup_size=v_variable_index_size;
   }
   free_output_variable_backup(list->output_variable_backup);
   list->output_variable_backup=create_output_variable_backup(output_var);
   clear_dic_variable_list(&list->dic_variable_backup);
   list->dic_variable_backup=clone_dic_variable_list(v2);
   if (list->dic_entry!=NULL) {
      fatal_error("Unexpected non NULL dic_entry in insert_if_absent\n");
   }
   list->left_ctx_shift=left_ctx_shift;
   list->left_ctx_base=left_ctx_base;
   return list;
}
list->next=insert_if_absent(pos,pos_in_token,state,list->next,stack_pointer,stack,v,output_var,v2,
                            left_ctx_shift,left_ctx_base,jamo,pos_in_jamo,prv_alloc_recycle);
return list;
}


/**
 * Inserts an element in the given information list only if there is no element
 * with position and same stack. */
struct parsing_info* insert_if_different(int pos,int pos_in_token,int state,struct parsing_info* list,int stack_pointer,
                                         unichar* stack,Variables* v,OutputVariables* output_var,
                                         struct dic_variable* v2,
                                         int left_ctx_shift,int left_ctx_base,
                                         unichar* jamo,int pos_in_jamo,Abstract_allocator prv_alloc_recycle) {
if (list==NULL) return new_parsing_info(pos,pos_in_token,state,stack_pointer,stack,v,output_var,NULL,v2,
                                        left_ctx_shift,left_ctx_base,jamo,pos_in_jamo,prv_alloc_recycle);
if ((list->position==pos) /* If the length is the same... */
    && (list->pos_in_token==pos_in_token)
    && (list->state_number==state)
    && !(u_strcmp(list->stack,stack)) /* ...and if the stack content too */
    && list->left_ctx_shift==left_ctx_shift
    && list->left_ctx_base==left_ctx_base
    && list->jamo==jamo /* See comment in insert_if_absent*/
    && list->pos_in_jamo==pos_in_jamo) {
    /* then we overwrite the current list element */
   list->stack_pointer=stack_pointer;

   int v_variable_index_size=0;
   if (v!=NULL)
     if (v->variable_index!=NULL)
         v_variable_index_size=v->variable_index->size;

   if (list->variable_backup_size == v_variable_index_size) {
      update_variable_backup(list->input_variable_backup,v);
   }
   else {
      free_variable_backup(list->input_variable_backup,prv_alloc_recycle);
      list->input_variable_backup=create_variable_backup(v,prv_alloc_recycle);
      list->variable_backup_size=v_variable_index_size;
   }
   free_output_variable_backup(list->output_variable_backup);
   list->output_variable_backup=create_output_variable_backup(output_var);
   clear_dic_variable_list(&list->dic_variable_backup);
   list->dic_variable_backup=clone_dic_variable_list(v2);
   if (list->dic_entry!=NULL) {
      fatal_error("Unexpected non NULL dic_entry in insert_if_different\n");
   }
   return list;
}
/* Otherwise, we look in the rest of the list */
list->next=insert_if_different(pos,pos_in_token,state,list->next,stack_pointer,stack,v,output_var,v2,
                               left_ctx_shift,left_ctx_base,jamo,pos_in_jamo,prv_alloc_recycle);
return list;
}


/**
 * This function behaves in the same way than 'insert_if_absent', except that
 * we take a DELAF entry into account, and that we don't have to take care of
 * the stack and variables.
 */
struct parsing_info* insert_morphological_match(int pos,int pos_in_token,int state,struct parsing_info* list,
                                                struct dela_entry* dic_entry,unichar* jamo,int pos_in_jamo,
                                                Abstract_allocator prv_alloc_recycle) {
if (list==NULL) return new_parsing_info(pos,pos_in_token,state,-1,NULL,NULL,NULL,dic_entry,NULL,-1,-1,jamo,pos_in_jamo,prv_alloc_recycle);
if (list->position==pos && list->pos_in_token==pos_in_token && list->state_number==state
    && list->dic_entry==dic_entry
    && list->jamo==jamo /* See comment in insert_if_absent*/
    && list->pos_in_jamo==pos_in_jamo) {
    /* If the morphological match is already in the list, we do nothing.
     * Note that this may occur when we don't take DELAF entries into account
     * (i.e. dic_entry==NULL) */
   return list;
}
list->next=insert_morphological_match(pos,pos_in_token,state,list->next,dic_entry,jamo,pos_in_jamo,prv_alloc_recycle);
return list;
}

