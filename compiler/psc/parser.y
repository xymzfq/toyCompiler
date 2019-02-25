%{
	#include "node.h"
        #include <cstdio>
        #include <cstdlib>
	NBlock *programBlock; /* the top level root node of our final AST */

	extern int yylex();
	void yyerror(const char *s) { std::printf("Error: %s\n", s);std::exit(1); }
%}

/* Represents the many different ways we can access our data */
%union {
	Node *node;
	NBlock *block;
	NExpression *expr;
	NStatement *stmt;
	NIdentifier *ident;
	NVariableDeclaration *var_decl;
	std::vector<NVariableDeclaration*> *varvec;
	std::vector<NExpression*> *exprvec;
	std::string *string;
	int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */

%token <string> TIDENTIFIER TINTEGER TLITERAL
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL TEQUAL_EX
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT TSEMICOLON TCOLON
%token <token> TPLUS TMINUS TMUL TDIV
%token <token> TRETURN TEXTERN TIF TELSE TFOR TSTRUCT TVAR TBEGIN TEND

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <ident> ident
%type <expr> numeric expr assign

%type <exprvec> call_args
%type <block> program stmts block
%type <stmt> stmt var_decl func_decl


/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV

%start program


%%

program : stmts { programBlock = $1; }
				;

stmts : stmt { $$ = new NBlock(); $$->statements.push_back(($1)); }
			| stmts stmt  { $1->statements.push_back(($2)); }
			;

stmt : var_decl | func_decl
		 | expr { $$ = new NExpressionStatement((*$1)); }
		 ;

block : TLBRACE stmts TRBRACE { $$ = $2; }
			| TLBRACE TRBRACE { $$ = new NBlock(); }
			;


var_decl : TVAR ident TCOLON ident { $$ = new NVariableDeclaration((*$4),(*$2), nullptr); }
				 ;

func_decl : TBEGIN stmts TEND
				{ $$ = new NFunctionDeclaration(*(new NIdentifier("void")), *(new NIdentifier("main")), *(new VariableList()), *($2));  }

ident : TIDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
			;

numeric : TINTEGER { $$ = new NInteger(atol($1->c_str())); }
				;

expr : 	ident TEQUAL_EX expr { $$ = new NAssignment(*$<ident>1, *$3); }
		 | ident TLPAREN call_args TRPAREN { $$ = new NMethodCall(*($1), *($3)); }
		 | ident { $<ident>$ = $1; }
		 | numeric
		 | expr TMUL expr { $$ = new NBinaryOperator(*($1), $2, *($3)); }
		 | expr TDIV expr { $$ = new NBinaryOperator(*($1), $2, *($3)); }
		 | expr TPLUS expr { $$ = new NBinaryOperator(*($1), $2, *($3)); }
		 | expr TMINUS expr { $$ = new NBinaryOperator(*($1), $2, *($3)); }
		 ;


call_args : /*blank*/  { $$ = new ExpressionList(); }
	  | expr { $$ = new ExpressionList(); $$->push_back($1); }
	  ;

%%
