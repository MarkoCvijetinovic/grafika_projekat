//
// Created by marko-cvijetinovic on 1/18/25.
//

#ifndef GUICONTROLLER_HPP
#define GUICONTROLLER_HPP

#include<engine/core/Engine.hpp>

class GUIController : public engine::core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "GUIController";
    }

    void initialize() override;

    void draw() override;

    void poll_events() override;
};

#endif //GUICONTROLLER_HPP
