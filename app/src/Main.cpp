#include <memory>

#include "MyApp.hpp"

int main(int argc, char **argv) {
    auto app = std::make_unique<MyApp>();
    return app->run(argc, argv);

}
