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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "Alphabet.h"
#include "String_hash.h"
#include "File.h"
#include "Copyright.h"
#include "DELA.h"
#include "Error.h"
#include "Vector.h"
#include "HashTable.h"
#include "UnitexGetOpt.h"
#include "Tokenize.h"
#include "Token.h"


#define NORMAL 0
#define CHAR_BY_CHAR 1




void sort_and_save_by_frequence(U_FILE*,vector_ptr*,vector_int*);
void sort_and_save_by_alph_order(U_FILE*,vector_ptr*,vector_int*);
void compute_statistics(U_FILE*,vector_ptr*,Alphabet*,int,int,int,int);
void normal_tokenization(U_FILE*,U_FILE*,U_FILE*,Alphabet*,vector_ptr*,struct hash_table*,vector_int*,vector_int*,
		   int*,int*,int*,int*);
void char_by_char_tokenization(U_FILE*,U_FILE*,U_FILE*,Alphabet*,vector_ptr*,struct hash_table*,vector_int*,vector_int*,
		   int*,int*,int*,int*);
void save_new_line_positions(U_FILE*,vector_int*);
void load_token_file(char* filename,int mask_encoding_compatibility_input,vector_ptr* tokens,struct hash_table* hashtable,vector_int* n_occur);

void write_number_of_tokens(const char* name,Encoding encoding_output,int bom_output,int n) {
  U_FILE* f;
  char number[11];

  f=u_fopen_creating_versatile_encoding(encoding_output,bom_output,name,U_MODIFY);
  
  number[10]=0;
  int offset=9;
  for (;;) {
      number[offset]=(char)((n%10)+'0');
      n/=10;
      if (offset==0)
          break;
      offset--;
  }
u_fprintf(f,number);
u_fclose(f);
}




const char* usage_Tokenize =
         "Usage: Tokenize [OPTIONS] <txt>\n"
         "\n"
         "  <txt>: any unicode text file\n"
         "\n"
         "OPTIONS:\n"
         "  -a ALPH/--alphabet=ALPH: the alphabet file\n"
         "  -c/--char_by_char: with this option, the program does a char by char tokenization\n"
         "                     (except for the tags {S}, {STOP} or like {today,.ADV}). This\n"
         "                     mode may be used for languages like Thai.\n"
         "  -w/--word_by_word: word by word tokenization (default);\n"
         "  -t TOKENS/--tokens=TOKENS: specifies a tokens.txt file to load and modify, instead of\n"
         "                             creating a new one from scratch;\n"
         "  -h/--help: this help\n"
         "\n"
         "Tokenizes the text. The token list is stored into \"tokens.txt\" and\n"
         "the coded text is stored into \"text.cod\".\n"
         "The program also produces 4 files named \"tok_by_freq.txt\", \"tok_by_alph.txt\",\n"
         "\"stats.n\" and \"enter.pos\". They contain the token list sorted by frequence and by\n"
         "alphabetical order and \"stats.n\" contains some statistics. The file \"enter.pos\"\n"
         "contains the position in tokens of all the carriage return sequences. All\n"
         "files are saved in the XXX_snt directory where XXX is <txt> without its extension.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Tokenize);
}


const char* optstring_Tokenize=":a:cwt:hk:q:";
const struct option_TS lopts_Tokenize[]={
   {"alphabet", required_argument_TS, NULL, 'a'},
   {"char_by_char", no_argument_TS, NULL, 'c'},
   {"word_by_word", no_argument_TS, NULL, 'w'},
   {"tokens", required_argument_TS, NULL, 't'},
   {"input_encoding",required_argument_TS,NULL,'k'},
   {"output_encoding",required_argument_TS,NULL,'q'},
   {"help", no_argument_TS, NULL, 'h'},
   {NULL, no_argument_TS, NULL, 0}
};


