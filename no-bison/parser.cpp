//////////////////////////////////////////////////////////
//------------------------PARSER------------------------//
//////////////////////////////////////////////////////////
//current token parser is reading
static int cur_token;
//read next token from lexer and update current token
static int next_token() { return cur_token = getToken(); }
//precedence for binary operators
static std::map<char, int> OpPrecedence;
//get the precedence from OpPrecedence
static int getTokenPrec()
{
	if(!isascii(cur_token)) return -1;
	int prec = OpPrecedence[cur_token];
	if(prec <= 0) return -1;
	return prec;
}
///////////////////////DECLARATIONS/////////////////////
static Node* ParseExpression();
static Node* ParsePrimary();
static Node* ParseClass();
static Node* ParseReturn();
static Node* ParseFunctionContent();
static Node* ParseIfStmt();
static Node* ParseWhileStmt();
static Node* ParsePrintStmt();
static IdentifierNode* ParsePrintContent();
static void ParseScannerStmt();

//vettore con i nomi delle variabili di tipo scanner definite
//non inserite all'interno del codegen in quanto sono parte integrante di java e non esistono in llvm
static vector<string> scanner_variables;

//dichiarazione di funzioni usate successivamente
//typeOf restituisce il Type corrispondente al nome passato es. typeOf("int") == Type::getInt32Ty(MyContext)
static Type* typeOf(string type);
//LogError restituisce a vista un messaggio di errore con la stringa inserita e la linea del file in cui si verifica
//inoltre interrompe immediatamente l'esecuzione del compilatore
static void LogError(string str);
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
static IntegerNode* ParseInteger()
{
	IntegerNode* result = new IntegerNode(IntVal);
	next_token();
	return result;	
}
////////////////////////////////////////////////////////////////////////////////////////
static CharNode* ParseChar()
{
	CharNode* result = new CharNode(CharVal);
	next_token();
	return result;	
}
////////////////////////////////////////////////////////////////////////////////////////
static BoolNode* ParseBool()
{
	BoolNode* result = new BoolNode(IdentifierStr);
	next_token();
	return result;	
}
////////////////////////////////////////////////////////////////////////////////////////
static DoubleNode* ParseDouble()
{
	DoubleNode* result = new DoubleNode(DoubleVal);
	next_token();
	return result;	
}
////////////////////////////////////////////////////////////////////////////////////////
static StringNode* ParseString()
{
	StringNode* result = new StringNode(IdentifierStr);
	next_token();
	return result;	
}
////////////////////////////////////////////////////////////////////////////////////////
static Node* ParseIdentifier()
{
	string name = IdentifierStr;
	next_token();
	
	if(cur_token == '=')
	{

		next_token();
		Node* E = ParseExpression();
		if(!E) LogError("missing value for assignment.");
		return new Assignment(name, E);
	}
	
	if(cur_token == '.')
	{
		next_token();
		if(cur_token == NEXT_T)
		{
			next_token();
 			return new InputNode(name);
		}
	}

	if(cur_token != '(') return new IdentifierNode(name);
	//otherwise it's a function call
	next_token();
	vector<Node*> Args;
	
	if(cur_token != ')')
	{
		while(true)
		{
			Node* Arg = ParseExpression();
			if(!Arg) LogError("expecting argument.");
			Args.push_back(Arg);
			if (cur_token == ')' ) break;
			if (cur_token != ',' ) LogError("expecting ')' at the end of arguments.");; 
			next_token();
		}
	}

	if(cur_token!=')') LogError("expecting ')' at the end of arguments.");
	next_token();
	return new FunctionCall(name, Args);
}
////////////////////////////////////////////////////////////////////////////////////////
static Node* ParseOperator(int prec, Node* lhs)
{
	while(true)
	{
		int token_prec = getTokenPrec();
		if(token_prec < prec) return lhs;
		//otherwise is a binary operator
		//store binary operator
		int binop = cur_token;
		//read and store right element
		next_token();
		Node* rhs = ParsePrimary();
		if(!rhs)  LogError("missing operand.");
		
		//check next token precedence
		int next_prec = getTokenPrec();
		if(token_prec < next_prec)
		{
			rhs = ParseOperator(token_prec+1, rhs);
			if(!rhs) LogError("missing operand.");
		}
		lhs = new BinOperator(char(binop), lhs, rhs);
	}
	return lhs;
}
////////////////////////////////////////////////////////////////////////////////////////
static Node* ParseExpression()
{
	Node* lhs = ParsePrimary();
	if(!lhs) LogError("missing operand.");
	return ParseOperator(0, lhs);
}
////////////////////////////////////////////////////////////////////////////////////////
static VariableDec* ParseVariableDeclaration(string type, string name)
{
	return new VariableDec(type, name, NULL);
}
////////////////////////////////////////////////////////////////////////////////////////
static VariableDec* ParseAssignmentDeclaration(string type, string name)
{
	next_token();
	Node* E = ParseExpression();
	if(!E) LogError("missing operand.");
	return new VariableDec(type, name, E);
}
////////////////////////////////////////////////////////////////////////////////////////
static FunctionDec* ParseFunctionDeclaration(string type, string name)
{
	vector<DeclarationNode*> Args;
	vector<Node*> Stmts;
	next_token();
	string vartype;
	string varname;
	if(cur_token != ')')
	{
		while(true)
		{	
			vartype=IdentifierStr;
			next_token();
			varname=IdentifierStr;
			DeclarationNode* Arg = ParseVariableDeclaration(vartype, varname);
			if(!Arg) LogError("wrong argument.");
			Args.push_back(Arg);
			next_token(); 
			if(cur_token == ')') break;
			if(cur_token != ',') LogError("expecting ')' at the end of arguments.");
			next_token();
		}
	}
	next_token();
	
	if(cur_token == '{')
	{
		next_token();
		if(cur_token != '}')
		{
			while(true)
			{
				if(cur_token==TYPE_T)
				{
					string type = IdentifierStr;
					next_token();
					string name = IdentifierStr;
					next_token();
					DeclarationNode* Stmt;
					
					if(cur_token=='=')
					{
						Stmt = ParseAssignmentDeclaration(type, name);
					}else{
						Stmt = ParseVariableDeclaration(type, name);
					}
					if(!Stmt) LogError("unrecognised statement.");
					if(cur_token!=';') LogError("expecting ';' at the end of line."); 
					else next_token();
					Stmts.push_back(Stmt);

				}else if(cur_token==SCANNER_T){
					ParseScannerStmt();
					if(cur_token!=';') LogError("expecting ';' at the end of line."); 
					else next_token();
				}else{
					Node* Stmt = ParseFunctionContent();
					if(!Stmt) LogError("unrecognised statement.");
					Stmts.push_back(Stmt);
				}
			
				if (cur_token == '}') break;				
			}
		}
	}
	if(cur_token!='}') LogError("expecting '}' at the end of function body.");
	return new FunctionDec(type, name, Args, Stmts);
}
////////////////////////////////////////////////////////////////////////////////////////
static Node* ParsePrimary()
{
	switch(cur_token)
	{
		case INT_T		: return ParseInteger();
		case IDENT_T	: return ParseIdentifier();
		case DOUBLE_T	: return ParseDouble();
		case CHAR_T		: return ParseChar();
		case BOOL_T		: return ParseBool();
		case STR_T		: return ParseString();
		
		default			: LogError("unrecognised type.");
	}
}
////////////////////////////////////////////////////////////////////////////////////////
//gestisce nodi contenuti all'interno di un metodo
//possono essere assegnazione di variabili, chiamata di metodi, if stmt, while stmt, return stmt, print
//(le dichiarazioni sono già previste nel parsing della dichiarazione della funzione)
static Node* ParseFunctionContent()
{
	switch(cur_token)
	{
		case IDENT_T	: { Node* t = ParseIdentifier(); 
							if(cur_token!=';') LogError("expecting ';' at the end of line."); 
							else { next_token(); return t; }
						}
		case RETURN_T	: { Node* t = ParseReturn(); 
							if(cur_token!=';') LogError("expecting ';' at the end of line."); 
							else { next_token();return t; }
						}
		case PRINT_T	: { Node* t = ParsePrintStmt(); 
							if(cur_token!=';') LogError("expecting ';' at the end of line.");
							else { next_token(); return t; }
						}
		case IF_T		: return ParseIfStmt();
		case WHILE_T	: return ParseWhileStmt();
		
		default			: LogError("wrong statement.");
	}
}
////////////////////////////////////////////////////////////////////////////////////////
//gestisce i possibili parametri passati alla funzione di output System.out.println(...)
static Node* ParsePrintStmt()
{
	next_token();
	next_token();
	Node* all_v = nullptr;
	StringNode* string_v = nullptr;
	IdentifierNode* id_v = nullptr;
	
	switch(cur_token)
	{
		case STR_T		:  	string_v = ParseString(); break;
		case INT_T 		:  	all_v = ParseInteger(); break;
		case DOUBLE_T	:  	all_v = ParseDouble(); break;
		case CHAR_T		: 	all_v = ParseChar(); break;
		case IDENT_T	: 	id_v = ParsePrintContent(); break;
		
		default			:	LogError("wrong statement.");
	}
	
	next_token();

	return new PrintStmt(all_v, string_v, id_v);
}
////////////////////////////////////////////////////////////////////////////////////////
static IdentifierNode* ParsePrintContent()
{
	string name = IdentifierStr;
	next_token();
	return new IdentifierNode(name);
}
////////////////////////////////////////////////////////////////////////////////////////			
static Node* ParseReturn()
{
	next_token();
	Node* e = ParseExpression();
	if(!e) LogError("wrong return statement.");
	return new ReturnNode(e);
}
////////////////////////////////////////////////////////////////////////////////////////
//parsing della dichiarazione di una classe e del suo contenuto
//al suo interno sono previste dichiarazioni di variabili (senza assegnazione di un valore di default) e definizioni di funzioni
static Node* ParseClass()
{
	next_token();
	string name = IdentifierStr;
	next_token();
	if(cur_token!='{') LogError("expecting '{' at the beginning of class body.");
	vector<DeclarationNode*> content;
	vector<DeclarationNode*> variables;
	next_token();
	if(cur_token != '}')
		{
			while(true)
			{
				if(cur_token==TYPE_T){
					string type = IdentifierStr;
					if(type == "") LogError("no defined type");
					next_token();
					string name = IdentifierStr;
					if(name == "") LogError("no defined name");
					next_token();
					
					if(cur_token == '('){
						content.push_back(ParseFunctionDeclaration(type, name));
					}else{
						variables.push_back(ParseVariableDeclaration(type, name));
						if(cur_token!=';') LogError("expecting ';' at the end of line.");
					}
					next_token();
					if(cur_token=='}') break;		
					
				}else{
					LogError("unexpected statement in class body.");
				}
			}
		}
	if(cur_token!='}') LogError("expecting '}' at the end of class body.");
	return new ClassNode(name, variables, content);
}
////////////////////////////////////////////////////////////////////////////////////////
static Node* ParseIfStmt()
{
	next_token();
	if(cur_token!='(') LogError("expecting '(' at the beginning of condition.");
	next_token();
	Node* cond = ParseExpression();
	if(!cond) LogError("wrong condition.");
	if(cur_token!=')') LogError("expecting ')' at the end of condition.");
	next_token();
	if(cur_token!='{') LogError("expecting '{' at the beginning of true block.");
	next_token();
	vector<Node*> if_exec; //Blocco di comandi nel caso la condizione sia vera
	vector<Node*> else_exec; //Blocco di comandi nel caso la condizione sia falsa
	
	if(cur_token != '}')
	{
		while(true)
		{
			if(cur_token==TYPE_T)
			{
				string type = IdentifierStr;
				next_token();
				string name = IdentifierStr;
				next_token();
				DeclarationNode* Stmt;
				
				if(cur_token=='=')
				{
					Stmt = ParseAssignmentDeclaration(type, name);
				}else{
					Stmt = ParseVariableDeclaration(type, name);
				}
				if(!Stmt) LogError("unrecognised statement.");
				if(cur_token!=';') LogError("expecting ';' at the end of line."); 
				else next_token();
				if_exec.push_back(Stmt);
			}else if(cur_token==SCANNER_T){
				ParseScannerStmt();
				if(cur_token!=';') LogError("expecting ';' at the end of line."); 
				else next_token();
			}else{
				Node* Stmt = ParseFunctionContent();
				if(!Stmt) LogError("unrecognised statement.");
				if_exec.push_back(Stmt);
			}
			if (cur_token == '}') break;				
		}
	}
	if(cur_token!='}') LogError("expecting '}' at the end of true block.");

	next_token();

	if(cur_token == ELSE_T)
	{
		next_token();
		if(cur_token!='{')  LogError("expecting '{' at the beginning of false block.");
		next_token();

		if(cur_token != '}')
		{
			while(true)
			{
				if(cur_token==TYPE_T)
				{
					string type = IdentifierStr;
					next_token();
					string name = IdentifierStr;
					next_token();
					DeclarationNode* Stmt;

					if(cur_token=='=')
					{
						Stmt = ParseAssignmentDeclaration(type, name);
					}else{
						Stmt = ParseVariableDeclaration(type, name);
					}
					if(!Stmt) LogError("unrecognised statement.");
					if(cur_token!=';') LogError("expecting ';' at the end of line."); 
					else next_token();
					else_exec.push_back(Stmt);

				}else if(cur_token==SCANNER_T){
					ParseScannerStmt();
					if(cur_token!=';') LogError("expecting ';' at the end of line."); 
					else next_token();
				}else{
					Node* Stmt = ParseFunctionContent();
					if(!Stmt) LogError("unrecognised statement.");
					else_exec.push_back(Stmt);
				}
				if (cur_token == '}') break;		
			}
		}

		if(cur_token!='}') LogError("expecting '}' at the end of false block.");
		next_token();
	}
	return new IfStmt(cond, if_exec, else_exec);
}
////////////////////////////////////////////////////////////////////////////////////////
static Node* ParseWhileStmt()
{
	next_token();
	if(cur_token!='(') LogError("expecting '(' at the beginning of condition.");
	next_token();
	Node* cond = ParseExpression();
	if(!cond) LogError("wrong condition.");
	if(cur_token!=')') LogError("expecting ')' at the end of condition.");
	next_token();
	if(cur_token!='{') LogError("expecting '{' at the beginning of true block.");
	next_token();
	vector<Node*> while_exec; //Blocco di comandi nel caso la condizione sia vera
	
	if(cur_token != '}')
		{
			while(true)
			{
				if(cur_token==TYPE_T)
				{
					string type = IdentifierStr;
					next_token();
					string name = IdentifierStr;
					next_token();
					DeclarationNode* Stmt;
					
					if(cur_token=='=')
					{
						Stmt = ParseAssignmentDeclaration(type, name);
					}else{
						Stmt = ParseVariableDeclaration(type, name);
					}
					if(!Stmt) LogError("unrecognised statement.");
					if(cur_token!=';') LogError("expecting ';' at the end of line."); 
					else next_token();
					while_exec.push_back(Stmt);

				}else if(cur_token==SCANNER_T){
					ParseScannerStmt();
					if(cur_token!=';') LogError("expecting ';' at the end of line.");  
					else next_token();
				}else{
					Node* Stmt = ParseFunctionContent();
					if(!Stmt) LogError("unrecognised statement.");
					while_exec.push_back(Stmt);
				}
				if (cur_token == '}') break;				
			}
		}
		if(cur_token!='}') LogError("expecting '}' at the end of true block.");
		next_token();
		return new WhileStmt(cond, while_exec);
}
////////////////////////////////////////////////////////////////////////////////////////
static void ParseScannerStmt()
{
	next_token();
	string id = IdentifierStr;
	next_token();
	if(cur_token!='=') LogError("expected '=' in Scanner declaration.");
	next_token();
	if(cur_token!=NEW_T) LogError("expected 'new' in Scanner declaration.");
	next_token();
	if(cur_token!=SCANNER_T) LogError("expected 'Scanner' in Scanner declaration.");
	next_token();
	if(cur_token!='(') LogError("expected '(' in Scanner declaration.");
	next_token();
	if(cur_token!=SYSTEMIN_T) LogError("expected 'System.in' in Scanner declaration."); 
	next_token();
	if(cur_token!=')') LogError("expected ')' in Scanner declaration."); 
	next_token();
	scanner_variables.push_back(id);
}
////////////////////////////////////////////////////////////////////////////////////////