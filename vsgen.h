#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <utility>
#include <stdexcept>

//------------------------------------------------------------------------------
inline int Find(const std::string& s, const std::string& p, int off = 0) {
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
inline std::string Trim(const std::string& s) {
    std::string::const_iterator i = s.begin();
    while(isblank(*i) && i != s.end()) i++;
    if(i == s.end()) return std::string(i, s.end());
    std::string::const_iterator e = --s.end();
    while(isblank(*e) && e != s.begin()) e--;
    ++e;
    return std::string(i, e);
}

//------------------------------------------------------------------------------
inline std::vector< std::string > Split(const std::string& s,
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
struct Parameter {
    std::string type;
    std::string name;
    Parameter(const std::string& t, const std::string& n) : type(t), name(n) {}
};

//------------------------------------------------------------------------------
struct FunctionSignature {
    std::string returnType;
    std::string name;
    bool constModifier = false;
    std::vector< Parameter > parameters;    
    FunctionSignature(const std::string& r,
                      const std::string& n,
                      const std::vector< Parameter >& p,
                      bool cm) :
        returnType(r), name(n), parameters(p), constModifier(cm)
    {}
};

//------------------------------------------------------------------------------
inline std::pair< int, int > FunctionName(const std::string& f) {
    int e = Find(f, "(");
    --e;
    while(isblank(f[e])) --e;
    int b = e;
    while(!isblank(f[b])) --b;
    return std::make_pair(++b, ++e);
}

//------------------------------------------------------------------------------
inline std::pair< int, int > ReturnType(const std::string& s) {
    int b = 0;
    while(isblank(s[b])) ++b;
    std::pair< int, int > p = FunctionName(s);
    --p.first;
    while(isblank(s[p.first])) --p.first;
    return std::make_pair(b, ++p.first);
}

//------------------------------------------------------------------------------
inline bool ConstModifier(const std::string& s) {
    int i = Find(s, ")");
    if(i < 0) throw std::logic_error("Invalid signature");
    ++i;
    while(i != s.size() && isblank(s[i])) ++i;
    if(i != s.size() && Find(s, "const", i) == i) return true;
    return false; 
}

//------------------------------------------------------------------------------
inline FunctionSignature Signature(const std::string& f) {
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
    std::vector< Parameter > out;
    //split into array of <type name> strings
    std::vector< std::string > split = Split(s, ",");
    //split each <type name> into (type, name) pairs and add to map
    //as (name, type)
    for(auto& i: split) {
        int e = i.size() - 1;
        for(; e >= 0 && isblank(i[e]); --e);
        if(e < 0) continue;
        for(; e >= 0 && !isblank(i[e]); --e);
        if(e < 0) continue;
        const std::string type(i.begin(), i.begin() + e);
        const std::string name(i.begin() + e, i.end());
        out.push_back({Trim(type),Trim(name)});
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
inline Type ReadType(std::istream& is) {
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
                    type.comment = Trim(commentBuffer);
                    first = false;
                    commentBuffer.clear();
                } else {
                    const int c = Find(buffer, "//");
                    if(c > 0) {
                        type.methodInlineComments.push_back(
                            Trim(std::string(buffer.begin() + c, buffer.end())));
                    }
                    type.methods.push_back(Signature(buffer));
                    type.methodComments.push_back(Trim(commentBuffer));
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
inline std::string Substitute(const std::string in,
                              const std::string& from,
                              const std::string& to) {
    std::string out;
    int i = 0;
    int prev = 0;
    i = Find(in, from, i);
    while(i >= 0) {
        out.append(std::string(in.begin() + prev, in.begin() + i)); 
        out.append(to);
        prev = i + from.size();
        i = Find(in, from, prev);
    }
    if(prev < in.size()) {
        out.append(std::string(in.begin() + prev, in.end()));
    }
    return out;
}


//------------------------------------------------------------------------------
inline std::string GenerateSignature(const FunctionSignature& s,
                                     bool virt = false,
                                     bool pure = false) {
    std::ostringstream oss;
    if(virt) oss << "virtual ";
    oss << s.returnType << ' ' << s.name << "(";
    if(s.parameters.size() == 0) oss << ")";
    else {
        for(int i = 0; i != s.parameters.size() - 1; ++i) {
            oss << s.parameters[i].type << ' ' << s.parameters[i].name << ", ";
        }
        oss << s.parameters[s.parameters.size() - 1].type
            << s.parameters[s.parameters.size() - 1].name;
        oss << ")";
    }
    if(s.constModifier) oss << " const";
    if(pure) oss << " = 0";
    return oss.str();
}

//------------------------------------------------------------------------------
inline std::string GenerateMethod(const FunctionSignature& s,
                                  const std::string& impl,
                                  int indentationLevel = 1,  
                                  int tabSize = 4,
                                  const std::string& methodAccess = "->") {
    std::ostringstream oss;
    const std::string tab(tabSize, ' ');
    for(int i = 0; i != indentationLevel; ++i) {
        oss << tab;
    }
    oss << GenerateSignature(s) << " {\n";
     for(int i = 0; i != indentationLevel + 1; ++i) {
        oss << tab;
    }
    if(s.returnType != "void") oss << "return ";
    oss << impl << methodAccess << s.name << "(";
    if(s.parameters.size() == 0) oss << ");\n";
    else {
        for(int i = 0; i != s.parameters.size() - 1; ++i) {
            oss << s.parameters[i].name << ", ";
        }
        oss <<  s.parameters[s.parameters.size() - 1].name;
        oss << ");\n";
    }
    
    for(int i = 0; i != indentationLevel; ++i) {
        oss << tab;
    }
    oss << "}\n";
    return oss.str();
}
