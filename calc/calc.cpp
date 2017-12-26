#include "../../std_lib_facilities.h"

//------------------------------------------------------------------------------

const char number = '8'; // t.kind==number means t is a number
const char quit = 'x'; // t.kind==number means t is a number
const char print = ';'; // t.kind==number means t is a number
const char name = 'a'; // t.kind==number means t is a number
const char let = 'L'; // t.kind==number means t is a number
const string declkey = "let"; // t.kind==number means t is a number

//------------------------------------------------------------------------------

class Token
{
public:
	char kind;        // what kind of token
	double value;     // for numbers: a value 
	string name;

	Token(char ch) :kind(ch)/*, value(0)*/ { }  // make a Token from a char

	Token(char ch, double val) :kind(ch), value(val) { }    // make a Token from a char and a double

	Token(char ch, string n) :kind(ch), name(n) { }

};

//------------------------------------------------------------------------------
class Variable
{
public:
	string name;
	double value;

	Variable(string var, double val) : name(var), value(val) { }
};
//------------------------------------------------------------------------------


class Token_stream
{
public:
	Token_stream();   // make a Token_stream that reads from cin
	Token get();      // get a Token (get() is defined elsewhere)
	void putback(Token t);    // put a Token back
	void ignore(char с);      // get rid of symbols till "c"(including it)
private:
	bool full;        // is there a Token in the buffer?
	Token buffer;     // here is where we keep a Token put back using putback()
};

//------------------------------------------------------------------------------

// The constructor just sets full to indicate that the buffer is empty:
Token_stream::Token_stream()
	: full(false), buffer(0)    // no Token in buffer
{
}
//------------------------------------------------------------------------------

// The putback() member function puts its argument back into the Token_stream's buffer:
void Token_stream::putback(Token t)
{
	if ( full ) error("putback() into a full buffer");
	buffer = t;       // copy t to buffer
	full = true;      // buffer is now full
}
void Token_stream::ignore(char c)
{
	if ( full && c == buffer.kind )
	{
		full = false;
		return;
	}
	full = false;

	char ch = 0;
	while ( cin >> ch )
		if ( ch == c ) return;
}
//------------------------------------------------------------------------------

Token Token_stream::get()
{
	if ( full )
	{       // do we already have a Token ready?
			// remove token from buffer
		full = false;
		return buffer;
	}

	char ch;
	cin >> ch;    // ">>" skips whitespace (space, newline, tab, etc.)

	switch ( ch )
	{
	case print:    // for "print"
	case quit:    // for "quit"
	case let:		//for defining variable
	case '!':    // for "factorial"
	case '^':    // for "power"
	case '(':
	case ')':
	case '{':
	case '}':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
		return Token(ch);       // let each character represent itself
	case '.':					// number with floating point can start with "."
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':

	{
		cin.putback(ch);         // put digit back into the input stream
		double val;

		cin >> val;              // read a floating-point number
		return Token(number, val);   // let '8' represent "a number"
	}
	default:
		if ( isalpha(ch) ) {
			cin.putback(ch);
			string s;
			cin >> s;
			if ( s == declkey )
				return Token(let); // "let" keyword
			return Token{ name,s };
		}
		error("Bad token");
	}
}
//------------------------------------------------------------------------------


vector<Variable> var_table;

//------------------------------------------------------------------------------

double get_value(string s)
{
	for ( const Variable& v : var_table )
		if ( v.name == s ) return v.value;
	error("get: undefined variable ", s);
}
//------------------------------------------------------------------------------

void set_value(string s, double d)
{
	for ( Variable& v : var_table )
		if ( v.name == s )
		{
			v.value = d;
			return;
		}
	error("set: undefined variable ", s);
}
//------------------------------------------------------------------------------

//if variable var is already in var_table
bool is_declared(string var)
{
	for ( const Variable& v : var_table )
		if ( v.name == var ) return true;
	return false;
}
//------------------------------------------------------------------------------

//add (var, val) pair to var_table
double define_name(string var, double val)
{
	if ( is_declared(var) ) error(var, "already declared");
	var_table.push_back(Variable(var, val));
	return val;
}
//------------------------------------------------------------------------------

int factorial(int  num_int)
{
	if ( num_int == 0 ) return num_int = 1;
	else if ( num_int >= 2 ) return num_int = num_int * factorial(num_int - 1);
	else
		return num_int;
}
//------------------------------------------------------------------------------

Token_stream ts;        // provides get() and putback() 

//------------------------------------------------------------------------------

void clean_up_mess()
{
	ts.ignore(print);
}
//------------------------------------------------------------------------------

