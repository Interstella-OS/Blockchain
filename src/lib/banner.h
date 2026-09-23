#include <iostream>
#include <string>
#include <sstream>
using namespace std;

const string banner = R"(╔══════════════════════════════════╗
║       Core Blockchain C++        ║
║  RSA - P2P - Mining - Logging    ║
║  Version: 0.0.1                  ║
║  Author : Truong Chinh           ║
║  Github : Github.com/trgchinhh   ║
╚══════════════════════════════════╝

)";

inline void printbanner(const string banner){
    stringstream ss(banner);
    string line;
    while(getline(ss, line)){
        cout << line << endl;
    }
}