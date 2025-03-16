//
// Created by marko-cvijetinovic on 1/17/25.
//
#include "GUIController.hpp"

#include "MyApp.hpp"

#include "MainController.hpp"

void MyApp::app_setup() {
    auto main_controller = register_controller<MainController>();
    main_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());

    auto gui_controller = register_controller<GUIController>();
    main_controller->before(gui_controller);
}

