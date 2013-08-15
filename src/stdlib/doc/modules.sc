/**
@page ref_modules Modules
@brief Modules in SecreC

@section ref_modules Modules

SecreC supports very simple module system. A module name can be declared with **module** keyword, and module can be imported using a **import** keyword. Filename of the module must match with the modules name. Imported modules are searched within paths specified to the compiler. Importing a module will make all of the global symbols defined within the imported module visible. Modules can not be separately compiled and linked, they simply offer a way to break code into components.

Listing 6.33: Module syntax
\code
module additive3pp;
import common;
...
\endcode

Imported symbols will not be exported by a module. If an imported symbol is redefined, a type error is raised. Procedures and templates with the same name but different parameters are considered to be different symbols.

*/