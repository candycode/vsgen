#include <cassert>
#include "parse-signature.h"

//------------------------------------------------------------------------------
bool TestFind();
bool TestTrim();
bool TestSplit();
bool TestReturnType();
bool TestFunctionName();

//------------------------------------------------------------------------------
int main(int, char**) {
    
    assert(TestFind());
    assert(TestTrim());
    assert(TestSplit());
    assert(TestReturnType());
    assert(TestFunctionName());
    
    const std::string sig=" float Foo  ( int i, float j, const std::string& msg )";
    FunctionSignature fs = Signature(sig);
    assert(fs.returnType == "float");
    assert(fs.name == "Foo");
    assert(fs.parameters[0].type == "int");
    assert(fs.parameters[0].name == "i");
    assert(fs.parameters[1].type == "float");
    assert(fs.parameters[1].name == "j");
    assert(fs.parameters[2].type == "const std::string&");
    assert(fs.parameters[2].name == "msg");
    assert(fs.constModifier == false);

    const std::string sig2="void f() const; //a const method";
    FunctionSignature fs2 = Signature(sig2);
    assert(fs2.returnType == "void");
    assert(fs2.name == "f");
    assert(fs2.parameters.empty());
    assert(fs2.constModifier == true);
    return 0;
}

//==============================================================================
bool TestFind() {
    const std::string s = "1  , 2";
    return Find(s, ",") == 3;
}

bool TestTrim() {
    const std::string t = "  a  b ";
    return Trim(t) == "a  b";
}

bool TestSplit() {
    const std::string ts = " a, b ,  c "; 
    const std::vector< std::string > s = Split(ts, ",");
    return s.size() == 3 && s[0] == " a" && s[1] == " b " && s[2] == "  c ";
}

bool TestReturnType() {
    const std::string s = "float Foo(int i, float j, const std::string& msg)";
    return ReturnType(s).first == 0 && ReturnType(s).second == 5;
}

bool TestFunctionName() {
    const std::string s = "float Foo(int i, float j, const std::string& msg)";
    return FunctionName(s).first == 6 &&  FunctionName(s).second == 9;
}