%{
#include <string>
#include <cstdio>
#include <iostream>
#include "node.h"
#include "parser.hpp"



%}

using namespace std;

%option noyywrap
%option yylineno

%x comment
%x linecomment
%%


"//".*  { }

"/*" {BEGIN(comment); }	
<comment>.|"\n"	
<comment><<EOF>> { yyterminate(); } 
<comment>"*"+"/" { BEGIN(INITIAL); }
	
[ \t\r] {}
[\n] { yylineno+1; }

[();,{}.] { return yytext[0]; }

"int"|"double"|"char"|"string"|"boolean"|"void" {yylval.a = strdup(yytext); return TYPE_T;} 
 
"new" 		{ return NEW_T; } 
"class" 	{ return CLASS_T; }                               
"else" 		{ return ELSE_T; }                       
"if" 		{ return IF_T; }           
"return" 	{ return RETURN_T; }                      
"while" 	{ return WHILE_T; } 
"null" 		{ return NULL_T; }
"next"		{ return NEXT_T; }

"+" { return SUM_T; }
"-" { return MIN_T; }
"*" { return MUL_T; }
"/" { return DIV_T; }
"<" { return LT_T; }
">" { return GT_T; }
"=" { return ASSIGN_T; }

"==" 		{ return EQUAL_T;}
"<=" 		{ return LE_T;}
">=" 		{ return GE_T;}
"!=" 		{ return NOTEQUAL_T;}
"&&" 		{ return AND_T;}
"||" 		{ return OR_T;}

"Scanner" 				{ return SCANNER_T;} 
"System.out.println"	{ return PRINT_T;} 
"System.in" 			{ return SYSTEMIN_T;}       


"define" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"abstract" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"assert" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"continue" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"static" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"extern" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"break" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"byte" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"case" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"catch" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"do" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"for" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"enum" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"extends" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"final" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"finally" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"float" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"import" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"implements" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"interface" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"long" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"package" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"protected" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"public" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"short" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"super" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"strictfp" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"switch" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"synchronized" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"this" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"throw" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"throws" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"transient" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"try" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"volatile" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"goto" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0); }
"const" { printf("line %d - error: use of reserved word '%s' as identifier \n", yylineno, yytext); exit(0);}

"true"|"false" 			{yylval.a = strdup(yytext); return BOOL_T;}

[a-zA-Z_][a-zA-Z0-9_]*  	{yylval.a = strdup(yytext); return IDENT_T;}

([0]|[1-9][0-9]*) 			{ yylval.a = strdup(yytext); return INT_T;}

([0-9]*)[.]?[0-9]+ 			{ yylval.a = strdup(yytext); return DOUBLE_T;}

[']([^"\\\r\n]|[\\][btnfr"'\\])['] 	{ yylval.a= strdup(yytext);; return CHAR_T;}

["]([^"\\\r\n]|[\\][btnfr"'\\])*["] { yylval.a = strdup(yytext); return STR_T;}


%%