int main_Tokenize(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

char alphabet[FILENAME_MAX]="";
char token_file[FILENAME_MAX]="";

Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
int val,index=-1;
int mode=NORMAL;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Tokenize,lopts_Tokenize,&index,vars))) {
   switch(val) {
   case 'a': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet file name\n");
             }
             strcpy(alphabet,vars->optarg);
             break;
   case 'c': mode=CHAR_BY_CHAR; break;
   case 'w': mode=NORMAL; break;
   case 't': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty token file name\n");
             }
             strcpy(token_file,vars->optarg);
             break;
   case 'k': if (vars->optarg[0]=='\0') {
                fatal_error("Empty input_encoding argument\n");
             }
             decode_reading_encoding_parameter(&mask_encoding_compatibility_input,vars->optarg);
             break;
   case 'q': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_encoding argument\n");
             }
             decode_writing_encoding_parameter(&encoding_output,&bom_output,vars->optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_Tokenize[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
U_FILE* text;
U_FILE* out;
U_FILE* output;
U_FILE* enter;
char tokens_txt[FILENAME_MAX];
char text_cod[FILENAME_MAX];
char enter_pos[FILENAME_MAX];
Alphabet* alph=NULL;

get_snt_path(argv[vars->optind],text_cod);
strcat(text_cod,"text.cod");
get_snt_path(argv[vars->optind],tokens_txt);
strcat(tokens_txt,"tokens.txt");
get_snt_path(argv[vars->optind],enter_pos);
strcat(enter_pos,"enter.pos");
text=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,argv[vars->optind],U_READ);
if (text==NULL) {
   fatal_error("Cannot open text file %s\n",argv[vars->optind]);
}
if (alphabet[0]!='\0') {
   alph=load_alphabet(alphabet);
   if (alph==NULL) {
      error("Cannot load alphabet file %s\n",alphabet);
      u_fclose(text);
      return 1;
   }
}
out=u_fopen(BINARY,text_cod,U_WRITE);
if (out==NULL) {
   error("Cannot create file %s\n",text_cod);
   u_fclose(text);
   if (alph!=NULL) {
      free_alphabet(alph);
   }
   return 1;
}
enter=u_fopen(BINARY,enter_pos,U_WRITE);
if (enter==NULL) {
   error("Cannot create file %s\n",enter_pos);
   u_fclose(text);
   u_fclose(out);
   if (alph!=NULL) {
      free_alphabet(alph);
   }
   return 1;
}


vector_ptr* tokens=new_vector_ptr(4096);
vector_int* n_occur=new_vector_int(4096);
vector_int* n_enter_pos=new_vector_int(4096);
struct hash_table* hashtable=new_hash_table((HASH_FUNCTION)hash_unichar,(EQUAL_FUNCTION)u_equal,
                                            (FREE_FUNCTION)free,NULL,(KEYCOPY_FUNCTION)keycopy);
if (token_file[0]!='\0') {
   load_token_file(token_file,mask_encoding_compatibility_input,tokens,hashtable,n_occur);
}

output=u_fopen_creating_versatile_encoding(encoding_output,bom_output,tokens_txt,U_WRITE);
if (output==NULL) {
   error("Cannot create file %s\n",tokens_txt);
   u_fclose(text);
   u_fclose(out);
   u_fclose(enter);
   if (alph!=NULL) {
      free_alphabet(alph);
   }

   free_hash_table(hashtable);
   free_vector_ptr(tokens,free);
   free_vector_int(n_occur);
   free_vector_int(n_enter_pos);

   return 1;
}
u_fprintf(output,"0000000000\n");

int SENTENCES=0;
int TOKENS_TOTAL=0;
int WORDS_TOTAL=0;
int DIGITS_TOTAL=0;
u_printf("Tokenizing text...\n");
if (mode==NORMAL) {
   normal_tokenization(text,out,output,alph,tokens,hashtable,n_occur,n_enter_pos,
		   &SENTENCES,&TOKENS_TOTAL,&WORDS_TOTAL,&DIGITS_TOTAL);
}
else {
   char_by_char_tokenization(text,out,output,alph,tokens,hashtable,n_occur,n_enter_pos,
		   &SENTENCES,&TOKENS_TOTAL,&WORDS_TOTAL,&DIGITS_TOTAL);
}
u_printf("\nDone.\n");
save_new_line_positions(enter,n_enter_pos);
u_fclose(enter);
u_fclose(text);
u_fclose(out);
u_fclose(output);
write_number_of_tokens(tokens_txt,encoding_output,bom_output,tokens->nbelems);
// we compute some statistics
get_snt_path(argv[vars->optind],tokens_txt);
strcat(tokens_txt,"stats.n");
output=u_fopen_creating_versatile_encoding(encoding_output,bom_output,tokens_txt,U_WRITE);
if (output==NULL) {
   error("Cannot write %s\n",tokens_txt);
}
else {
   compute_statistics(output,tokens,alph,SENTENCES,TOKENS_TOTAL,WORDS_TOTAL,DIGITS_TOTAL);
   u_fclose(output);
}
// we save the tokens by frequence
get_snt_path(argv[vars->optind],tokens_txt);
strcat(tokens_txt,"tok_by_freq.txt");
output=u_fopen_creating_versatile_encoding(encoding_output,bom_output,tokens_txt,U_WRITE);
if (output==NULL) {
   error("Cannot write %s\n",tokens_txt);
}
else {
   sort_and_save_by_frequence(output,tokens,n_occur);
   u_fclose(output);
}
// we save the tokens by alphabetical order
get_snt_path(argv[vars->optind],tokens_txt);
strcat(tokens_txt,"tok_by_alph.txt");
output=u_fopen_creating_versatile_encoding(encoding_output,bom_output,tokens_txt,U_WRITE);
if (output==NULL) {
   error("Cannot write %s\n",tokens_txt);
}
else {
   sort_and_save_by_alph_order(output,tokens,n_occur);
   u_fclose(output);
}
free_hash_table(hashtable);
free_vector_ptr(tokens,free);
free_vector_int(n_occur);
free_vector_int(n_enter_pos);
if (alph!=NULL) {
   free_alphabet(alph);
}
free_OptVars(vars);
return 0;
}
//---------------------------------------------------------------------------


