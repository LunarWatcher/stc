#include <catch2/catch_session.hpp>

int main(int argc, const char* argv[]) {

    auto sess = Catch::Session();

    return sess.run(argc, argv);
}
