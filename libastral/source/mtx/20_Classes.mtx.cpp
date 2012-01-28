.	Classes

The /Astral/ library contains the following classes, which are declared in the "astral.h" file.

As each source file is added to the /CodeBase/ instance a /CompilationUnit/ object is instantiated.
When its /initialise/ method is called the corresponding source file is parsed into an /astral::ast::AST/ object.
The compilation units are stored in the /CodeBase/ /files/ dictionary, which maps file paths to /CompilationUnit/ objects.
An instance of /SymbolDB/ is used to federate the symbols exposed by each compilation unit.
/CompilationUnit/ instances can be registered and deregisterd with the /SymbolDB/ allowing for the modification of files.

The /MethodSignature/ class is a helper class that parses a fully qualified method signature into its component parts,
i.e. return type, package, name, and parameters types.
Instances of /MethodSignature/ are passed to various methods so that programmers do not need to remember whether a fully-qualified string identifier is required, or not.

Similarly, the /MemberSignature/ class is a helper class that parses a fully qualified member signature into its component parts,
i.e. modifier, type, and name.
Instances of /MemberSignature/ are passed to various methods so that programmers do not need to remember whether a fully-qualified string identifier is required, or not.

The /VariableScopes/ class is used when implementing an AST traversal.
It is used to keep track of method parameters, and scoped variable declarations within methods.
An instance of the /VariableScopes/ class is required for several methods that resolve variable names to types.

Lastly, the /Export/ class contains a number of static methods that can be used for outputing in HTML.

~!include/astral.h~
#ifndef ASTRAL_H
#define ASTRAL_H

#include <openxds/types.h>
#include <openxds/global.h>

namespace astral {

class PUBLIC CodeBase;
class PUBLIC CompilationUnit;
class PUBLIC SymbolDB;
class PUBLIC MemberSignature;
class PUBLIC Method;
class PUBLIC MethodSignature;
class PUBLIC VariableScopes;
class PUBLIC Export;

};

#endif
~

For documentation for each class:
~html~
<table>
<tr><td><a href='./CodeBase.html'>CodeBase</a></td></tr>
<tr><td><a href='./CompilationUnit.html'>CompilationUnit</a></td></tr>
<tr><td><a href='./SymbolDB.html'>SymbolDB</a></td></tr>
<tr><td><a href='./MethodSignature.html'>MethodSignature</a></td></tr>
</table>
~

..		Dependencies

~!dep/OpenXDS.link~
$BUILD_PATH/OpenXDS
~

~!dep/OpenXDS_Core.link~
$BUILD_PATH/OpenXDS_Core/0.x
~

~!dep/libastral_ast.link~
$BUILD_PATH/Astral/0.x/libastral_ast
~

~!dep/libastral_tokenizer.link~
$BUILD_PATH/Astral/0.x/libastral_tokenizer
~
