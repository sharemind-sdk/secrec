/**
@page arrays Arrays
@brief Arrays in SecreC

@section arrays Arrays

SecreC is strongly focused on arrays, and the majority of arithmetic, relational and logical operations operate point-wise on them. The main motivation for this behaviour is that private operations are individually slow on some protection domain implementations, and usually require a great deal of network communication overhead. Performing private operations in parallel reduces the time cost involved.The network communication cost is reduced too, as it’s more efficient to send data in bulk rather than sending small packets individually for each operation.
 SecreC supports multidimensional rectangular arrays. The arrays are more similar to those in Fortran than those in C or Java. The main difference is that multi dimensional arrays in both C and Java are so called Iliffe vectors [Ili61] storing single dimensional vector of pointers to arrays of one dimension
less. Like in Fortran arrays in SecreC are always stored as a contiguous block of memory. 
 Every array is associated with sizes of all its dimensions, we call tuple of those a shape of the array. The shape, unlike dimensionality, is a dynamic property and can freely change in the process of program evaluation. The shape is also always a public property, even for arrays containing private data.

 @subsection assigning_arrays Assigning arrays

 There are two ways to assign arrays. If the left hand side is a variable then its data and shape are rewritten with that of the right hand side expression. This allows the developer to change the size of an array dynamically. A static check is performed to guarantee that dimensionalities of both sides match. If a scalar is assigned to an array, the assignment is performed point-wise.

Listing 6.13: Examples of array assignments
\code
	int [[1]] full; // empty right now
	int [[1]] arr (10);
	full = arr; // no longer empty
	arr = 1; // all values set to 1
\endcode

@subsection array_expressions Array expressions

Arithmetic, logical and relational operations all operate point-wise on arrays. Additionally, where context allows, scalar values are converted into properly sized arrays implicitly. For example scalars at right hand side of array assignment expressions are converted to constant arrays, and all arithmetic, relational and boolean operators convert scalars to arrays implicitly.

Listing 6.14: Array expressions
\code
	int [[1]] a (10);
	int [[1]] b (10);
	// point -wise operations :
	a = b + b;
	++ a;
	// implicit conversions :
	b = 2 * a;
	a = 5;
\endcode

@subsection indexing_arrays Indexing arrays

Indexing of arrays is performed by writing a comma separated list of indices between square brackets after an expression. It’s statically checked that number of indices is equal to the dimensionality of the array being indexed, and that all indices are signed integers of a public security type. Dynamic checks
are performed to guarantee that indices are within array bounds. Indexing of variables may also be performed in the left hand side of an assignment.
Let us first consider the case of indexing a single dimensional array. Indexing with a public integer returns a value in the array at that position, if it is within the bounds. Run-time error is raised otherwise.
Like in C, indices always start at zero.

Listing 6.15: Indexing
\code
	int [[1]] arr (5) = 1;
	int val = arr [0]; // val == 1
	arr [1] = 0; // second element of arr is now 0
\endcode

Any index is allowed to be a slice by denoting it by a colon : separated lower and an upper bound expressions. A slice defines an interval of indices which includes the lower bound of the slice, but excludes the upper bound. The returned value is an array with elements taken from the original array
falling between the bounds denoted by that slice. A dynamic check is performed to ensure that all of the elements denoted by the slice are within array bounds.
Another way of using slice indexing is in the left hand side of an assignment in which case the region specified by indices is overwritten by the value of the right-hand-side. The assignment is only performed if the shapes of both sides match (an error is raised otherwise). The assignment of a scalar value has the
expected point-wise behavior.

Listing 6.16: Indexing with slices
\code
	int [[1]] arr (5);
	arr [1:4] = 1;
	arr [2:3] = 2;
	// arr == [0 ,1 ,2 ,1 ,0]
\endcode 

There is also some syntactic sugar associated with array slices. If the lower bound of a slice is missing, it is taken to be the constant zero, if an upper bound is missing, the size of the array is used. For example, indexing a one-dimensional array with just a colon returns a copy of the original array.In case of multiple indices, the indexing is performed on all of the dimensions in a natural manner. A nice property of our approach is that it is possible to compute the resulting dimensionality of the expression by simply counting the number of slices that the array has been indexed with. Note, that indexing an array multiple times does not have the same effect as in C. For example, if mat is a two-dimensional array, then the expression mat[0][0] does not type check as mat has to be indexed with two indices while only one is given. The type-correct expression would be mat[0, 0]. Chaining indices is still possible. For example, given a vector vec the expression vec[1:][2] is completely legal
and returns the fourth element of the vector.

@subsection array_primitives Array Primitives

In addition to the indexing operators, there are four additional built-in constructs for manipulating arrays.

	size

Returns the number of elements in the argument array as a public integer. Size can be called on expressions
of any type. Computing the size of an array takes—in the worst case— a linear number of multiplications
in the number of dimensions. If invoked on scalars the size expression always evaluates to 1, and the
subexpression will be evaluated.

Listing 6.17: Size expression
\code
	int [[3]] arr (2, 3, 5);
	// size (arr) == 2*3*5
	int [[1]] empty ;
	// size ( empty ) == 0
\endcode

\n

	shape

Returns the sizes of dimensions of the argument array as a public integer vector. The type of the argument
is not restricted in any way. If called on scalar value, an empty array is returned. The subexpression is
always evaluated, even if the value of it is not required.

Listing 6.18: Shape expression
\code
	int [[3]] arr (2, 3, 5);
	int [[1]] s = shape (arr ); // == [2, 3, 5]
\endcode

\n

	cat

Concatenates the first two argument arrays along the dimension specified by the third argument. The
last argument has to be a public integer literal. The argument arrays must have equal dimensions and the
same data type. The last argument has to be between zero and the number of dimensions of arguments.
The last argument may be omitted in which case it is implicitly assumed to be zero.

Listing 6.19: Concatenation of arrays
\code
	int [[1]] a (5) = 0;
	int [[1]] b (5) = 1;
	int [[1]] c = cat (a, b); // or cat (a, b, 0)
	// size (c) == 10
	// c [0:5] == 0
	// c [5:10] == 1
\endcode

Run-time error is raised if shape of concatenated arrays does not match.

\n

	reshape

Returns an array with the same data, but a different shape than the original. The first argument is the
initial array and the rest of the arguments specify the new shape. A run-time check is performed to check
that the number of elements in the old and new arrays are equal. The returned array inherits the values
and the security type of the original array. A common idiom is to combine the use of reshape and size
for flattening multi-dimensional arrays into a vector form.

Listing 6.20: Array flattening
\code
	int [[2]] mat (5, 5);
	int [[1]] arr = reshape (mat , size (mat ));
	// size (arr) == 25
\endcode

It is possible to use reshape to create a temporary constant array from a scalar with fixed size.
This supplies a convenient tool for changing both the size and value of an already created array, or for
constructing temporary constant arrays of given size.

Listing 6.21: Temporary constant array
\code
	int [[1]] arr (100);
	// some computation on arr
	arr = reshape (1, 10);
\endcode




*/