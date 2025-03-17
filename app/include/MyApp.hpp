#ifndef MYAPP_HPP
#define MYAPP_HPP

#include <engine/core/Engine.hpp>

class MyApp final : public engine::core::App {

public:
    void app_setup() override;
};

#endif //MYAPP_HPP
