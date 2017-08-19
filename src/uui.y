%{
#include <common.h>
#include <uui/components.h>
#include <lib/strhashmap.h>

#include "uui_parser.h"
#include "uui_lexer.h"

void yyerror (YYLTYPE *locp, yyscan_t yyscanner, char const *s);

static void uui_object_tryinit(uui_component_parsed_t** pobj);

size_t fileoffset = 0;
size_t linestart = 0;
static Hashmap *imports = NULL;
static uui_component_parsed_t *parsed_component = NULL;

%}
%define parse.error verbose
%locations %define api.pure full
%lex-param {void *scanner}
%parse-param {void *scanner}

%union {
    char *str;
    int64_t i64;
    double fp; // TODO: use softfloat lib
    uui_component_value_t *uui_value;
    uui_component_parsed_t *uui_object;
}

%type <str> STRING IDENTIFIER IDENTIFIER_CHAIN id_or_idchain IMPORT optas
%type <uui_value> value
%type <uui_object> members object rootobject;

%token NUMBER FP_NUMBER
%token STRING
%token IDENTIFIER IDENTIFIER_CHAIN
%token TK_TRUE TK_FALSE TK_NULL
%token IMPORT
%token AS
%left CBRACE_START CBRACE_END
%left COLON
%left SEMICOLON
%left NEWLINE
%left POINT

%%
start : newlines rootobject newlines {
            parsed_component = $2;
        }
      ;

rootobject : newlines rootobject { $$ = $2; }
           | object { $$ = $1; }
           | IMPORT id_or_idchain optas delimiter rootobject {
                $$ = $5;
                char *last = AsciiStrRChr($2, '.');
                if(last==NULL) last = $2;
                else last++;
                hashmapPut(imports, $3?:last, $2);
             }
           ;

optas  : { $$ = NULL; }
       | AS IDENTIFIER { $$ = $2; }
       ;

newlines  :
          | NEWLINE newlines
          ;

object : id_or_idchain newlines CBRACE_START members CBRACE_END {
             $$ = $4;
             uui_object_tryinit(&$$);
             $$->typestr = $1;
         }
       ;

delimiter : SEMICOLON
          | NEWLINE
          ;

id_or_idchain : IDENTIFIER
              | IDENTIFIER_CHAIN
              ;

members : { $$ = NULL; }
        | delimiter members { $$ = $2; }
        | object members {
            $$ = $2;
            uui_object_tryinit(&$$);
            list_add_head(&$$->children, &$1->node);
          }
        | id_or_idchain COLON value delimiter members {
            $$ = $5;
            uui_object_tryinit(&$$);
            hashmapPut($$->default_properties, $1, $3);
          }
        | id_or_idchain COLON value {
            $$ = NULL;
            uui_object_tryinit(&$$);
            hashmapPut($$->default_properties, $1, $3);
          }
        ;

value   : STRING {
              $$ = yyalloc(sizeof(uui_component_valuetype_t), scanner);
              $$->type = UUI_COMPONENT_VALUETYPE_STRING;
              $$->u.str = yylval.str;
          }
        | NUMBER {
              $$ = yyalloc(sizeof(uui_component_valuetype_t), scanner);
              $$->type = UUI_COMPONENT_VALUETYPE_NUMBER;
              $$->u.i64 = yylval.i64;
          }
        | FP_NUMBER {
              $$ = yyalloc(sizeof(uui_component_valuetype_t), scanner);
              $$->type = UUI_COMPONENT_VALUETYPE_FP_NUMBER;
              $$->u.fp = yylval.fp;
          }
        | TK_TRUE  {
              $$ = yyalloc(sizeof(uui_component_valuetype_t), scanner);
              $$->type = UUI_COMPONENT_VALUETYPE_BOOLEAN;
              $$->u.boolean = TRUE;
          }
        | TK_FALSE {
              $$ = yyalloc(sizeof(uui_component_valuetype_t), scanner);
              $$->type = UUI_COMPONENT_VALUETYPE_BOOLEAN;
              $$->u.boolean = FALSE;
          }
        | IDENTIFIER {
              $$ = yyalloc(sizeof(uui_component_valuetype_t), scanner);
              $$->type = UUI_COMPONENT_VALUETYPE_IDENTIFIER;
              $$->u.str = yylval.str;
          }
        ;
%%