/**
 * Returns the number of the given token, inserting it if needed in the
 * data structures. Its number of occurrences is also updated.
 */
int get_token_number(unichar* s,vector_ptr* tokens,struct hash_table* hashtable,vector_int* n_occur) {
int ret;
struct any* value=get_value(hashtable,s,HT_INSERT_IF_NEEDED,&ret);
if (ret==HT_KEY_ADDED) {
   /* If the token was not already in the hash table, we must give it
    * a number */
   value->_int=vector_ptr_add(tokens,u_strdup(s));
   vector_int_add(n_occur,0);
}
int n=value->_int;
/* Then we update the number of occurrences */
n_occur->tab[n]++;
return n;
}


/**
 * Loads an existing token file.
 */
void load_token_file(char* filename,int mask_encoding_compatibility_input,vector_ptr* tokens,struct hash_table* hashtable,vector_int* n_occur) {
U_FILE* f=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,filename,U_READ);
if (f==NULL) {
   fatal_error("Cannot open token file %s\n",filename);
}
unichar tmp[1024];
if (EOF==u_fgets_limit2(tmp,1024,f)) {
   fatal_error("Unexpected empty token file %s\n",filename);
}
while (EOF!=u_fgets_limit2(tmp,1024,f)) {
   int n=get_token_number(tmp,tokens,hashtable,n_occur);
   /* We decrease the number of occurrences, in order to have all those numbers equal to 0 */
   n_occur->tab[n]--;
}
u_fclose(f);
}


