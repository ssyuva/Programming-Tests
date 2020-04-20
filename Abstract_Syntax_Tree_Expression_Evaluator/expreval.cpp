// Abstract Syntax Tree based expression evaluator
// Author: Yuvaraja Subramaniam (www.linkedin.com/in/yuvaraja)

// Step 1: Build an expression tree for an expression
// Step 2: Evaluate the expression

#include <iostream>
#include <stack>
#include <vector>
#include <string>
#include <cctype>
#include <unistd.h>
#include <algorithm>


using namespace std;


//Globals
bool debug = false;
enum NodeType { OPERATOR, OPERAND, BRACKET};
enum Precedence {NUM = 0, ADD_SUB = 1, MUL_DIV = 2};


//AST Node
struct ASTNode {
	char nodeChar;
	enum NodeType node_type;
	ASTNode *left;
	ASTNode *right;
};


//Function Prototypes
Precedence getPrecedence(char oprator);

bool buildAST(const string inexpr, ASTNode ** astTree);  //STEP 1

double applyOperator(char op, double left_value, double right_value);

void printAST(ASTNode *astTree, int level = 0);

void freeAST(ASTNode *astTree);

bool evalAST(ASTNode *astTree, double & evalValue);    //STEP 2

bool processExpression(string inexpr, double & result);

bool runTests();

void usage(int argc, char* argv[]);


//Main Program
int main(int argc, char* argv[]) {

	int c;
	bool test  = false;
	bool help  = false;

	while ( (c = getopt(argc, argv, "tdh")) != -1) {
		switch(c)
		{
			case 't' : 
				test = true;
				break;
			case 'd' : 
				debug = true;
				break;
			case 'h' :
				help = true;
				break;
		}
	}

	//if help requested, display and exit
	if (help) {
		usage(argc, argv);
		exit(0);
	}

	//Run tests if tests flag on
	if (test) {
		runTests();
		cout << endl;
	}

	cout << "======================== EVALUATE USER ENTERED EXPRESSION  ========================" << endl << endl;
	string expr;
	cout <<"Enter mathematical expression to evaluate : ";
	getline(cin, expr);
	cout <<"Entered expression                        : " << expr << endl;
	double result;
	if( true == processExpression(expr, result) ) {
		cout << "Evaluated expression value               : " << result << endl;
	}
	else {
		cout << "Invalid expression.. please check your input" << endl;
	}
}


//Process Expression
bool processExpression(string inexpr, double & result) {

	ASTNode *astTree;

	//STEP 1: build the ast tree first
	if (true != buildAST(inexpr, &astTree)) {
		cout << "Error: AST tree construction failed for expression : " << inexpr << endl;
		return false;
	}
		cout << "Successful AST tree construction for expression    : " << inexpr << endl;

	//Print the AST tree
	cout << "AST TREE : " << endl;
	printAST(astTree);

	//STEP 2: evaluate the ast tree and get the result
	bool success = evalAST(astTree, result);

	//Free the ast tree
	freeAST(astTree);

	return success;
}


