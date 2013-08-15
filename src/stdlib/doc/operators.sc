/**

@page operators Operators
@brief Operators within the SecreC language

@section public public protection domain

Supported types: \ref bool "bool" / \ref uint8 / \ref uint16 / \ref uint32 / \ref uint64 "uint" / \ref int8 / \ref int16 / \ref int32 / \ref int64 "int" / \ref float32 "float32" / \ref float64

@subsection pub_operators operators

| type     | operators              |
| :------: | :--------------------: |
| bool     | > , < , >= , <= , == , != |
| uint8:   | + , - , * , / , % , > , < , >= , <= , == , != |
| uint16:  | + , - , * , / , % , > , < , >= , <= , == , != |
| uint32:  | + , - , * , / , % , > , < , >= , <= , == , != |
| uint64:  | + , - , * , / , % , > , < , >= , <= , == , != |
| int8:    | + , - , * , / , % , > , < , >= , <= , == , != |
| int16:   | + , - , * , / , % , > , < , >= , <= , == , != |
| int32:   | + , - , * , / , % , > , < , >= , <= , == , != |
| int64:   | + , - , * , / , % , > , < , >= , <= , == , != |
| float32: | + , - , * , / , % , > , < , >= , <= , == , != |
| float64: | + , - , * , / , % , > , < , >= , <= , == , != |

@subsection pub_cast casting

| type     | castings               |
| :------: | :--------------------: |
| bool     | uint8/16/32/64 , int8/16/32/64 , float32/64 |
| uint8:   | bool , uint16/32/64 , int8/16/32/64 , float32/64 |
| uint16:  | bool , uint8/32/64 , int8/16/32/64 , float32/64 |
| uint32:  | bool , uint8/16/64 , int8/16/32/64 , float32/64 |
| uint64:  | bool , int8/16/32 , int8/16/32/64 , float32/64 |
| int8:    | bool , int16/32/64 , uint8/16/32/64 , float32/64 |
| int16:   | bool , int8/32/64 , uint8/16/32/64 , float32/64 |
| int32:   | bool , int8/16/64 , uint8/16/32/64 , float32/64 |
| int64:   | bool , int8/16/32 , uint8/16/32/64 , float32/64 |
| float32: | bool , uint8/16/32/64 , int8/16/32/64 , float64 |
| float64: | bool , uint8/16/32/64 , int8/16/32/64 , float32 |

@section additive3pp additive3pp protection domain

Supported types: \ref bool "bool" / \ref uint8 / \ref uint16 / \ref uint32 / \ref uint64 "uint" / \ref int8 / \ref int16 / \ref int32 / \ref int64 "int" / \ref float32 "float32" / \ref float64 / \ref xor_uint8 / \ref xor_uint16 / \ref xor_uint32 / \ref xor_uint64 "xor_uint64"

@subsection a3p_operators operators

| type        | operators              |
| :---------: | :--------------------: |
| bool        | == , != |
| uint8:      | + , - , * , > , < , >= , <= , == , != |
| uint16:     | + , - , * , > , < , >= , <= , == , != |
| uint32:     | + , - , * , / , > , < , >= , <= , == , != |
| uint64:     | + , - , * , / , > , < , >= , <= , == , != |
| int8:       | + , - , * , > , < , >= , <= , == , != |
| int16:      | + , - , * , > , < , >= , <= , == , != |
| int32:      | + , - , * , > , < , >= , <= , == , != |
| int64:      | + , - , * , > , < , >= , <= , == , != |
| float32:    | + , - , * , / , == , != |
| float64:    | + , - , * , / , == , != |
| xor_uint8:  | > , < , >= , <= , == , != |
| xor_uint16: | > , < , >= , <= , == , != |
| xor_uint32: | > , < , >= , <= , == , != |
| xor_uint64: | > , < , >= , <= , == , != |

@subsection a3p_cast casting

| type     | castings               |
| :------: | :--------------------: |
| bool     | uint8/16/32/64 , int8/16/32/64 , float32/64 , xor_uint8/16/32/64 |
| uint8:   | bool , uint16/32/64 , int8, float32/64 |
| uint16:  | bool , uint8/32/64 , int16, float32/64 |
| uint32:  | bool , uint8/16/64 , int32, float32/64 |
| uint64:  | bool , int8/16/32 , int64 |
| int8:    | bool , uint8, float32/64 |
| int16:   | bool , uint16, float32/64 |
| int32:   | bool , uint32, float32/64 |
| int64:   | bool , uint64 |
| float32: | - |
| float64: | - |
| xor_uint8:  | - |
| xor_uint16: | - |
| xor_uint32: | - |
| xor_uint64: | - |

*/
 
