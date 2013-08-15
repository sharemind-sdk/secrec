/**
@page types Types
@brief Types in SecreC

@section types Types

SecreC is strongly and statically typed. There are two features that make SecreC type system unique. The type system is strongly focused on arrays, and all primitive data has privacy (security) type. Types in SecreC start with an optional security type, followed by the primitive data type and finally
an optional array dimensionality type between double square brackets. For example, we can write: **int** for integers, private **bool** for booleans in security domain *private*, or **public** **int** [[5]] for public 5-dimensional integer array. The default security type is public, and the default dimensionality type is
scalar.

@subsection privacy_types Privacy types

Privacy types in SecreC are classified into public, and non-public security domains. Public types can be optionally annotated with keyword **public**, and non-public types are annotated with a privacy domain. Security domains are declared in global scope using a keyword **domain**. Every privacy domain belongs
to some privacy domain kind. The distinction between privacy domains, and privacy kinds is necessary because it’s possible to have data in different privacy domains of the same kind. Security domain kinds are declared in global scope using the **kind** keyword.
 To declare a privacy domain kind, and two domains that belong to that kind we could write the
following in the global scope:

Listing 6.2: Different protection domains of the same kind

	kind additive3pp ;
	domain sharemind_test_pd additive3pp ;
	domain my_pd additive3pp ;

 We impose a lattice structure on the privacy types, and often call public security type looser than non-public security types.
 Privacy types are used to statically guarantee that information does not flow from the private memory space into the public one, if not otherwise specified with the declassify construct. There is no special construct for moving public data into the private space, as it’s allowed implicitly in many cases. As almost all operations on private data types can be significantly slower than same operations on public data we recommend to use private variables as sparingly as possible, and to keep operations on them to a minimum. The choice between public and private operations also allows the algorithm developer to balance between security and performance as required by the application.

@subsection primitive_types Primitive types

SecreC has following primitive data types:

- **int**, **int64**, **int32**, **int16** and **int8** for signed integers,
- **uint**, **uint64**, **uint32**, **uint16** and **uint8** for unsigned integers,
- **bool** for booleans, either **true** or **false**.
- **float32**, **float** and **float64** for floating point numbers
- **string** for strings

Integer types **int**, and **uint** are synonyms for 64-bit signed, and unsigned integer types. Floating point type **float** is synonymous with 32-bit floating point type.

@subsection array_types Array types

Types declare the number of dimensions by writing an integer literal between the double square brackets. For example, we write [[1]] to declare one-dimensional vectors, and [[2]] for declaring matrices. It is possible to denote scalars by [[0]], but it’s more concise to omit the annotation. The number of dimensions is not limited by any hard cap.
 Throughout this document we will call the number of dimensions of an array its *dimensionality*. Dimensionality is a static property and never changes throughout the life of a variable. This is guaranteed by the type system. We often call a one-dimensional array a vector, and a two-dimension array a matrix.




*/
