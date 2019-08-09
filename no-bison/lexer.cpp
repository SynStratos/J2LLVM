/////////////////////////////////////////////////////////
//------------------------LEXER------------------------//
/////////////////////////////////////////////////////////

enum Token_Type {
	EOF_T = 0,
	INT_T,
	CLASS_T,
	RETURN_T,
	DOUBLE_T,
	IDENT_T,
	IF_T,
	ELSE_T,
	WHILE_T,
	CHAR_T,
	BOOL_T,
	STR_T,
	PRINT_T,
	NEXT_T,
	SCANNER_T,
	SYSTEMIN_T,
	NEW_T,
	TYPE_T,
	AND_T,
	OR_T,
	EQUAL_T,
	NOTEQUAL_T,
	LE_T,
	GE_T
};

static string IdentifierStr;	//se token identifier
static int IntVal;				//se token intero
static double DoubleVal;		//se token double precision
static int CharVal;				//se token char
static int num_lines = 1;
static vector<int> right_token {1};

static int getToken()
{
	static int LastChar = ' ';
	
	while(isspace(LastChar))
	{
		if(LastChar == '\n') {num_lines++;} //Conto il numero di righe ogni volta che vado a capo
		LastChar=getchar();
	}

	//lettura del carattere ['] per definire un char
	if(LastChar == char(39)){
		LastChar=getchar();
	   CharVal=LastChar;
		LastChar=getchar();
		LastChar=getchar();

		right_token.push_back(num_lines);
	   return CHAR_T;
	}
	
	if(LastChar == '='){
		LastChar = getchar();
		if(LastChar == '='){
			LastChar = getchar();
			right_token.push_back(num_lines);
			return EQUAL_T;
		}else{
			right_token.push_back(num_lines);
			return '=';
		}
	}
	
	if(LastChar == '!'){
		LastChar = getchar();
		if(LastChar == '='){
			LastChar = getchar();
			right_token.push_back(num_lines);
			return NOTEQUAL_T;
		}else{
			right_token.push_back(num_lines);
			return '!';
		}
	}

	if(LastChar == '<'){
		LastChar = getchar();
		if(LastChar == '='){
			LastChar = getchar();
			right_token.push_back(num_lines);
			return LE_T;
		}else{
			right_token.push_back(num_lines);
			return '<';
		}
	}

	if(LastChar == '>'){
		LastChar = getchar();
		if(LastChar == '='){
			LastChar = getchar();
			right_token.push_back(num_lines);
			return GE_T;
		}else{
			right_token.push_back(num_lines);
			return '>';
		}
	}

	if(LastChar == '|'){
		LastChar = getchar();
		if(LastChar == '|'){
			LastChar = getchar();
			right_token.push_back(num_lines);
			return OR_T;
		}else{
			right_token.push_back(num_lines);
			return '|';
		}
	}

	if(LastChar == '&'){
		LastChar = getchar();
		if(LastChar == '&'){
			LastChar = getchar();
			right_token.push_back(num_lines);
			return AND_T;
		}else{
			right_token.push_back(num_lines);
			return '&';
		}
	}

	if(LastChar == '"'){
		LastChar=getchar();
		IdentifierStr = "";
		if(LastChar!='"')
		{
			IdentifierStr = LastChar;
			while((LastChar=getchar())!='"')	IdentifierStr += LastChar;
		}
		LastChar=getchar();
		right_token.push_back(num_lines);
	   return STR_T;
	}
	
	if(isalpha(LastChar))
	{
		IdentifierStr = LastChar;
		while(isalnum(LastChar=getchar()))	IdentifierStr += LastChar;
	
		if(IdentifierStr == "class") {right_token.push_back(num_lines); return CLASS_T;}
		if(IdentifierStr == "if")	{right_token.push_back(num_lines); return IF_T;}
		if(IdentifierStr == "else") {right_token.push_back(num_lines); return ELSE_T;}
		if(IdentifierStr == "while") {right_token.push_back(num_lines); return WHILE_T;}
		if(IdentifierStr == "return") {right_token.push_back(num_lines); return RETURN_T;}
		if(IdentifierStr == "Scanner") {right_token.push_back(num_lines); return SCANNER_T;}
		if(IdentifierStr == "new") {right_token.push_back(num_lines); return NEW_T;}
		if(IdentifierStr == "next")
		{ 
			IdentifierStr += LastChar;
			LastChar=getchar();
			IdentifierStr += LastChar;
			if(IdentifierStr=="next()"){

				LastChar = ' ';
				right_token.push_back(num_lines); 
				return NEXT_T;
			}
		} 
		if(IdentifierStr == "System")
		{
			IdentifierStr += LastChar; //il while ha letto anche il punto ma non l'ha salvato perchè non "alnum"
			//ce l'ho comunque in memoria ancora su lastchar
			LastChar=getchar();
			if(LastChar=='i'){
				IdentifierStr += LastChar;
				LastChar=getchar();
				if(LastChar=='n') IdentifierStr += LastChar;
				LastChar = ' ';
				right_token.push_back(num_lines); 
				return SYSTEMIN_T;
			}

			while(isalpha(LastChar) || LastChar == '.'){
				IdentifierStr += LastChar;
				LastChar = getchar();

				if(LastChar == '('){
					if(IdentifierStr == "System.out.println")  break;
					else return 0;
				}
			};
			right_token.push_back(num_lines); 
			return PRINT_T;
		}
		if(IdentifierStr == "true"
			|| IdentifierStr == "false") {right_token.push_back(num_lines); return BOOL_T;}
		if(IdentifierStr == "void" 
			|| IdentifierStr == "int" 
			|| IdentifierStr == "char" 
			|| IdentifierStr == "double"
			|| IdentifierStr == "string"
			|| IdentifierStr == "boolean") {right_token.push_back(num_lines); return TYPE_T;}
		
		if(IdentifierStr == "define"
			|| IdentifierStr == "abstract"
			|| IdentifierStr == "assert"
			|| IdentifierStr == "continue"
			|| IdentifierStr == "static"
			|| IdentifierStr == "extern"
			|| IdentifierStr == "break"
			|| IdentifierStr == "byte"
			|| IdentifierStr == "case"
			|| IdentifierStr == "catch"
			|| IdentifierStr == "do"
			|| IdentifierStr == "for"
			|| IdentifierStr == "enum"
			|| IdentifierStr == "extends"
			|| IdentifierStr == "final"
			|| IdentifierStr == "finally"
			|| IdentifierStr == "float"
			|| IdentifierStr == "import"
			|| IdentifierStr == "implements"
			|| IdentifierStr == "interface"
			|| IdentifierStr == "long"
			|| IdentifierStr == "package"
			|| IdentifierStr == "protected"
			|| IdentifierStr == "public"
			|| IdentifierStr == "short"
			|| IdentifierStr == "super"
			|| IdentifierStr == "strictfp"
			|| IdentifierStr == "switch"
			|| IdentifierStr == "synchronized"
			|| IdentifierStr == "this"
			|| IdentifierStr == "throw"
			|| IdentifierStr == "throws"
			|| IdentifierStr == "transient"
			|| IdentifierStr == "try"
			|| IdentifierStr == "volatile"
			|| IdentifierStr == "goto"
			|| IdentifierStr == "const") { printf("line %d - error: use of reserved word '%s' as identifier \n", num_lines, IdentifierStr.c_str()); exit(0); }

		right_token.push_back(num_lines); 
		return IDENT_T;
	}
	
	if(isdigit(LastChar))
	{
		string NumStr;
		
		do
		{
			NumStr += LastChar;
			LastChar = getchar();
		}while(isdigit(LastChar));
		
		if(LastChar == '.')
		{
			do{
				NumStr += LastChar;
				LastChar = getchar();
			}while(isdigit(LastChar));
			DoubleVal = atof(NumStr.c_str());
			right_token.push_back(num_lines); 
			return DOUBLE_T;
		}else{
			IntVal = stoi(NumStr.c_str(), nullptr);
			right_token.push_back(num_lines); 
			return INT_T;
		}
	}
	
	if(LastChar == '#') {
		do LastChar = getchar();
		while(LastChar != EOF && LastChar != '\n' && LastChar != 'r');
	}

	// check su end of file
	if(LastChar == EOF) {right_token.push_back(num_lines); return EOF_T;}


	// ritorna il carattere con il suo valore di ascii
	// in caso di token non specificato
	int ThisChar = LastChar;
	LastChar = getchar();
	//right_token.push_back(num_lines); 
	return ThisChar;
}