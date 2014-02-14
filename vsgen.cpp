#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include "parse-signature.h"

static const char* T =
R"T(
class $C {
public:
    template < typename T >
    $C(const T& t) : $I(new $CModel<T>(t)) {}
    $C(const $C& $c) : $I($c.$I->Clone()) {}
    $C($C&&) = default;
    $C& operator($C $c) {
        $I = std::move(c.$I);
    }
public: //public interface
    $Methods
private:
    struct $CType {
        $IMethods
        $CType* Clone() const = 0;
        virtual ~$CType() {}
    };
    template < typename T >
    struct $CModel : $CType {
        $MMethods
        $CModel(const T& t) : d(t) {}
        $CType* Clone() const {
            return new CModel(d);
        }
        T d;
    };
private:
    std::unique_ptr< $CType > $I;
};
)T";




//------------------------------------------------------------------------------
bool TestSubstitute();
bool TestGenClass();

//------------------------------------------------------------------------------
std::string GenerateClass(std::istream& is, bool comments = true, int indent = 4) {
    Type t = ReadType(is);
    std::string instname = t.name;
    instname[0] = std::tolower(instname[0]);
    std::string impl =  instname + "Impl_";
    const std::string classPlaceHolder = "$C";
    const std::string implPlaceHolder = "$I";
    std::string src = Substitute(T, classPlaceHolder, t.name);
    src = Substitute(src, implPlaceHolder, impl);
    src = Substitute(src, "$c", instname);
    std::cout << src << std::endl;
    std::cout << src << std::endl;
}


using namespace std;


//------------------------------------------------------------------------------
int main(int argc, char** argv) {
    TestGenClass();
//    if(argc != 4) {
//        cout << "usage: " << argv[0] << "<input file> <output file> <indent>\n";// <typeinfo=y|n>"
//        return 0;
//    }
//    ifstream is(argv[1]);
//    if(!is) {
//        cerr << "Cannot open file " << argv[1] << endl;
//        return 1;
//    }
//    const string g = GenerateClass(is);
//    cout << g << endl;
    return 0;
}

//==============================================================================
bool TestSubstitute() {
    const std::string s =
    R"(
    class $C : $CType {
    public:
    $C() : i_(0) {}
    ~$C() {}        
    )";
    
    const std::string ss = Substitute(s, "$C", "MyClass");
    
    cout << ss;
    return true;
}

bool TestGenClass() {
    const char* C = 
    R"(
    Service
    void Next()
    void Flush()
    int Failed()
    void Start()
    void Stop()
    )";
    std::istringstream iss(C);
    cout << GenerateClass(iss);
    
}