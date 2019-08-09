%{
	
	#include <string>
	#include <iostream>	
	#include <vector>
	#include <cstdio>
    #include <cstdlib>

	#include "node.h"

	using namespace std;
	
	Node* goalBlock;

	bool err=false;

	/* variabile aggiunta per evitare print di errori che generano loop */
	int err_line;

	vector<string> scanners;

	vector<Node*> global;
	vector<Node*> methods;

	extern int yylineno;
	extern int yylex();
	void yyerror(const char *s) { err=true; printf("line: %d - %s\n", yylineno, s); }
%}

%error-verbose

%union {
		
	Node* node;
	Vec* vec;
	char *a;

	char op;
	int token;
}

%token <a> IDENT_T STR_T TYPE_T BOOL_T INT_T DOUBLE_T CHAR_T

%token <token> GE_T LE_T EQUAL_T NOTEQUAL_T AND_T OR_T 
%token <token> RETURN_T IF_T ELSE_T WHILE_T PRINT_T NEW_T SYSTEMIN_T SCANNER_T NEXT_T CLASS_T NULL_T
%token <token> SUM_T MIN_T MUL_T DIV_T ASSIGN_T LT_T GT_T

%type <node> Goal
%type <node> ClassDeclaration
%type <node> Identifier
%type <node> Primary
%type <node> WhileStmt
%type <node> IfStmt
%type <node> ReturnStmt
%type <node> PrintStmt
%type <node> Expression
%type <node> ScannerStmt
%type <node> Assignment

%type <node> VariableAssignment
%type <node> VariableDeclaration
%type <node> FunctionDeclaration


%type <vec> FunctionContent
%type <vec> CallArguments
%type <vec> ArgumentsDefinition


%right ASSIGN_T
%left AND_T OR_T
%left LT_T GT_T EQUAL_T NOTEQUAL_T GE_T LE_T
%left SUM_T MIN_T
%left MUL_T DIV_T


%start Goal

%%

Goal
	: ClassDeclaration 								{ if(err) exit(1); goalBlock = $1;  }
	;

ClassDeclaration
	: CLASS_T IDENT_T '{' ClassContent '}' 			{ reverse(global.begin(), global.end()); reverse(methods.begin(), methods.end()); $$ = new ClassNode(yylineno, $2, global, methods); global.clear(); }
	;
	
ClassContent
	: VariableDeclaration ';' 						{ global.push_back($1); }
	| FunctionDeclaration							{ methods.push_back($1); }
	| VariableDeclaration ';' ClassContent			{ global.push_back($1); }
	| FunctionDeclaration ClassContent				{ methods.push_back($1); }
	| error ClassContent 							{ if(err_line==yylineno) exit(1); err_line=yylineno; yyerrok;}
	;

VariableDeclaration
	: TYPE_T IDENT_T 					{ $$ = new VariableDec(yylineno, $1, $2); }
	;
	
VariableAssignment
	: TYPE_T IDENT_T ASSIGN_T Expression		{ $$ = new VariableDec(yylineno, $1, $2, $4); }
	| TYPE_T IDENT_T ASSIGN_T Identifier 	{ $$ = new VariableDec(yylineno, $1, $2, $4); }
	;
	
Primary
	: INT_T 			{ $$ = new IntegerNode(yylineno, $1); }
	| DOUBLE_T 			{ $$ = new DoubleNode(yylineno, $1); }
	| CHAR_T 			{ $$ = new CharNode(yylineno, $1); }
	| BOOL_T 			{ $$ = new BoolNode(yylineno, $1); }
	| STR_T 			{ $$ = new StringNode(yylineno, $1); }
	| IDENT_T			{ $$ = new IdentifierNode(yylineno, $1); }
	;
	
FunctionDeclaration
	: TYPE_T IDENT_T '(' ArgumentsDefinition ')' '{' FunctionContent '}' 	{ $$ = new FunctionDec(yylineno, $1, $2, $4->nodes, $7->nodes); }
	| TYPE_T IDENT_T '(' ')' '{' FunctionContent '}' 						{ $$ = new FunctionDec(yylineno, $1, $2, $6->nodes); }
	| TYPE_T IDENT_T  error '{' FunctionContent '}' 						{ if(err_line==yylineno) exit(1); err_line=yylineno; yyerrok;}
	;
	
ArgumentsDefinition
	: VariableDeclaration							{ $$=new Vec(); $$->push($1); }
	| ArgumentsDefinition ',' VariableDeclaration	{ $$->push($3); }
	;
	
FunctionContent
	: /* blank */									{ $$ = new Vec(); }
	| FunctionContent VariableDeclaration ';'		{ $$->push($2); }
	| FunctionContent VariableAssignment ';'		{ $$->push($2); }
	| FunctionContent ScannerStmt ';'				{ $$ = $$; }
	| FunctionContent Identifier ';'				{ $$->push($2); }
	| FunctionContent PrintStmt ';'					{ $$->push($2); }
	| FunctionContent IfStmt						{ $$->push($2); }
	| FunctionContent WhileStmt						{ $$->push($2); }
	| FunctionContent ReturnStmt ';'				{ $$->push($2); }
	| FunctionContent Assignment ';'				{ $$->push($2); }
	| FunctionContent error							{ if(err_line==yylineno) exit(1); err_line=yylineno; yyerrok;}
	;

