/////////////////////////////////////////////////////////
//-------------------------AST-------------------------//
/////////////////////////////////////////////////////////
namespace{
	//oggetto base
	class Node
	{
	public:
		virtual ~Node() = default;
		virtual Value* codegen() = 0;
	};
	//oggetto dichiarazione base
	class DeclarationNode : public Node
	{
	public:
		string type;
		string name;
	};
	class ClassNode : public Node
	{
		string name;
		vector<DeclarationNode*> variables;
		vector<DeclarationNode*> content;
	public:
		ClassNode(string name, vector<DeclarationNode*> variables, vector<DeclarationNode*> content):name(name),variables(variables),content(content) {}
		virtual Value* codegen();
	};
	//oggetto statement di tipo return
	class ReturnNode : public Node
	{
	public:
		Node* value;

		ReturnNode(Node* value):value(value){}
		virtual Value* codegen();
	};
	//oggetto per la dichiarazione di una variabile, eredita nome e tipo
	class VariableDec : public DeclarationNode
	{
	public:
		Node* value; 

		VariableDec(string type, string name, Node* value){
			this->type=type;
			this->name=name;
			this->value=value;
			
		}
		virtual Value* codegen();
	};
	//oggetto per la dichiarazione di una funzione, eredita nome e tipo
	class FunctionDec : public DeclarationNode
	{
	public:
		vector<DeclarationNode*> args;
		vector<Node*> body;
		
		FunctionDec(string type, string name, vector<DeclarationNode*> args, vector<Node*> body){
			this->type=type;
			this->name=name;
			this->args=args;
			this->body=body;		
		}
		virtual Function* codegen();
	};
	//oggetto per la chiamata di una funzione già definita
	class FunctionCall : public Node
	{
	public:
		string name;
		vector<Node*> args;

		FunctionCall(string name, vector<Node*> args):name(name),args(args) {}
		virtual Value* codegen();
	};
	//oggetto per uno statement di tipo IF
	class IfStmt : public Node
	{
	public:
		Node* cond;
		vector<Node*> if_exec; //Blocco di comandi nel caso la condizione è vera
		vector<Node*> else_exec; //Blocco di comandi nel caso la condizione è falsa

		IfStmt(Node* cond, vector<Node*> if_exec, vector<Node*> else_exec):cond(cond),if_exec(if_exec), else_exec(else_exec){}
		virtual Value* codegen();
	};
	//oggetto per uno statement di tipo FOR
	class WhileStmt : public Node
	{
	public:
		Node* cond;
		vector<Node*> while_exec; //Blocco di comandi nel caso la condizione è verificata
		
		WhileStmt(Node* cond, vector<Node*> while_exec):cond(cond),while_exec(while_exec){}
		virtual Value* codegen();
	};
	//oggetto per l'assegnazione di un valore/espressione ad una variabile
	class Assignment : public Node
	{
	public:
		string name;
		Node* value;

		Assignment(string name, Node* value):name(name),value(value) {}
		virtual Value* codegen();
	};
	//oggetto per un operatore binario
	class BinOperator : public Node
	{
	public:
		char op;
		Node*	lhs;
		Node*	rhs;

		BinOperator(char op, Node* lhs, Node* rhs):op(op),lhs(lhs),rhs(rhs) {}
		virtual Value* codegen();
	};
	//oggetti per primitive di tipo int identifier char double boolean string
	class IntegerNode : public Node
	{
	public:
		int value;

		IntegerNode(int value):value(value){}
		virtual Value* codegen();
	};
	class IdentifierNode : public Node
	{
	public:
		string name;

		IdentifierNode(string name):name(name) {}
		virtual Value* codegen();
	};
	class CharNode : public Node
	{
	public:
		int value;
		
		CharNode(int value):value(value){}
		virtual Value* codegen();
	};
	class DoubleNode : public Node
	{
	public:
		double value;
		
		DoubleNode(double value):value(value){}
		virtual Value* codegen();
	};
	class BoolNode : public Node
	{
	public:
		string value;
		
		BoolNode(string value):value(value){}
		virtual Value* codegen();
	};
	class StringNode : public Node
	{
	public:
		string value;
		
		StringNode(string value):value(value){}
		virtual Value* codegen();
	};
	// print ed input
	class PrintStmt : public Node
	{
	public:
		Node* all_v;
		StringNode* string_v;
		IdentifierNode* id_v;

		PrintStmt(Node* all_v, StringNode* string_v,IdentifierNode* id_v):all_v(all_v),string_v(string_v),id_v(id_v){}
		virtual Value* codegen();
	};
	class InputNode : public Node
	{
	public:
		string identifier;

		InputNode(string identifier):identifier(identifier){}
		virtual Value* codegen();
	};
}