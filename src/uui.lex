%{
#include <common.h>
#include <uui/components.h>
#include "uui_parser.h"

void yyerror (YYLTYPE *locp, yyscan_t yyscanner, char const *s);

static void comment(yyscan_t yyscanner);
static void linecomment(yyscan_t yyscanner);
static int64_t str2i64(const char *s);
static double str2double(const char *s);

extern size_t fileoffset;
extern size_t linestart;

#define YY_USER_ACTION { \
        fileoffset += yyleng; \
        yylloc->first_line = yylineno; \
        yylloc->first_column = yycolumn; \
        yycolumn += yyleng; \
        yylloc->last_column = yycolumn; \
        yylloc->last_line = yylineno; \
    }

#define COMMENT_ACTION { \
    fileoffset++; \
    yycolumn++; \
    if (c=='\n') { \
        linestart = fileoffset; \
        yycolumn = 1; \
    } \
}
%}

%option reentrant bison-bridge bison-locations yylineno

D   [0-9]
L   [a-zA-Z_]
H   [a-fA-F0-9]
E   [Ee][+-]?{D}+
ID {L}({L}|{D})*

%%
\/\*     { comment(yyscanner); }
\/\/     { linecomment(yyscanner); }

"true"   return TK_TRUE;
"false"  return TK_FALSE;
"null"   return TK_NULL;
"import" return IMPORT;
"as"     return AS;

{ID} {
            yylval->str = AsciiStrDup(yytext);
            return IDENTIFIER;
         };

{ID}(\.|{ID})* {
            yylval->str = AsciiStrDup(yytext);
            return IDENTIFIER_CHAIN;
         };

0[xX]{H}+ {
            yylval->i64 = str2i64(yytext);
            return NUMBER;
         }
0{D}+    {
            yylval->i64 = str2i64(yytext);
            return NUMBER;
         }
{D}+     {
            yylval->i64 = str2i64(yytext);
            return NUMBER;
         }
'(\\.|[^\\'])+' {
            yylval->i64 = str2i64(yytext);
            return NUMBER;
         }

{D}+{E}  {
            yylval->fp = str2double(yytext);
            return FP_NUMBER;
         }
{D}*"."{D}+({E})? {
            yylval->fp = str2double(yytext);
            return FP_NUMBER;
         }
{D}+"."{D}*({E})? {
            yylval->fp = str2double(yytext);
            return FP_NUMBER;
         }

\"(\\.|[^\\"])*\" {
            size_t l = AsciiStrLen(yytext) - 2;
            yylval->str = yyalloc(l + 1, yyscanner);
            CopyMem(yylval->str, yytext+1, l);
            yylval->str[l] = '\0';
            return STRING;
         }

"{"      return CBRACE_START;
"}"      return CBRACE_END;
"."      return POINT;
":"      return COLON;
";"      return SEMICOLON;

\n       { linestart = fileoffset; return NEWLINE; }
[ \t]+ /* throw away whitespace */;
.        {
            yyerror(yylloc, yyscanner, "Illegal character"); 
            return -1;
         }
%%

static void comment(yyscan_t yyscanner) {
    char c, c1;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    (void)(yyg);

loop:
    while ((c = input(yyscanner)) != '*' && c != 0) {
        COMMENT_ACTION;
    }
    COMMENT_ACTION;

    if ((c1 = input(yyscanner)) != '/' && c != 0) {
        unput(c1);
        goto loop;
    }

    COMMENT_ACTION;
}

static void linecomment(yyscan_t yyscanner) {
    char c;
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    (void)(yyg);

    while ((c = input(yyscanner)) != '\n' && c != 0) {
        COMMENT_ACTION;
    }

    unput(c);
}

static int64_t str2i64(const char *s) {
    return (int64_t)AsciiStrToLL(s, NULL, 0);
}

static double str2double(const char *s) {
    //return strtod(s, NULL);
    // TODO: use softfloat lib
    return 0;
}
