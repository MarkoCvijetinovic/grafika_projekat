//
// Created by marko-cvijetinovic on 1/17/25.
//

#include <cmath>
#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>

#include "MainController.hpp"

#include <future>
#include <chrono>
#include <thread>
#include <spdlog/spdlog.h>

#include "GUIController.hpp"

#include <random>

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

    initialize_bloom();
    initialize_asteroids();
}

bool MainController::loop() {
    auto platform = get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KeyId::KEY_ESCAPE).is_down()) return false;

    return true;
}

void MainController::draw_phoenix() {
    auto resources = get<engine::resources::ResourcesController>();
    auto phoenix = resources->model("phoenix");
    auto shader = resources->shader("planet");
    shader->use();

    glm::mat4 model = glm::mat4(1.0f);
    model = translate(model, glm::vec3(-2.0f, 0.0f, -3.0f));
    model = scale(model, glm::vec3(0.8f));
    model = rotate(model, glm::radians(-20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    shader->set_mat4("model", model);

    set_rotation(shader, 6000);

    phoenix->draw(shader);
}

void MainController::draw_spaceship() {
    auto resources = get<engine::resources::ResourcesController>();
    auto spaceship = resources->model("spaceship");
    auto shader = resources->shader("planet");
    shader->use();

    glm::mat4 model = glm::mat4(1.0f);
    model = translate(model, m_csillaPos + glm::vec3(0.1f, 0.2f, 1.4f));
    model = scale(model, glm::vec3(0.001f));
    model = rotate(model, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    shader->set_mat4("model", model);

    set_rotation(shader, m_csillaSpeed);

    spaceship->draw(shader);
}

void MainController::configure_planet() {
    auto resources = get<engine::resources::ResourcesController>();
    auto shader = resources->shader("planet");
    shader->use();

    auto graphics = get<engine::graphics::GraphicsController>();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());

    auto camera = graphics->camera();
    shader->set_vec3("viewPos", camera->Position);

    set_spot_light(shader);
    set_star_light(shader);
}

void MainController::draw_csilla() {
    auto resources = get<engine::resources::ResourcesController>();
    auto csilla = resources->model("csilla");
    auto shader = resources->shader("planet");
    shader->use();

    glm::mat4 model = glm::mat4(1.0f);
    model = translate(model, m_csillaPos);
    model = scale(model, glm::vec3(0.1f));
    shader->set_mat4("model", model);

    set_rotation(shader, m_csillaSpeed);

    shader->set_float("lightModifier", 1.3f);
    for (int i = 0; i < 4; i++) {
        model = glm::mat4(1.0f);
        model = translate(model, m_starPos + glm::vec3(2.5f - i % 2 * 5.0f, 0.0f, 2.5f - i / 2 * 5.0f));
        model = scale(model, glm::vec3(0.1f));
        model = rotate(model, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        shader->set_mat4("model", model);

        set_rotation(shader, 3000);

        csilla->draw(shader);
    }
    shader->set_float("lightModifier", 1.0f);
}

void MainController::draw_terran() {
    auto resources = get<engine::resources::ResourcesController>();
    auto mars = resources->model("terran");
    auto shader = resources->shader("planet");
    shader->use();

    glm::mat4 model = glm::mat4(1.0f);
    model = translate(model, glm::vec3(4.0f, 0.0f, -2.0f));
    model = scale(model, glm::vec3(m_terranScale));
    shader->set_mat4("model", model);

    set_rotation(shader, 18000);

    mars->draw(shader);
}

void MainController::draw_star() {
    auto resources = get<engine::resources::ResourcesController>();
    auto star = resources->model("star");
    auto shader = resources->shader("star");
    shader->use();

    auto graphics = get<engine::graphics::GraphicsController>();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    glm::mat4 model = glm::mat4(1.0f);
    model = translate(model, m_starPos);
    model = scale(model, glm::vec3(0.6f));
    shader->set_mat4("model", model);
    shader->set_float("luminocity", m_starLuminocity);

    auto camera = graphics->camera();
    shader->set_vec3("viewPos", camera->Position);

    star->draw(shader);
}

void MainController::draw_asteroid() {
    auto resources = get<engine::resources::ResourcesController>();
    auto asteroid = resources->model("asteroid");
    auto shader = resources->shader("asteroid");
    shader->use();

    auto graphics = get<engine::graphics::GraphicsController>();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());

    set_star_light(shader);
    set_spot_light(shader);
    set_rotation(shader, m_csillaSpeed);

    auto platform = get<engine::platform::PlatformController>();
    float angle = fmod((platform->frame_time().current), 3000) / (3000.0f / 360);

    auto rotation = translate(glm::mat4(1.0f), m_csillaPos);
    rotation = rotate(rotation, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = translate(rotation, -m_csillaPos);

    shader->set_mat4("moonRotation", rotation);

    auto camera = graphics->camera();
    shader->set_vec3("viewPos", camera->Position);

    engine::graphics::OpenGL::draw_instanced(asteroid, m_amount);
}

void MainController::begin_draw() {
    engine::graphics::OpenGL::begin_bloom();

    engine::graphics::OpenGL::clear_buffers();

    configure_planet();
}

void MainController::draw_skybox() {
    auto resources = get<engine::resources::ResourcesController>();
    auto skybox = resources->skybox("galaxy_skybox");
    auto shader = resources->shader("skybox");
    auto graphics = get<engine::graphics::GraphicsController>();
    graphics->draw_skybox(shader, skybox);
}

void MainController::draw() {
    configure_planet();
    draw_phoenix();
    draw_csilla();
    draw_asteroid();
    draw_spaceship();
    draw_terran();
    draw_star();
    draw_skybox();
    draw_star();
}

void MainController::end_draw() {
    auto resources = get<engine::resources::ResourcesController>();
    auto shaderBlur = resources->shader("blur");
    auto shaderBloom = resources->shader("bloom");

    engine::graphics::OpenGL::end_bloom(shaderBlur, shaderBloom, m_bloom, m_exposure);

    auto platform = get<engine::platform::PlatformController>();
    platform->swap_buffers();
}

void MainController::update() { update_camera(); }

void MainController::update_camera() {
    auto gui_controller = get<GUIController>();
    if (gui_controller->is_enabled()) return;

    auto platform = get<engine::platform::PlatformController>();
    auto graphics = get<engine::graphics::GraphicsController>();
    auto camera = graphics->camera();

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

    if (platform->key(engine::platform::KeyId::KEY_SPACE).is_down() && !m_bloomKeyPressed) {
        m_bloom = !m_bloom;
        m_bloomKeyPressed = true;
        spdlog::info("Bloom switched");
    }
    if (platform->key(engine::platform::KeyId::KEY_SPACE).is_up()) { m_bloomKeyPressed = false; }

    if (platform->key(engine::platform::KeyId::KEY_Q).is_down()) {
        spdlog::info("Exposure decreased");
        if (m_exposure > 0.0f) m_exposure -= 0.01f;
        else m_exposure = 0.0f;
    } else if (platform->key(engine::platform::KeyId::KEY_E).is_down()) {
        spdlog::info("Exposure increased");
        m_exposure += 0.01f;
    }
}

void MainController::initialize_asteroids() {
    modelMatrices = new glm::mat4[amount];
    auto platform = get<engine::platform::PlatformController>();
    srand(static_cast<unsigned int>(platform->frame_time().current));// initialize random seed

    for (unsigned int i = 0; i < m_amount; i++) {
        float offset = 0.25f;
        float radius = 1.0;
        auto model = glm::mat4(1.0f);
        model = translate(model, m_csillaPos);

        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution dis_offset(-offset, offset);
        static std::uniform_real_distribution dis_scale(0.003f, 0.015f);
        static std::uniform_real_distribution dis_rotation(0.0f, 360.0f);

        // 1. Translation: displace along a circle with 'radius' in range [-offset, offset]
        float angle = static_cast<float>(i) / static_cast<float>(m_amount) * 360.0f;
        float displacement = dis_offset(gen);
        float x = std::sin(glm::radians(angle)) * radius + displacement;
        displacement = dis_offset(gen);
        float y = displacement * 0.4f;// Keep height smaller compared to width
        displacement = dis_offset(gen);
        float z = std::cos(glm::radians(angle)) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2. Scale: Scale between 0.05 and 0.25
        float scale = dis_scale(gen);
        model = glm::scale(model, glm::vec3(scale));

        // 3. Rotation: Add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = dis_rotation(gen);
        model = glm::rotate(model, glm::radians(rotAngle), glm::vec3(0.4f, 0.6f, 0.8f));

        // 4. now add to list of matrices
        m_modelMatrices[i] = model;
    }

    auto resources = get<engine::resources::ResourcesController>();
    auto asteroid = resources->model("asteroid");

    engine::graphics::OpenGL::initialize_instancing(asteroid, m_modelMatrices, m_amount);
}

void MainController::initialize_bloom() {
    auto resources = get<engine::resources::ResourcesController>();
    auto platform = get<engine::platform::PlatformController>();

    auto shaderBlur = resources->shader("blur");
    auto shaderBloom = resources->shader("bloom");

    engine::graphics::OpenGL::initialize_bloom(platform->window()->width(), platform->window()->height(), shaderBlur,
                                               shaderBloom);
}

void MainController::set_spot_light(engine::resources::Shader *shader) {
    auto graphics = get<engine::graphics::GraphicsController>();
    auto camera = graphics->camera();

    shader->set_vec3("light.position", camera->Position);
    shader->set_vec3("light.direction", camera->Front);
    shader->set_float("light.cutOff", glm::cos(glm::radians(10.5f)));
    shader->set_float("light.outerCutOff", glm::cos(glm::radians(12.5f)));

    // light properties
    shader->set_vec3("light.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
    // we configure the diffuse intensity slightly higher; the right lighting conditions differ with each lighting method and environment.
    // each environment and lighting type requires some tweaking to get the best out of your environment.
    shader->set_vec3("light.diffuse", m_spotLightColor);
    shader->set_vec3("light.specular", m_spotLightColor);
    shader->set_float("light.constant", 1.0f);
    shader->set_float("light.linear", 0.35f);
    shader->set_float("light.quadratic", 0.44f);
}

void MainController::set_star_light(engine::resources::Shader *shader) {
    shader->set_vec3("lightPos", m_starPos);
    shader->set_vec3("lightColor", m_starColor);
}

void MainController::set_rotation(engine::resources::Shader *shader, int speed) {
    auto platform = get<engine::platform::PlatformController>();

    float angle = fmod((platform->frame_time().current), speed) / (speed / 360.0);

    auto rotation = translate(glm::mat4(1.0f), m_starPos);
    rotation = rotate(rotation, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = translate(rotation, -m_starPos);

    shader->set_mat4("starRotation", rotation);
    //shader->set_mat4("starRotation", glm::mat4(1.0f));
}

void MainController::poll_events() {
    const auto platform = get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KEY_F1).state() == engine::platform::Key::State::JustPressed) {
        cursor_enabled = !cursor_enabled;
        platform->set_enable_cursor(cursor_enabled);
    }

    if (platform->key(engine::platform::KeyId::KEY_SPACE).is_down() && !m_bloomKeyPressed) {
        m_bloom = !m_bloom;
        m_bloomKeyPressed = true;
    }
    if (platform->key(engine::platform::KeyId::KEY_SPACE).is_up()) { m_bloomKeyPressed = false; }

    if (platform->key(engine::platform::KeyId::KEY_Q).is_down()) {
        if (m_exposure > 0.0f) m_exposure -= 0.01f;
        else m_exposure = 0.0f;
    } else if (platform->key(engine::platform::KeyId::KEY_E).is_down()) { m_exposure += 0.01f; }

    if (platform->key(engine::platform::KeyId::KEY_J).is_down()) { m_spotLightColor[0] += 0.02f; }
    if (platform->key(engine::platform::KeyId::KEY_K).is_down()) { m_spotLightColor[1] += 0.02f; }
    if (platform->key(engine::platform::KeyId::KEY_L).is_down()) { m_spotLightColor[2] += 0.02f; }
    if (platform->key(engine::platform::KeyId::KEY_I).is_down()) { m_spotLightColor[0] = std::max(m_spotLightColor[0] - 0.02f, 0.0f); }
    if (platform->key(engine::platform::KeyId::KEY_O).is_down()) { m_spotLightColor[1] = std::max(m_spotLightColor[1] - 0.02f, 0.0f); }
    if (platform->key(engine::platform::KeyId::KEY_P).is_down()) { m_spotLightColor[2] = std::max(m_spotLightColor[2] - 0.02f, 0.0f); }

    if (platform->key(engine::platform::KeyId::KEY_T).is_down() && !m_starKeyPressed) {
        std::thread(&MainController::alter_star, this).detach();
        m_starKeyPressed = true;
    }
    if (platform->key(engine::platform::KeyId::KEY_T).is_up()) { m_starKeyPressed = false; }
}

void MainController::alter_star() {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    m_starLuminocity *= 1.5f;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    m_starLuminocity /= 1.5f;
    m_terranScale *= 2.0f;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    m_terranScale /= 2.0f;
}