static void uui_object_tryinit(uui_component_parsed_t** pobj) {
    if ((*pobj) == NULL) {
        uui_component_parsed_t *obj = AllocateZeroPool(sizeof(uui_component_parsed_t));
        obj->typestr = NULL;
        list_initialize(&obj->children);
        obj->default_properties = strHashmapCreate(5);
        *pobj = obj;
    }
}

uui_component_parsed_t * uui_comp_internal_parse_component(uui_comp_context_t *context, const char *filename, Hashmap **pimports) {
    yyscan_t scanner;
    urc_file_t *f;

    if (context->urc->open(context->urc, filename, &f))
        return NULL;

    parsed_component = NULL;
    imports = strHashmapCreate(5);
    fileoffset = 0;
    linestart = 0;

    yylex_init(&scanner);
    yyset_in(f, scanner);
    yyset_out((void*)0x1, scanner);
    yyparse(scanner);
    yylex_destroy(scanner);

    *pimports = imports;
    return parsed_component;
}

int yywrap(yyscan_t yyscanner) {
    return 1;
}

void yyerror (YYLTYPE *locp, yyscan_t yyscanner, char const *s) {
    size_t outsize;
    int rc;
    int i;
    int c = 0;
    DEBUG((DEBUG_ERROR, "\033[1;37mFILE:%d:%d:\033[0m \033[1;31merror:\033[0m %"FMTS"\n", locp->first_line, locp->first_column, s));

    urc_file_t *f = yyget_in(yyscanner);
    if (linestart != fileoffset) {
        // seek to erroneous line
        outsize = linestart;
        rc = f->seek(f, &outsize, URC_SEEK_SET);
        if (rc) return;

        // print all characters up to the error
        DEBUG((DEBUG_ERROR, " "));
        for(i=0; i<locp->first_column-1; i++) {
            outsize = 1;
            rc = f->read(f, &c, &outsize);
            if (rc) return;

            DEBUG((DEBUG_ERROR, "%c", c));
        }

        // print the erroneous part in color
        DEBUG((DEBUG_ERROR, "\033[1;35m"));
        for(; i<locp->last_column-1; i++) {
            outsize = 1;
            rc = f->read(f, &c, &outsize);
            if (rc) return;

            if (c!='\n')
                DEBUG((DEBUG_ERROR, "%c", c));
        }
        DEBUG((DEBUG_ERROR, "\033[0m"));

        // print the rest of the line
        if (c!='\n') {
            for(;;) {
                outsize = 1;
                rc = f->read(f, &c, &outsize);
                if (rc) return;

                if (c=='\n') break;
                DEBUG((DEBUG_ERROR, "%c", c));
            }
        }
        DEBUG((DEBUG_ERROR, "\n", c));

        // print whitespace until we're in front of the error in the line afterwards
        for(i=0; i<locp->first_column; i++)
            DEBUG((DEBUG_ERROR, " ", c));

        // print a hint marker
        DEBUG((DEBUG_ERROR, "\033[1;35m^"));
        for(; i<locp->last_column-1; i++)
            DEBUG((DEBUG_ERROR, "~", c));
        DEBUG((DEBUG_ERROR, "\033[0m\n"));
    }
}

#ifdef YY_BARE_METAL
void *yyalloc ( yy_size_t s, yyscan_t yyscanner ) {
    return AllocatePool(s);
}

void *yyrealloc ( void *ptr, yy_size_t oldsize, yy_size_t newsize, yyscan_t yyscanner ) {
    return ReallocatePool(oldsize, newsize, ptr);
}

void yyfree ( void * ptr, yyscan_t yyscanner ) {
    if (ptr)
        FreePool(ptr);
}

void __attribute__((__noreturn__)) yy_platform_fatal_error(const char *msg) {
    DEBUG((DEBUG_ERROR, "yylex error: "FMTS"\n", msg));
    ASSERT(FALSE);
    for(;;);
}

int yy_platform_input(void* yyin, char *buf, int max_size) {
    urc_file_t *file = yyin;
    int rc;
    size_t size = (size_t)max_size;

    rc = file->read(file, buf, &size);
    if (rc) return rc;

    return (int)size;
}

void yy_platform_output(void* yyout, const char *buf, int size) {
    int i;
    for (i=0; i<size; i++) {
        DEBUG((DEBUG_INFO, "%c", buf[i]));
    }
}

void * yy_platform_memset ( void * ptr, int value, yy_size_t num ) {
    SetMem(ptr, num, value);
    return ptr;
}

yy_size_t yy_platform_strlen ( const char * str ) {
    return AsciiStrLen(str);
}
#endif
