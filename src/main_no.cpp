#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " [arg]" << endl;
        return -1;
    }
    cout << "Nó " << argv[1] << " inicializado" << endl;
    return 0;
}
