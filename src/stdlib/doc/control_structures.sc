/**
@page control_structures Control structures
@brief Control structures in SecreC

@section control_structures Control structures

Most statements in SecreC are separated by a semicolon, and after normal execution of the statement, control is given to the next statement in the statement list. Statements can be grouped between curly braces to form a compound statement. An empty statement is considered a statement. Expressions ending with a semicolon are also statements and if the expression evaluates to non-void, the resulting value is discarded. 
 If the semicolon is not a part of a syntactic construct, such as a variable declaration or expression statement, it is considered a statement and has no effect. For example, **int** i;; is a composition of a declaration statement and a skip statement.

 @subsection if_statement If statement

The if-construct is the simplest of the control flow-modifying statements and executes a statement if the conditional expression evaluates to **true** . To avoid information leakage from the control flow, the conditional expression is forced to have a public boolean type. Multiple statements can be conditionally
evaluated with one if statement by combining them into single block.

Listing 6.22: if statements
\code
	//general form of if statement :

	if( expression )
		statement;

	//to conditionally execute multiple statements :

	if (i > j) {
		int t = i;
		i = j;
		j = t;
	}
\endcode

The if statement may be followed by the **else** keyword and another statement which will only be evaluated if the condition is not met.

@subsection while_loop while loop

The while statement is the simplest way of looping in SecreC. The body of a while statement is executed repeatedly as long as the condition is met. The conditional expression is evaluated and checked every time before evaluating the statement. For example, if the expression evaluates to false immediately,
the statement is not executed at all.

Listing 6.23: Loop through numbers from 1 to 10
\code
	int i = 1;
	while (i <= 10) {
		i ++;
	}
\endcode

@subsection do_while_loop do-while loop

The do-while construct is very similar to while. The only difference besides syntax is that the condition is checked every time after the execution of the statement instead of before. This means that the
statement is evaluated at least once.

Listing 6.24: Loop through numbers from 1 to 10
\code
	int i = 1;
	do {
		i += 1;
	} while (i <= 10);
\endcode

@subsection for_loop for loop

The for statement allows the programmer to specify an initialization expression (or declaration), a conditional expression and a step expression. Initialization is performed before looping is started, the statement is only executed if conditional is **true** and looping is stopped otherwise. The step expression is executed each time after the statement body.

Listing 6.25: Loop through numbers from 1 to 10
\code
	for (int i = 1; i <= 10; ++ i) {
		// this is the body of the statement
	}
\endcode

Every component of the for construct, other than the body, may be omitted. If the conditional expression is not present, the for statement is executed as if it was always **true**. For example infinitely looping statement can be written as simply as **for**(;;);.

@subsection break_statement break statement

The break statement ends the execution of current while, do-while or for loops.

Listing 6.26: Loop through numbers from 1 to 10
\code
	int i = 1;
	while (true) {
		if (i > 10)
			break ;
		i ++;
	}
\endcode

@subsection continue_statement continue statement

The continue statement skips the execution of the rest of the current while, do-while or for loop iteration and continues with the conditional expression and next iteration if required.

@subsection return_statement return statement

The return statement (further discussed in the section about functions) breaks the execution of a function.

@subsection assert_statement assert statement

The language supports statement for asserting a public boolean condition. The assert statement is purely for asserting properties that are supposed to hold, but are not immediately obvious. This offers some primitive safety guarantees and eases debugging by failing the program as early as possible in case of incorrect behaviour. If assertion fails the execution of the program is halted.

Listing 6.27: Assert statement
\code
int f () { ... }

void main () {
	int x = f ();
	assert (x > 0);
	...
}
\endcode

*/