//STEP 1: Build Abstract Syntax Tree (AST)
bool buildAST(const string inexpr, ASTNode ** astTree) {

	string ex = inexpr;

	//remove the whitespaces
	ex.erase(remove_if(ex.begin(), ex.end(), ::isspace), ex.end());

	if (debug) { cout << "buildAST processing expr : " << ex << endl ;}

	if ( 0 == ex.length() )
		return false;

	//operand stack
	stack <ASTNode *> OperandStack;

	//Operator stack
	stack <ASTNode *> OperatorStack;

	bool prevchar_is_digit = false;

	for(int i=0; i < ex.length(); i++) {

	   if (debug) { cout << "Parser saw : " << ex[i] << endl; }

		if ( !isdigit(ex[i]) && !isspace(ex[i]) && ex[i] != '*' && ex[i] != '/' && ex[i] != '+' && ex[i] != '-' && ex[i] != ')' && ex[i] != '(' ) {
			cout << "Error: non-digit,non-operator, non-whitespace character seen in expression" << endl;
			return false;
		}

		if (ex[i] == '(') {
			ASTNode *curNode = new ASTNode;
			curNode->nodeChar = '(';
			curNode->left  = nullptr;
			curNode->right = nullptr;
			curNode->node_type = BRACKET;
			OperatorStack.push(curNode);
			prevchar_is_digit = false;

		} else if (isdigit(ex[i])) {

			if(true == prevchar_is_digit) {
				cout << "Error: multidigit number found in the expression. only single digit number values (0,1,2,..,8,9) allowed" << endl;
				return false;
			}

			if (debug) { cout << ex[i] << " is a digit" << endl; }

			ASTNode *curNode = new ASTNode;
			curNode->nodeChar = ex[i];
			curNode->left  = nullptr;
			curNode->right = nullptr;
			curNode->node_type = OPERAND;
			OperandStack.push(curNode);
			prevchar_is_digit = true;

		}  else if (ex[i] == ')') { 

			while (! OperatorStack.empty() && OperatorStack.top()->nodeChar != '(') {

				ASTNode *rightNode = OperandStack.top();
				OperandStack.pop();
				ASTNode *leftNode  = OperandStack.top();
				OperandStack.pop();

				ASTNode *opNode = OperatorStack.top();
				OperatorStack.pop();

				opNode->left  = leftNode;
				opNode->right = rightNode;

				OperandStack.push(opNode);
  			}
			prevchar_is_digit = false;

			if (! OperatorStack.empty() ) {
				OperatorStack.pop();	// pop out the opening brace
			}

		} else {
			//current token is an operator. adjust the AST for precedence
			prevchar_is_digit = false;

			while ( (!OperatorStack.empty()) && (getPrecedence(OperatorStack.top()->nodeChar) >= getPrecedence(ex[i])) ) {

				if (OperandStack.size() < 2)
					return false;

				ASTNode *rightNode = OperandStack.top();
				OperandStack.pop();

				ASTNode *leftNode  = OperandStack.top();
				OperandStack.pop();

				ASTNode *opNode = OperatorStack.top();
				OperatorStack.pop();

				opNode->left  = leftNode;
				opNode->right = rightNode;

				OperandStack.push(opNode);
			}

			//now push the current operator on operator stack
			ASTNode *curNode = new ASTNode;
			curNode->nodeChar = ex[i]; 
			curNode->left  = nullptr;
			curNode->right = nullptr;
			curNode->node_type = OPERATOR;
			OperatorStack.push(curNode);
		}
	}

	if(debug) {cout << "Parsed the input expression ... checking the operator stack for precedence operators" << endl;}

	//Apply the remaining operators on the operator stack to nodes in the operand stack and complete the ast tree

	while ( ! OperatorStack.empty() ) {

		if (OperandStack.size() < 2)
			return false;


		ASTNode *rightNode = OperandStack.top();
		OperandStack.pop();

		ASTNode *leftNode  = OperandStack.top();
		OperandStack.pop();

		ASTNode *opNode = OperatorStack.top();
		OperatorStack.pop();

		opNode->left  = leftNode;
		opNode->right = rightNode;

		OperandStack.push(opNode);
	}

   *astTree = OperandStack.top();
   OperandStack.pop();

	//if the stacks are not empty, the expression is malformed
	if (! OperandStack.empty()  || ! OperatorStack.empty() )
		return false;

	return true;
}


