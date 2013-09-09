/**
@page code_stucture Code structure
@brief code structure in SecreC

A SecreC program starts with a series of global value definitions which are followed by a series of function definitions. Other kinds of statements are not allowed in the global program scope. A SecreC program has to define a function called “main” taking no arguments and returning no value. This function is
called when the program is executed by the Sharemind machine. There are two types of comments which are treated as white-space. The single line comments start
with two consecutive forwards slashes `//` and continue until a new line. Multi line comments start with a consecutive forward slash and asterisk and continue up until and including a consecutive asterisk and forward slash. New comments are not started if the starting characters are found within string literals or other comments. Comments count as a white-space, and can be used to separate tokens.

Listing 6.1: Trivial program
\code

	void main () {
		return ;
	}

\endcode
*/
