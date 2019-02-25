%{
#include <string>
#include "node.h"
#include "parser.hpp"

#define SAVE_TOKEN  yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t)    (yylval.token = t)
%}

%option noyywrap

%%

[ \t\n]					        ;
"function" 							puts("TFUNC"); return TOKEN(TFUNC);
"var"  									puts("TVAR"); return TOKEN(TVAR);
"return"				        return TOKEN(TRETURN);
"extern"                SAVE_TOKEN; puts("TEXTERN"); return TEXTERN;
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return TIDENTIFIER;
[0-9]+					        SAVE_TOKEN; return TINTEGER;



"="						          return TOKEN(TEQUAL);
"=="										puts("TCEQ"); return TOKEN(TCEQ);
"!="                    puts("TCNE"); return TOKEN(TCNE);
"<"                     puts("TCLT"); return TOKEN(TCLT);
"<="                    puts("TCLE"); return TOKEN(TCLE);
">"                     puts("TCGT"); return TOKEN(TCGT);
">="                    puts("TCGE"); return TOKEN(TCGE);
"("	          					return TOKEN(TLPAREN);
")"					          	return TOKEN(TRPAREN);
"{"         						return TOKEN(TLBRACE);
"}"					          	return TOKEN(TRBRACE);

","				          		return TOKEN(TCOMMA);

"."                     puts("TDOT"); return TOKEN(TDOT);


"+"				          		return TOKEN(TPLUS);
"-"		          				return TOKEN(TMINUS);
"*"		          				return TOKEN(TMUL);
"/"				          		return TOKEN(TDIV);

.                       printf("Unknown token!\n"); yyterminate();

%%

