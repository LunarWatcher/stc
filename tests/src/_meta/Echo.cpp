#include <iostream>

int main(int argc, const char* argv[]) {
    for (int i = 0; i < argc; ++i) {
        std::cout << "Argument: " << argv[i] << std::endl;
    }
}