void normal_tokenization(U_FILE* f,U_FILE* coded_text,U_FILE* output,Alphabet* alph,
                         vector_ptr* tokens,struct hash_table* hashtable,
                         vector_int* n_occur,vector_int* n_enter_pos,
                         int *SENTENCES,int *TOKENS_TOTAL,int *WORDS_TOTAL,
                         int *DIGITS_TOTAL) {
int c;
unichar s[MAX_TAG_LENGTH];
int n;
char ENTER;
int COUNT=0;
int current_megabyte=0;
c=u_fgetc(f);
while (c!=EOF) {
   COUNT++;
   if ((COUNT/(1024*512))!=current_megabyte) {
      current_megabyte++;
      int z=(COUNT/(1024*512));
      u_printf("%d megabyte%s read...       \r",z,(z>1)?"s":"");
   }
   if (c==' ' || c==0x0d || c==0x0a) {
      ENTER=0;
      if (c=='\n') ENTER=1;
      // if the char is a separator, we jump all the separators
      while ((c=u_fgetc(f))==' ' || c==0x0d || c==0x0a) {
        if (c=='\n') ENTER=1;
        COUNT++;
      }
      s[0]=' ';
      s[1]='\0';
      n=get_token_number(s,tokens,hashtable,n_occur);
      /* If there is a \n, we note it */
      if (ENTER==1) {
         vector_int_add(n_enter_pos,*TOKENS_TOTAL);
      }
      (*TOKENS_TOTAL)++;
      fwrite(&n,4,1,coded_text);
   }
   else if (c=='{') {
     s[0]='{';
     int z=1;
     bool protected_char = false; // Cassys add
     while (z < (MAX_TAG_LENGTH - 1) && (((c = u_fgetc(f)) != '}' && c
					!= '{' && c != '\n') || protected_char)) {
			protected_char = false; // Cassys add
			if (c == '\\') { // Cassys add
				protected_char = true; // Cassys add
			} // Cassys add
			s[z++] = (unichar) c;
			COUNT++;
	}

     if (z==(MAX_TAG_LENGTH-1) || c!='}') {
        // if the tag has no ending }
        s[z]='\0';
        fatal_error("Error: a tag without ending } has been found:\n%S\n",s);
     }
     if (c=='\n') {
        // if the tag contains a return
        fatal_error("Error: a tag containing a new-line sequence has been found\n");
     }
     s[z]='}';
     s[z+1]='\0';
     if (!u_strcmp(s,"{S}")) {
        // if we have found a sentence delimiter
        (*SENTENCES)++;
     } else {
        if (u_strcmp(s,"{STOP}") && !check_tag_token(s)) {
           // if a tag is incorrect, we exit
           fatal_error("The text contains an invalid tag. Unitex cannot process it.");
        }
     }
     n=get_token_number(s,tokens,hashtable,n_occur);
     (*TOKENS_TOTAL)++;
     fwrite(&n,4,1,coded_text);
     c=u_fgetc(f);
   }
   else {
      s[0]=(unichar)c;
      n=1;
      if (!is_letter(s[0],alph)) {
         s[1]='\0';
         n=get_token_number(s,tokens,hashtable,n_occur);
         (*TOKENS_TOTAL)++;
         if (c>='0' && c<='9') (*DIGITS_TOTAL)++;
         fwrite(&n,4,1,coded_text);
         c=u_fgetc(f);
      }
      else {
         while ((n<(MAX_TAG_LENGTH-1)) && EOF!=(c=u_fgetc(f)) && is_letter((unichar)c,alph)) {
           s[n++]=(unichar)c;
           COUNT++;
         }
         if (n==(MAX_TAG_LENGTH-1)) {
        	 s[n]='\0';
            error("Token too long at position %d:\n<%S>\n",COUNT,s);
         }
         s[n]='\0';
         n=get_token_number(s,tokens,hashtable,n_occur);
         (*TOKENS_TOTAL)++;
         (*WORDS_TOTAL)++;
         fwrite(&n,4,1,coded_text);
      }
   }
}
for (n=0;n<tokens->nbelems;n++) {
   u_fprintf(output,"%S\n",tokens->tab[n]);
}
}



