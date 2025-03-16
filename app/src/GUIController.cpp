//
// Created by marko-cvijetinovic on 1/18/25.
//

#include <engine/graphics/GraphicsController.hpp>
#include <imgui.h>

#include "GUIController.hpp"

void GUIController::initialize() {
    set_enable(false);
}

void GUIController::draw() {
    auto graphics = get<engine::graphics::GraphicsController>();
    auto camera   = graphics->camera();

    graphics->begin_gui();
    ImGui::Begin("Camera info");

    ImGui::Text("Camera position: (%f, %f, %f)", camera->Position.x, camera->Position.y, camera->Position.z);

    ImGui::End();
    graphics->end_gui();
}

void GUIController::poll_events() {
    auto platform = get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KEY_F2).state() == engine::platform::Key::State::JustPressed) {
        set_enable(!is_enabled());
    }
}