Assignment
	: IDENT_T ASSIGN_T Expression			{ $$ = new Assignment(yylineno, $1, $3); }
	| IDENT_T ASSIGN_T Identifier			{ $$ = new Assignment(yylineno, $1, $3); }
	| error 								{ if(err_line==yylineno) exit(1); err_line=yylineno; yyerrok;}
	;

ReturnStmt
	: RETURN_T Expression				{ $$ = new ReturnNode(yylineno, $2); }
	;
	
PrintStmt
	: PRINT_T '(' STR_T ')'				{ $$ = new PrintNode(yylineno, NULL, new StringNode(yylineno, $3), NULL); }
	| PRINT_T '(' INT_T ')'				{ $$ = new PrintNode(yylineno, new IntegerNode(yylineno, $3), NULL, NULL); }
	| PRINT_T '(' DOUBLE_T ')'			{ $$ = new PrintNode(yylineno, new DoubleNode(yylineno, $3), NULL, NULL); }
	| PRINT_T '(' CHAR_T ')'			{ $$ = new PrintNode(yylineno, new DoubleNode(yylineno, $3), NULL, NULL); }
	| PRINT_T '(' IDENT_T ')'			{ $$ = new PrintNode(yylineno, NULL, NULL, new IdentifierNode(yylineno, $3)); }
	| PRINT_T  error 					{ if(err_line==yylineno) exit(1); err_line=yylineno; yyerrok;}
	;
	
IfStmt
	: IF_T '(' Expression ')' '{' FunctionContent '}'										{ $$ = new IfNode(yylineno, $3, $6->nodes); }
	| IF_T '(' Expression ')' '{' FunctionContent '}' ELSE_T '{' FunctionContent '}'		{ $$ = new IfNode(yylineno, $3, $6->nodes, $10->nodes); }
	| IF_T error '{' FunctionContent '}'													{ if(err_line==yylineno) exit(1); err_line=yylineno; yyerrok;}
	| IF_T error '{' FunctionContent '}' ELSE_T	'{' FunctionContent '}'						{ if(err_line==yylineno) exit(1); err_line=yylineno; yyerrok;}
	;
	
WhileStmt
	: WHILE_T '(' Expression ')' '{' FunctionContent '}'			{ $$ = new WhileNode(yylineno, $3, $6->nodes); }
	| WHILE_T error '{' FunctionContent '}'							{ if(err_line==yylineno) exit(1); err_line=yylineno; yyerrok;}
	;
	
ScannerStmt
	: SCANNER_T IDENT_T ASSIGN_T NEW_T SCANNER_T '(' SYSTEMIN_T ')'		{ scanners.push_back($2); }
	| SCANNER_T error 						{ if(err_line==yylineno) exit(1); err_line=yylineno; yyerrok;}
	;
	
Identifier
	: IDENT_T '.' NEXT_T '(' ')'			{ $$ = new InputNode(yylineno, $1, scanners); }
	| IDENT_T '(' CallArguments ')'			{ $$ = new FunctionCall(yylineno, $1, $3->nodes); }
	| IDENT_T '(' ')'						{ $$ = new FunctionCall(yylineno, $1); }
	| IDENT_T '(' error ')'					{ if(err_line==yylineno) exit(1); err_line=yylineno; yyerrok;}
	;
	
CallArguments
	: Expression 						{ $$ = new Vec(); $$->push($1); }
	| CallArguments ',' Expression		{ $$->push($3); }
	;
	
Expression
	: Primary								{ $$ = $1; }
	| '(' Expression ')'					{ $$ = $2; }
	| Expression SUM_T Expression 			{ $$ = new BinOperator(yylineno, SUM_T, $1, $3); }
	| Expression MIN_T Expression 			{ $$ = new BinOperator(yylineno, MIN_T, $1, $3); }
	| Expression MUL_T Expression 			{ $$ = new BinOperator(yylineno, MUL_T, $1, $3); }
	| Expression DIV_T Expression 			{ $$ = new BinOperator(yylineno, DIV_T, $1, $3); }
	| Expression LT_T Expression 			{ $$ = new BinOperator(yylineno, LT_T, $1, $3); }
	| Expression GT_T Expression 			{ $$ = new BinOperator(yylineno, GT_T, $1, $3); }
	| Expression GE_T Expression 			{ $$ = new BinOperator(yylineno, GE_T, $1, $3); }
	| Expression LE_T Expression 			{ $$ = new BinOperator(yylineno, LE_T, $1, $3); }
	| Expression AND_T Expression 			{ $$ = new BinOperator(yylineno, AND_T, $1, $3); }
	| Expression OR_T Expression 			{ $$ = new BinOperator(yylineno, OR_T, $1, $3); }
	| Expression EQUAL_T Expression 		{ $$ = new BinOperator(yylineno, EQUAL_T, $1, $3); }
	| Expression NOTEQUAL_T Expression 		{ $$ = new BinOperator(yylineno, NOTEQUAL_T, $1, $3); }
	| '(' error ')'							{ if(err_line==yylineno) exit(1); err_line=yylineno; yyerrok;}
	;
