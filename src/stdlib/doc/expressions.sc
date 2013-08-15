/**
@page expressions Expressions
@brief Expressions in SecreC

@section expressions Expressions

Many of the expressions implicitly operate point-wise on arrays and, in many cases, scalar values are
automatically converted into a higher-dimensional array, if the size of resulting array is derivable from
the context. For example, if one argument of addition operator is matrix and other is constant scalar
1, then the scalar is converted into properly sized matrix before point-wise addition is performed. This
feature is desirable in several data mining operations and simplifies the development of such code.

@subsection arithmetic_operators Arithmetic Operators

SecreC supports the following arithmetic operators: (a) + for addition, (b) - for subtraction, (c) * for
multiplication, (d) / for division, (e) % for modulo, and (f) - for numeric unary negation.
 Arithmetic operators can be called on mixed security typed parameters. The resulting security type
is always the stricter of parameters, and the value with the looser security type is implicitly classified. If operator is called on mixed security typed parameters a type error is raised. Calling binary operators on
expressions of mixed data types is forbidden, and requires explicit data type cast. The types of arithmetic
operators are informally described in Table 2.

Table 2: Public scalar operator types

| Operator          | Return        | Parameters           |
| :---------------- | :------------ | :------------------- |
| `&&,`\|\|`,!`     | **bool**      | **bool**             |
| `<,>,<=,>=,==,!=` | **bool**      | Any types            |
| `+,*,/,%,++`      | Argument type | Integer types        |
| `-,--`            | Argument type | Signed integer types |

@subsection logical_operators Logical operators

Following relational operators are supported: (a) == equality, (b) != inequality, (c) < less-than, (d) <= less-or-equal, (e) > greater-than, and (f) >= greater-or-equal. Relational operators always return boolean value. Arguments can be boolean, and integer typed. Arguments have to be of the same data type, but
can be called on mixed security types in which case the result is the stricter of the two. If needed scalars are implicitly converted to arrays.
SecreC supports following logical operations: (a) && for conjunction, (b) || for disjunction, and (c) ! for logical unary negation. All require arguments and return types to be boolean. As with arithmetic and relational expressions boolean expressions can be called on mixed security typed parameters, and
implicit scalar to array conversion is performed when required. However, calling boolean expressions on mixed security types has an important caveat.
It’s common for strict languages to evaluate logical boolean expressions lazily (known as *short-circuit evaluation* ). For example, if the first branch of conjunction expression evaluates to false the second branch is never evaluated. This is also the case in SecreC for boolean expressions called on
public scalar values, but not if any of the arguments is private or scalar. The reason for this behavior is that any changes in control flow are publicly visible, and that can cause unwanted private data to leak
into public space. To avoid leaking information in such manner the logical expression on private data always evaluate both branches. Due to the point-wise behavior a short-circuit evaluation is not possible for boolean expressions if one of the arguments is a non-scalar.

@subsection casting_operators Casting operators

SecreC does not support implicit type casts, but does support explicit C-style data-type conversion. To perform a type cast a parenthesised data-type is written before an expression. Non-zero integer typed values cast to **true** boolean type, and zero values casts to **false**. Conversely, booleans cast to 1 in case
of **true** and 0 in case of **false**. Non-public data type conversions are allowed.

Listing 6.7: Data type conversion
\code
	int i = 0;
	bool b = true;
	b = (bool) i;
\endcode

@subsection type_annotation Type annotation

Due to lack of implicit type casts, and support for function overloading by return type it’s sometimes required to specify a type of an expression explicitly. For example, if a procedure is over-loaded by multiple different return types, an explicit type annotation is required to disambiguate the function call.
To annotate an expression with a type, the type is written after double colon following the expression. If the type annotation expression type checks, then it evaluates to given type.

Listing 6.8: Type annotations
\code
	int g (int x, int y) { ... }
	sharemind_test_pd int g (int x, int y) { ... }
	void main () {
		int x, y;
		f (g (x, y) :: sharemind_test_pd );
	}
\endcode

@subsection ternary_operator Ternary operator

The ternary expressions take three arguments the first of which is a boolean value. If the first argument evaluates to **true** then the value of the second branch is returned. Otherwise, if the first argument evaluates to **false** , the value of the third branch is returned.

Listing 6.9: Use of ternary operator
\code
	int i = 1;
	int x = i < 1 ? 0 : 42;
	// x == 42
\endcode

Unlike with logical expressions, the boolean argument of ternary expression is forced to be of a public security type. Data and dimensionality types of branches are also forced to be equal. The resulting
privacy type is the stricter of the two branches, and the looser type is implicitly classified. The dimensionalities of branches are forced to be equal because the shape of the resulting array
might not be derivable by evaluating only one branch. Consider the case where a conditional expression evaluates to **true**, the first branch is scalar, but the second is a matrix. In that case, the scalar value
will be required to be converted to constant array with the shape of the second branch, but in order to derive the shape of the second branch it needs to be evaluated. By forcing the branches to have same
dimensionalities we can avoid such issues.
Ternary expressions operate point-wise if the conditional expression is non-scalar. In order to avoid even more confusing semantics, we force the branches to have equal number of dimensions.

@subsection assignment_operators Assignment operators

An assignment expression evaluates the right hand side expression, and returns the value. After the evaluation the variable points to the computed value of the right hand side expression. In the case of
arrays, even the shape of the left hand side is overwritten. The assignment operator is right-associative. The assignment operator can be used in conjunction with binary arithmetic operators to modify the
variable. For example, to increment a variable i by two we can write i += 2 or, similarly, i -= 1 to decrement it by one. If the right hand side of any assignment expression is public, but the left hand side
is non-public then the public data is first implicitly classified before the assignment is performed.

Listing 6.10: Assignment operators
\code
	public int i;
	private int secret ;
	// some computation
	i += 5;
	 // increment by 5
	secret = i; // implicit classify
\endcode

@subsection increment_decrement Increment/decrement operators

SecreC supports both prefix and postfix increment and decrement operators. We write ++ i or i ++ to increment a variable i by one. The difference between the two methods is that the prefix increment first increments the value and then returns it, while the postfix increment first returns a copy of the value of the variable and then increments it. The same holds for the decrement operator.

Listing 6.11: Increment and decrement operators
\code
	int i = 5;
	int j;
	j = i ++; // j == 5, i == 6
	j = 42; // j == 42, i == 6
	j = -- i; // j == 5, i == 5
\endcode

@subsection declassify_operator Declassify operator

The only way to move private data into a public memory space is via the **declassify** operator. The declassify construct takes a private argument with any kind of data type and returns the value with the same data type in public security domain. In order to forbid expressions such as **declassify**(**declassify**(x)), the arguments of declassify are not implicitly classified.

Listing 6.12: Declassify example
\code
	private int val;
	// read the val from database
	// do some computation on data
	bool result = declassify (val < 0);
\endcode


*/