void char_by_char_tokenization(U_FILE* f,U_FILE* coded_text,U_FILE* output,Alphabet* alph,
                               vector_ptr* tokens,struct hash_table* hashtable,
                               vector_int* n_occur,vector_int* n_enter_pos,
                               int *SENTENCES,int *TOKENS_TOTAL,int *WORDS_TOTAL,
                               int *DIGITS_TOTAL) {
int c;
unichar s[MAX_TAG_LENGTH];
int n;
char ENTER;
int COUNT=0;
int current_megabyte=0;
c=u_fgetc(f);
while (c!=EOF) {
   COUNT++;
   if ((COUNT/(1024*512))!=current_megabyte) {
      current_megabyte++;
      u_printf("%d megabytes read...         \r",(COUNT/(1024*512)));
   }
   if (c==' ' || c==0x0d || c==0x0a) {
      ENTER=0;
      if (c=='\n') {
         ENTER=1;
      }
      // if the char is a separator, we jump all the separators
      while ((c=u_fgetc(f))==' ' || c==0x0d || c==0x0a) {
         if (c=='\n') ENTER=1;
         COUNT++;
      }
      s[0]=' ';
      s[1]='\0';
      n=get_token_number(s,tokens,hashtable,n_occur);
      /* If there is a \n, we note it */
      if (ENTER==1) {
         vector_int_add(n_enter_pos,*TOKENS_TOTAL);
      }
      (*TOKENS_TOTAL)++;
      fwrite(&n,4,1,coded_text);
   }
   else if (c=='{') {
     s[0]='{';
     int z=1;
     while (z<(MAX_TAG_LENGTH-1) && (c=u_fgetc(f))!='}' && c!='{' && c!='\n') {
        s[z++]=(unichar)c;
        COUNT++;
     }
     if (c=='\n') {
        // if the tag contains a return
        fatal_error("Error: a tag containing a new-line sequence has been found\n");
     }
     if (z==(MAX_TAG_LENGTH-1) || c!='}') {
        // if the tag has no ending }
        if (z==(MAX_TAG_LENGTH-1)) {z--;}
        s[z]='\0';
        fatal_error("Error: a tag without ending } has been found:\n==>%S<==\n",s);
     }
     s[z]='}';
     s[z+1]='\0';
     if (!u_strcmp(s,"{S}")) {
        // if we have found a sentence delimiter
        (*SENTENCES)++;
     } else {
        if (u_strcmp(s,"{STOP}") && !check_tag_token(s)) {
           // if a tag is incorrect, we exit
           fatal_error("The text contains an invalid tag. Unitex cannot process it.");
        }
     }
     n=get_token_number(s,tokens,hashtable,n_occur);
     (*TOKENS_TOTAL)++;
     fwrite(&n,4,1,coded_text);
     c=u_fgetc(f);
   }
   else {
      s[0]=(unichar)c;
      s[1]='\0';
      n=get_token_number(s,tokens,hashtable,n_occur);
      (*TOKENS_TOTAL)++;
      if (is_letter((unichar)c,alph)) (*WORDS_TOTAL)++;
      else if (c>='0' && c<='9') (*DIGITS_TOTAL)++;
      fwrite(&n,4,1,coded_text);
      c=u_fgetc(f);
   }
}
for (n=0;n<tokens->nbelems;n++) {
   u_fprintf(output,"%S\n",tokens->tab[n],output);
}
}




int partition_pour_quicksort_by_frequence(int m, int n,vector_ptr* tokens,vector_int* n_occur) {
int pivot;
int tmp;
unichar* tmp_char;
int i = m-1;
int j = n+1; // final pivot index
pivot=n_occur->tab[(m+n)/2];
for (;;) {
  do j--;
  while ((j>(m-1))&&(pivot>n_occur->tab[j]));
  do i++;
  while ((i<n+1)&&(n_occur->tab[i]>pivot));
  if (i<j) {
    tmp=n_occur->tab[i];
    n_occur->tab[i]=n_occur->tab[j];
    n_occur->tab[j]=tmp;

    tmp_char=(unichar*)tokens->tab[i];
    tokens->tab[i]=tokens->tab[j];
    tokens->tab[j]=tmp_char;
  } else return j;
}
}



