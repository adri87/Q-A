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
#include "Copyright.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "BuildKrMwuDic.h"
#include "KrMwuDic.h"
#include "Alphabet.h"
#include "Korean.h"
#include "MF_Global.h"
#include "MF_InflectTransd.h"
#include "MF_LangMorpho.h"
#include "File.h"

const char* usage_BuildKrMwuDic =
        "Usage: BuildKrMwuDic [OPTIONS] <dic>\n"
        "\n"
        "  <dic>: text file describing a Korean compound word DELAS\n"
        "\n"
        "OPTIONS:\n"
        "  -o GRF/--output=GRF: output .grf file to produce\n"
        "  -d DIR/--directory=DIR: specifies the directory that contains the inflection graphs\n"
        "                          required to produce morphological variants of roots\n"
        "  -a ALPH/--alphabet=ALPH: specifies the alphabet file to use\n"
        "  -b BIN/--binary=BIN: binary simple word dictionary\n"
        "\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_BuildKrMwuDic);
}





const char* optstring_BuildKrMwuDic="o:d:a:b:hk:q:";
const struct option_TS lopts_BuildKrMwuDic[]= {
      {"output",required_argument_TS,NULL,'o'},
      {"directory",required_argument_TS,NULL,'d'},
      {"alphabet",required_argument_TS,NULL,'a'},
      {"binary",required_argument_TS,NULL,'b'},
      {"help",no_argument_TS,NULL,'h'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {NULL,no_argument_TS,NULL,0}
};


/**
 * The same than main, but no call to setBufferMode.
 */
int main_BuildKrMwuDic(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}


int val,index=-1;
char output[FILENAME_MAX]="";
char inflection_dir[FILENAME_MAX]="";
char alphabet[FILENAME_MAX]="";
char dic_bin[FILENAME_MAX]="";
char dic_inf[FILENAME_MAX]="";
Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
int bom_output = DEFAULT_BOM_OUTPUT;
int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_BuildKrMwuDic,lopts_BuildKrMwuDic,&index,vars))) {
   switch(val) {
   case 'o': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty output file name\n");
             }
             strcpy(output,vars->optarg);
             break;
   case 'd': if (vars->optarg[0]=='\0') {
                fatal_error("Empty inflection directory\n");
             }
             strcpy(inflection_dir,vars->optarg);
             break;
   case 'a': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty alphabet file name\n");
             }
             strcpy(alphabet,vars->optarg);
             break;
   case 'b': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a non empty binary dictionary name\n");
             }
             strcpy(dic_bin,vars->optarg);
             remove_extension(dic_bin,dic_inf);
             strcat(dic_inf,".inf");
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_BuildKrMwuDic[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
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
   }
   index=-1;
}
if (vars->optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}
if (output[0]=='\0') {
   fatal_error("Output file must be specified\n");
}
if (inflection_dir[0]=='\0') {
   fatal_error("Inflection directory must be specified\n");
}
if (alphabet[0]=='\0') {
   fatal_error("Alphabet file must be specified\n");
}
if (dic_bin[0]=='\0') {
   fatal_error("Binary dictionary must be specified\n");
}

U_FILE* delas=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,argv[vars->optind],U_READ);
if (delas==NULL) {
   fatal_error("Cannot open %s\n",argv[vars->optind]);
}
U_FILE* grf=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,output,U_WRITE);
if (grf==NULL) {
   fatal_error("Cannot open %s\n",output);
}
Alphabet* alph=load_alphabet(alphabet,1);
if (alph==NULL) {
   fatal_error("Cannot open alphabet file %s\n",alphabet);
}
Korean* korean=new Korean(alph);
MultiFlex_ctx* multiFlex_ctx = (MultiFlex_ctx*)malloc(sizeof(MultiFlex_ctx));
if (multiFlex_ctx==NULL) {
   fatal_alloc_error("main_BuildKrMwuDic");
}
strcpy(multiFlex_ctx->inflection_directory,inflection_dir);
if (init_transducer_tree(multiFlex_ctx)) {
   fatal_error("init_transducer_tree error\n");
}
struct l_morpho_t* pL_MORPHO=init_langage_morph();
if (pL_MORPHO == NULL) {
   fatal_error("init_langage_morph error\n");
}

unsigned char* bin=load_BIN_file(dic_bin);
struct INF_codes* inf=load_INF_file(dic_inf);

create_mwu_dictionary(delas,grf,multiFlex_ctx,korean,pL_MORPHO,encoding_output,
       bom_output,mask_encoding_compatibility_input,bin,inf);

free(bin);
free_INF_codes(inf);
u_fclose(delas);
u_fclose(grf);
free_alphabet(alph);
delete korean;
free_transducer_tree(multiFlex_ctx);
for (int count_free_fst2=0;count_free_fst2<multiFlex_ctx->n_fst2;count_free_fst2++) {
    free_abstract_Fst2(multiFlex_ctx->fst2[count_free_fst2],&(multiFlex_ctx->fst2_free[count_free_fst2]));
    multiFlex_ctx->fst2[count_free_fst2]=NULL;
}
free_language_morpho(pL_MORPHO);
free(multiFlex_ctx);
free_OptVars(vars);
u_printf("Done.\n");
return 0;
}