//STEP 2: Evaluate the AST Tree and get the result
bool evalAST(ASTNode *astTree, double & evalValue) {
	double result;

	if (debug) {cout << "evaluating AST .. " << endl;}

	if (nullptr == astTree) {
		if (debug) { cout << "returning zero... " << endl; }
		result = 0;
	}
	else if (astTree->node_type == OPERAND) {
		if (debug) { cout << "node char = " << astTree->nodeChar << endl; }
		result = astTree->nodeChar - '0';
	}
    else if (astTree->node_type == OPERATOR) {

		double left_value;
		double right_value; 
		evalAST(astTree->left, left_value);
		evalAST(astTree->right, right_value);
		char op = astTree->nodeChar;
		result = applyOperator(op, left_value, right_value);
	}
evalValue = result;
if (debug) { cout << "Returning evalValue : " << evalValue << endl; }
return true;
}



//Return precedence
Precedence getPrecedence(char oprator) {
	if (oprator == '*' || oprator == '/')
		return MUL_DIV;
	if (oprator == '+' || oprator == '-')
		return ADD_SUB;
	return NUM;
}


//applyOperator and return value
double applyOperator(char op, double left_value, double right_value) {

	switch(op) {
		case '*':
			return left_value * right_value;
		case '/':
			return left_value / right_value;
		case '+':
			return left_value + right_value;
		case '-':
			return left_value - right_value;
	}
}

//Print AST
void printAST(ASTNode *astTree, int level) {
	if (nullptr == astTree) return;
	printAST(astTree->left, level + 1);
	for ( int k = 0; k < level; k++)
		cout << "--> " ;
	cout << "--> " << astTree->nodeChar << endl;
	printAST(astTree->right, level + 1);
}

//Free AST
void freeAST(ASTNode *astTree) {

	if (astTree->left != nullptr) 
		freeAST(astTree->left);
	if (astTree->right != nullptr) 
		freeAST(astTree->right);
	
	delete astTree;
}

//Tests
bool runTests() {
	struct TEST {
		string expr;
		bool valid;
		double result;

		TEST(string st, bool v, double res) : expr(st), valid(v), result(res) {}
	};

	vector<TEST> TestSet = 	{ 	TEST(string("(4 + 5 * (7 - 3)) - 2"), true, 22),	
							 	TEST(string("4+5+7/2"), true, 12.5), 
								TEST(string("10+1"), false, 0), 
								TEST(string("-10"), false, 0),
								TEST(string(""), false,0),
								TEST(string("asdf"), false, 0),
								TEST(string("(2 * 4) + (8 * 5) - ( 6 / 3 )"), true, 46),
								TEST(string("((2 * 4) + (8 * 5) - ( 6 / 3 )) / 2"), true, 23)
							};

	cout << "=================================== EXECUTING TESTS ====================================" << endl;
	for (int i=0; i < TestSet.size() ; i++) {
			cout << "Test case     	  : " << i + 1 << endl;
			cout << "Expr          	  : " << TestSet[i].expr << endl;
			cout << "Valid Expr    	  : " << (TestSet[i].valid == true ? "yes" : "no") << endl;
			cout << "Expeced Value 	  : " << TestSet[i].result << endl;

		bool success = false;
		double res   = 0.0;

		success = processExpression(TestSet[i].expr, res);
		
		if (success == true && res == TestSet[i].result) {
		
			cout << "Calculated Value : " << res << endl;
			cout << "Test Result      : Pass" << endl;

		} else if (success == true && res != TestSet[i].result) {

			cout << "Calculated Value : " << res << endl;
			cout << "Test Result      : Fail" << endl;

		} else if (success == false && TestSet[i].valid == false) {
	
			cout << "Test Result      : Pass" << endl;

		} else if (success == false && TestSet[i].valid == false) {

			cout << "Test Result      : Pass" << endl;
		}
		cout << "--------------------------------------" << endl;
		cout << endl << endl;
	}
	cout << "=================================== TESTS DONE ====================================" << endl;

}

//Usage
void usage(int argc, char* argv[]) {
	cout << argv[0] << " -tdh" << endl;
	cout << "       t - run tests" << endl;
	cout << "       d - print debug" << endl;
	cout << "       h - help" << endl;
}