void quicksort_by_frequence(int first,int last,vector_ptr* tokens,vector_int* n_occur) {
int p;
if (first<last) {
  p=partition_pour_quicksort_by_frequence(first,last,tokens,n_occur);
  quicksort_by_frequence(first,p,tokens,n_occur);
  quicksort_by_frequence(p+1,last,tokens,n_occur);
}
}



int partition_pour_quicksort_by_alph_order(int m, int n,vector_ptr* tokens,vector_int* n_occur) {
unichar* pivot;
unichar* tmp;
int tmp_int;
int i = m-1;
int j = n+1; // final pivot index
pivot=(unichar*)tokens->tab[(m+n)/2];
for (;;) {
  do j--;
  while ((j>(m-1))&&(u_strcmp(pivot,(unichar*)tokens->tab[j])<0));
  do i++;
  while ((i<n+1)&&(u_strcmp((unichar*)tokens->tab[i],pivot)<0));
  if (i<j) {
    tmp_int=n_occur->tab[i];
    n_occur->tab[i]=n_occur->tab[j];
    n_occur->tab[j]=tmp_int;

    tmp=(unichar*)tokens->tab[i];
    tokens->tab[i]=tokens->tab[j];
    tokens->tab[j]=tmp;
  } else return j;
}
}



void quicksort_by_alph_order(int first,int last,vector_ptr* tokens,vector_int* n_occur) {
int p;
if (first<last) {
  p=partition_pour_quicksort_by_alph_order(first,last,tokens,n_occur);
  quicksort_by_alph_order(first,p,tokens,n_occur);
  quicksort_by_alph_order(p+1,last,tokens,n_occur);
}
}




void sort_and_save_by_frequence(U_FILE *f,vector_ptr* tokens,vector_int* n_occur) {
quicksort_by_frequence(0,tokens->nbelems - 1,tokens,n_occur);
for (int i=0;i<tokens->nbelems;i++) {
   u_fprintf(f,"%d\t%S\n",n_occur->tab[i],tokens->tab[i]);
}
}



void sort_and_save_by_alph_order(U_FILE *f,vector_ptr* tokens,vector_int* n_occur) {
quicksort_by_alph_order(0,tokens->nbelems - 1,tokens,n_occur);
for (int i=0;i<tokens->nbelems;i++) {
   u_fprintf(f,"%S\t%d\n",tokens->tab[i],n_occur->tab[i]);
}
}



void compute_statistics(U_FILE *f,vector_ptr* tokens,Alphabet* alph,
		                int SENTENCES,int TOKENS_TOTAL,int WORDS_TOTAL,int DIGITS_TOTAL) {
int DIFFERENT_DIGITS=0;
int DIFFERENT_WORDS=0;
for (int i=0;i<tokens->nbelems;i++) {
   unichar* foo=(unichar*)tokens->tab[i];
   if (u_strlen(foo)==1) {
      if (foo[0]>='0' && foo[0]<='9') DIFFERENT_DIGITS++;
   }
   if (is_letter(foo[0],alph)) DIFFERENT_WORDS++;
}
u_fprintf(f,"%d sentence delimiter%s, %d (%d diff) token%s, %d (%d) simple form%s, %d (%d) digit%s\n",
		SENTENCES,(SENTENCES>1)?"s":"",TOKENS_TOTAL,tokens->nbelems,(TOKENS_TOTAL>1)?"s":"",WORDS_TOTAL,
		DIFFERENT_WORDS,(WORDS_TOTAL>1)?"s":"",DIGITS_TOTAL,DIFFERENT_DIGITS,(DIGITS_TOTAL>1)?"s":"");
}



void save_new_line_positions(U_FILE* f,vector_int* n_enter_pos) {
fwrite(n_enter_pos->tab,sizeof(int),n_enter_pos->nbelems,f);
}
