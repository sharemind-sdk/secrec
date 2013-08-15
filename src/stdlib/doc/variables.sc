/**
@page variables Variables
@brief Variables in SecreC

@section variables Variables

Variables in SecreC consist of lowercase and uppercase Latin characters, underscores and decimal digits. The first character of a variable must not be a decimal digit. Reserved keywords are not allowed to be used as variable names.

@subsection declaring_defining Declaring and defining

Variables are declared by writing a type annotation followed by one or more variable names. Optionally, it is possible to assign a value right after the variable declaration by writing an expression after the assignment sign. All declared variables are assigned reasonable default values. For integers this value is 0, for booleans it is **false**.

Listing 6.3: Some variable declarations
\code
	kind additive3pp ;
	domain sharemind_test_pd additive3pp ;
	domain private additive3pp ;
	void main () {
		int x; // assigned default value
		int y = 5;
		private uint8 z;
		sharemind_test_pd bool secret = true;
		uint i, j = 2, k;
		return ;
	}
\endcode

As previously mentioned, types are formed by writing security domain before the data type, and **public** may be omitted. 
 Array declarations allow the shape (sizes of dimensions) to be specified after the variable name between parenthesis. Initial shape may be non-static and can be an arbitrary public integer expression.
The shape is specified with public signed integer type.

Listing 6.4: Array declarations
\code
	int [[1]] vector (100); // vector of 100 elements
	bool [[2]] mat (3, 4) = true; // constant true 3x4 matrix
	public int n;
	// some computation on n
	int [[3]] cube (2*n, 3*n, 4*n);
 \endcode

It is possible to define an empty array by not specifying the shape, or by having any of the dimensions have no elements. If an array definition is immediately followed by an assignment, and the shape is not specified then the shape is inherited from the right hand side expression.

Listing 6.5: (Non)empty arrays
\code
	int [[1]] empty ; // empty array
	int [[1]] over9000 (9001);
	int [[1]] notEmpty = over9000 ;
\endcode

@subsection Scope

Table 1: Operator precedence
| Level | Operator           | Description                 | Associativity |
| ----: | :----------------- | :-------------------------- | :------------ |
| 1     | `++,--`            | Postfix increment/decrement | Left          |
|       | `()`               | Function call               |               |
|       | `[]`               | Array access                |               |
| 2     | `++,--`            | Prefix increment/decrement  | Right         |
|       | `!`                | Logical negotiation         |               |
|       | `-`                | Unary negotiation           |               |
| 3     | `*,/,%`            | Arithmetic operations       | Left          |
| 4     | `+,-`              |                             |               |
| 5     | `<,>,<=,>=`        | Relational Operations       |               |
| 6     | `==,!=`            |                             |               |
| 7     | `&&`               | Logical conjunction         |               |
| 8     | \|\|               | Logical disjunction         |               |
| 9     | `?:`               | Ternary operator            | Right         |
| 10    | `=,+=,-=,*=,/=,%=` | Assignment operations       |               |

The scope of a variable always ends with the containing statement block. Variables with same name can not be declared within the same scope, but can be overshadowed by declaring a new variable with same name in a deeper nested scope. Global variables never fall out of scope, and can not be overshadowed. Privacy domains, and domain kinds can not be overshadowed. Variables with the same names can be declared in non-overlapping scopes.

Listing 6.6: Variable overshadowing
\code
	int x = 1;
	{ // nested scope
		int x; // overshadowing !
		int y = 1;
		x = 5;
	}
	// x == 1
	// y is not reachable
\endcode

*/
