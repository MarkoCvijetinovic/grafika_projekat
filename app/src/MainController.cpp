//
// Created by marko-cvijetinovic on 1/17/25.
//

#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>

#include "MainController.hpp"
#include "GUIController.hpp"

class MainPlatformEventObserver : public engine::platform::PlatformEventObserver {
public:
    void on_mouse_move(engine::platform::MousePosition position) override;
};

void MainPlatformEventObserver::on_mouse_move(engine::platform::MousePosition position) {
    //auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
    //camera->process_mouse_movement(position.x, position.y, false);
}

void MainController::initialize() {
    auto platform = get<engine::platform::PlatformController>();
    platform->register_platform_event_observer(std::make_unique<MainPlatformEventObserver>());
    engine::graphics::OpenGL::enable_depth_testing();
}

bool MainController::loop() {
    auto platform = get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KeyId::KEY_ESCAPE).is_down())
        return false;

    return true;
}

void MainController::draw_backpack() {
    auto resources = get<engine::resources::ResourcesController>();
    auto backpack  = resources->model("backpack");
    auto shader    = resources->shader("basic");
    shader->use();

    auto graphics = get<engine::graphics::GraphicsController>();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    glm::mat4 model = glm::mat4(1.0f);
    model           = translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
    model           = scale(model, glm::vec3(0.3f));
    shader->set_mat4("model", model);

    backpack->draw(shader);
}

void MainController::begin_draw() {
    engine::graphics::OpenGL::clear_buffers();
}

void MainController::draw_skybox() {
    auto resources = get<engine::resources::ResourcesController>();
    auto skybox    = resources->skybox("mountain_skybox");
    auto shader    = resources->shader("skybox");
    auto graphics  = get<engine::graphics::GraphicsController>();
    graphics->draw_skybox(shader, skybox);
}

void MainController::draw() {
    draw_backpack();
    draw_skybox();
}

void MainController::end_draw() {
    auto platform = get<engine::platform::PlatformController>();
    platform->swap_buffers();
}

void MainController::update() {
    update_camera();
}

void MainController::update_camera() {
    auto gui_controller = get<GUIController>();
    if (gui_controller->is_enabled())
        return;

    auto platform = get<engine::platform::PlatformController>();
    auto graphics = get<engine::graphics::GraphicsController>();
    auto camera   = graphics->camera();

    float dt = platform->dt();
    if (platform->key(engine::platform::KeyId::KEY_W).is_down()) {
        camera->process_keyboard(engine::graphics::Camera::Movement::FORWARD, dt);
    }
    if (platform->key(engine::platform::KeyId::KEY_S).is_down()) {
        camera->process_keyboard(engine::graphics::Camera::Movement::BACKWARD, dt);
    }
    if (platform->key(engine::platform::KeyId::KEY_A).is_down()) {
        camera->process_keyboard(engine::graphics::Camera::Movement::LEFT, dt);
    }
    if (platform->key(engine::platform::KeyId::KEY_D).is_down()) {
        camera->process_keyboard(engine::graphics::Camera::Movement::RIGHT, dt);
    }

    auto mouse = platform->mouse();
    camera->process_mouse_movement(mouse.dx, mouse.dy);
    //camera->process_mouse_scroll(mouse.scroll);
}

void MainController::poll_events() {
    const auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KEY_F1).state() == engine::platform::Key::State::JustPressed) {
        cursor_enabled = !cursor_enabled;
        platform->set_enable_cursor(cursor_enabled);
    }
}
