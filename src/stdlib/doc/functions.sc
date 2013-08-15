/**
@page functions Functions
@brief Functions in SecreC

@section functions Functions

The function (also referred to as a procedure) header specifies the return type, name, types and names of the formal parameters, and body consisting of a list of statements. The same naming rules that applied to variable names apply to function identifiers. All functions have to be defined before they can be called, or in other words, a body of a function can only make calls to itself and to previously defined functions. Procedures have a global scope, and it is not possible to define nested functions. Every SecreC program has to define a special function called “main” with return type void taking no parameters. This function is called when the program execution starts. 
Arguments to a function call are always evaluated from left to right and are passed by value.

Listing 6.28: Parameter passing
\code
void do_nothing (int x, int y) {}
void change (int x) { x = 42; }
void main () {
	int x;
	do_nothing (x = 1, x = 2);

	// x == 2

	change (x);

	// x == 2
}
\endcode

The execution of a function can be stopped and values can be returned using a return statement. A return statement breaks the execution of any control-flow structure. If the procedure returns no value the execution of function body is allowed to reach the end of the function. Otherwise, if a function should return a value it has to reach a return statement. Static checks are performed to guarantee that a procedure reaches a return statement and that returned values are appropriately typed.

@subsection overloading Overloading

Declared function can be overloaded by the number of parameters, parameter types, or even by return type. For example, it is possible to define a functions with the same name for summing values of arrays of different dimensionalities.

Listing 6.29: Summing values
\code
	int sum (int [[1]] arr) { ... }
	int sum (int [[2]] arr) { ... }
\endcode

*/