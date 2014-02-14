#include <string>
#include <cassert>
#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>

//------------------------------------------------------------------------------
int Find(const std::string& s, const std::string& p, int off = 0) {
    int i = off;
    for(; i < s.size(); ++i) {
        int j = 0;
        for(; j != p.size(); ++j) {
            if(p[j] != s[i + j]) break;
        }
        if(j == p.size()) break;
    }
    return i < s.size() ? i :-1;
}

//------------------------------------------------------------------------------
std::string Trim(const std::string& s) {
    std::string::const_iterator i = s.begin();
    while(isblank(*i) && i != s.end()) i++;
    if(i == s.end()) return std::string(i, s.end());
    std::string::const_iterator e = --s.end();
    while(isblank(*e) && e != s.begin()) e--;
    ++e;
    return std::string(i, e);
}

//------------------------------------------------------------------------------
std::vector< std::string > Split(const std::string& s,
                                 const std::string& p) {
    std::vector< std::string > tokens;
    int prev = 0;
    int i = Find(s, p, prev);
    std::string t;
    while(i >= 0) {
        t = std::string(s, prev, i - prev);
        tokens.push_back(t);
        prev = i + p.size();
        i = Find(s, p, prev);
    }
    if(prev < s.size()) tokens.push_back(std::string(s.begin() + prev, s.end()));
    return tokens;
}

//------------------------------------------------------------------------------
struct FunctionSignature {
    std::string returnType;
    std::string name;
    bool constModifier = false;
    std::map< std::string, std::string > parameters;    
    FunctionSignature(const std::string& r,
                      const std::string& n,
                      const std::map< std::string, std::string >& p,
                      bool cm) :
        returnType(r), name(n), parameters(p), constModifier(cm)
    {}
};

//------------------------------------------------------------------------------
std::pair< int, int > FunctionName(const std::string& f) {
    int e = Find(f, "(");
    --e;
    while(isblank(f[e])) --e;
    int b = e;
    while(!isblank(f[b])) --b;
    return std::make_pair(++b, ++e);
}

//------------------------------------------------------------------------------
std::pair< int, int > ReturnType(const std::string& s) {
    int b = 0;
    while(isblank(s[b])) ++b;
    std::pair< int, int > p = FunctionName(s);
    --p.first;
    while(isblank(s[p.first])) --p.first;
    return std::make_pair(b, ++p.first);
}

//------------------------------------------------------------------------------
bool ConstModifier(const std::string& s) {
    int i = Find(s, ")");
    if(i < 0) throw std::logic_error("Invalid signature");
    ++i;
    while(i != s.size() && isblank(s[i])) ++i;
    if(i != s.size() && Find(s, "const", i) == i) return true;
    return false; 
}

//------------------------------------------------------------------------------
FunctionSignature Signature(const std::string& f) {
    const std::pair< int, int > r = ReturnType(f);
    const std::string ret(f.begin() + r.first, f.begin() + r.second);
    const std::pair< int, int > n = FunctionName(f);
    const std::string name(f.begin() + n.first, f.begin() + n.second);
    const bool cm = ConstModifier(f);
    int p = n.second;
    while(f[p] != '(' && p != f.size()) ++p;
    if(p == f.size()) throw std::logic_error("Invalid signature");    
    ++p;
    const int cp = Find(f, ")", p);
    if(cp < 0) throw std::logic_error("Invalid signature");    
    const std::string s(f.begin() + p, f.begin() + cp);
    std::map< std::string, std::string > out;
    //split into array of <type name> strings
    std::vector< std::string > split = Split(s, ",");
    //split each <type name> into (type, name) pairs and add to map
    //as (name, type)
    //for(auto& i: split) std::cout << i << std::endl;
    for(auto& i: split) {
        int e = i.size() - 1;
        for(; e >= 0 && isblank(i[e]); --e);
        if(e < 0) continue;
        for(; e >= 0 && !isblank(i[e]); --e);
        if(e < 0) continue;
        const std::string type(i.begin(), i.begin() + e);
        const std::string name(i.begin() + e, i.end());
        out[Trim(name)] = Trim(type);
    }
    return FunctionSignature(ret, name, out, cm);
}

//------------------------------------------------------------------------------
struct Type {
    std::string name;
    std::string comment;
    std::vector< FunctionSignature > methods;
    std::vector< std::string > methodComments;
    std::vector< std::string > methodInlineComments;
};

//------------------------------------------------------------------------------
//Service.interface
//Service
////a start method
//void Start()
//void End() //stops service
Type ReadType(std::istream& is) {
    std::string buffer;
    std::string commentBuffer;
    bool first = true;
    Type type;
    while(is) {
        buffer.clear();
        std::getline(is, buffer);
        if(buffer.size()) {
            std::string::iterator i = buffer.begin();
            for(; i != buffer.end() && isblank(*i); ++i);
            if(i != buffer.end() && isalpha(*i)) {
                if(first) {
                    type.name = Trim(buffer);
                    type.comment = commentBuffer;
                    first = false;
                    commentBuffer.clear();
                } else {
                    const int c = Find(buffer, "//");
                    if(c > 0) {
                        type.methodInlineComments.push_back(
                            std::string(buffer.begin() + c, buffer.end()));
                    }
                    type.methods.push_back(Signature(buffer));
                    type.methodComments.push_back(commentBuffer);
                    commentBuffer.clear();
                }
            } else if(i < buffer.end() - 1 && *i == '/' && *(i+1) == '/') {
                commentBuffer.append(std::string(i, buffer.end()) + "\n");
            }   
        }    
    }
    return type;
}


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
    assert(fs.parameters["i"] == "int");
    assert(fs.parameters["j"] == "float");
    assert(fs.parameters["msg"] == "const std::string&");
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