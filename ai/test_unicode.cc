#include "tokens.cc"

int main(int argc, char** argv) {
    setlocale(LC_ALL, "en_US.utf8");
    string s;
    unsigned k = 0;
    while (++k < argc) {
        s += argv[k] + string(" ");
    }
    lowercase(s);
    cout << s << endl;
}
