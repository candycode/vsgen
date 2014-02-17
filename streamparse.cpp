#include <sstream>
#include "parse-signature.h"

using namespace std;
//------------------------------------------------------------------------------
int main(int, char**) {
    const char* dec = R"Decl(
    Service
    ///Start service
    ///@param port port number
    void Start(int port)
    ///Stop service
    void Stop() // need to restart after stop
    int GetPort() const
    )Decl";
    istringstream iss(dec);
    Type type = ReadType(iss);
    cout << type.comment << endl;
    cout << "Comments:" << endl;
    for(auto& i: type.methodComments) cout << i << endl;
    for(auto& i: type.methodInlineComments) cout << i << endl;
    for(auto& i: type.methods) {
        cout << "\n---------------------\n";
        cout << "return type: " << i.returnType << endl;
        cout << "name:        " << i.name << endl;
        cout << "const:       " << boolalpha << i.constModifier << endl;
        if(i.parameters.size() > 0) cout << "parameters:     " << endl;
        for(auto& j: i.parameters) {
            cout << "  name: " << j.name << endl;
            cout << "  type: " << j.type << endl;
        }
     }
    return 0;
}