double expression();    // declaration so that primary() can call expression()

//------------------------------------------------------------------------------

// deal with numbers and parentheses
double primary()
{
	//cout << "Primary :" << endl;

	Token t = ts.get();
	switch ( t.kind )
	{

	case '(':    // handle '(' expression ')'
	{
		double d = expression();
		t = ts.get();
		if ( t.kind != ')' ) error("')' expected");
		return d;
	}
	case '{':    // handle '{' expression '}'
	{
		double d = expression();
		t = ts.get();
		if ( t.kind != '}' ) error("'}' expected");
		return d;
	}
	case number:         // we use 'number' to represent a number
		return t.value;  // return the number's value
	case '-':
		return -primary();
	case '+':
		return primary();
	default:
		error("primary expected");
	}
}

// deal with ^, !
double postfix()
{
	//cout << "Postfix :" << endl;

	double left = primary();
	Token t = ts.get();        // get the next token from token stream

	while ( true )
	{
		switch ( t.kind )
		{
		case '!':
			left = factorial(left);
			t = ts.get();
			break;
		case '^':
		{
			left = pow(left, primary());
			t = ts.get();
			break;
		}
		default:
			ts.putback(t);     // put t back into the token stream
			return left;
		}
	}
}

// deal with *, /, and %
double term()
{
	//cout << "Term :" << endl;

	double left = postfix();
	Token t = ts.get();        // get the next token from token stream

	while ( true )
	{
		switch ( t.kind )
		{
		case '*':
			left *= postfix();
			t = ts.get();
			break;
		case '/':
		{
			double d = postfix();
			if ( d == 0 ) error("divide by zero");
			left /= d;
			t = ts.get();
			break;
		}
		case '%':
		{
			int d1 = narrow_cast<int>(left);
			int d2 = narrow_cast<int>(postfix());
			if ( d2 == 0 ) error("%: divide by zero");
			left = d1%d2;
			t = ts.get();
			break;
		}
		default:
			ts.putback(t);     // put t back into the token stream
			return left;
		}
	}
}
//------------------------------------------------------------------------------

// deal with + and -
double expression()
{
	//cout << "Expression :" << endl;

	double left = term();      // read and evaluate a Term
	Token t = ts.get();        // get the next token from token stream

	while ( true )
	{
		switch ( t.kind )
		{
		case '+':
			left += term();    // evaluate Term and add
			t = ts.get();
			break;
		case '-':
			left -= term();    // evaluate Term and subtract
			t = ts.get();
			break;
		default:
			ts.putback(t);     // put t back into the token stream
			return left;       // finally: no more + or -: return the answer
		}
	}
}
//------------------------------------------------------------------------------
//perhaps we've met "let"
//Name = Value
//declaring variable with Name with start value from expression
double declaration()
{
	Token t = ts.get();
	if ( t.kind != name )
		error("variable name expected");
	string var_name = t.name;

	Token t2 = ts.get();
	if ( t.kind != '=' )
		error("missing '=' in declaration of ", var_name);

	double d = expression();
	define_name(var_name, d);
	return d;
}
//------------------------------------------------------------------------------

double statement()
{
	Token t = ts.get();
	switch ( t.kind ) {
	case let:
		return declaration();
	default:
		ts.putback(t);
		return expression();
	}

}

//------------------------------------------------------------------------------

// Calcutating cycle:
void calculate()
{
	while ( cin )
		try
	{
		cout << "> ";
		Token t = ts.get();

		if ( t.kind == print ) t = ts.get();  // not ';' - changed to '=' for "print now"
		if ( t.kind == quit ) return;		// not 'q' - changed to 'x' for quit

		ts.putback(t);
		cout << "= " << statement() << endl;
	}
	catch ( exception& e )
	{
		cerr << e.what() << endl;
		clean_up_mess();
	}
}

//------------------------------------------------------------------------------

int main()
try
{
	cout << "Wellcome to app-calculator! Enter expressions with double numbers." << endl;

	calculate();

	keep_window_open();
	return 0;
}
catch ( exception& e )
{
	cerr << "error: " << e.what() << '\n';
	keep_window_open("~~");
	//cout << "To close the window enter ~\n";
	//char ch;
	//for ( char ch; cin >> ch;)  // Чтение до ввода символа ~
	//	if ( ch == '~' ) return 1;
	return 1;
}
catch ( ... )
{
	cerr << "Oops: unknown exception!\n";
	keep_window_open("~~");
	return 2;
}

//------------------------------------------------------------------------------