/**
@page templates Templates
@brief Templates in SecreC

@section templates Templates

In order to support domain type polymorphic procedures the language has support for C++’like function templates. Function templates give the language static domain, type and dimension polymorphism. Templates are declared using the **template** keyword, and the template domain, type and dimension parameters are listed between < and > parenthesis.

Listing 6.30: Simple template declaration
\code
	template <domain D, type T, dim N>
	D T minimum (D T[[N]] arr) { ... }
\endcode

Templates may restrict a domain variable to a protection kind.

Listing 6.31: Simple template declaration
\code
	kind additive3pp ;
	template <domain D : additive3pp,  type T>
	T declassify (D T x) { ... }
\endcode

Code generation of concrete template instances (or instantiation for short) is performed on demand after code for regular functions has been generated. If at any point a template function is called, the instantiation to given call parameters is added into instantiation queue if the template has yet to be instantiated with those parameters. The process continues as long as the queue is not empty. Note that the instantiation process eventually terminates, as there’s finite number of templates, and finite number of declared domain types. The template body is type checked when it’s required to be instantiated to concrete domain types. We do not perform two phase name lookup as C++, and uninstantiated templates may even refer to undeclared global symbols. If multiple function or template matches are found for a single call site a following priority system is used to select a single match. First the number of template variables are compared, next the number template variables that are not restricted with domain kind are compared, and finally the number of quantified function domain type parameters are compared. A unique minimum is used to perform the call. If unique minimum is not found a type error is raised. The intuitive idea behind the resolution system is that the most specialized template or procedure is selected. For example, reclassification can be defined between two potentially different domains, but also within a single domain. If so, then the version that uses a single template variable is selected whenever possible.

Listing 6.32: Reclassify
\code
	template <domain D : additive3pp , domain L : additive3pp >
	D int reclassify (L int x) { ... }
	template <domain D>
	D int reclassify (D int x) { return x; }
 \endcode

*/