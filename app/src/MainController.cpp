//
// Created by marko-cvijetinovic on 1/17/25.
//

#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>

#include "MainController.hpp"

#include <libs/assimp/code/AssetLib/3MF/3MFXmlTags.h>
#include <libs/glad/include/glad/glad.h>
#include <spdlog/spdlog.h>

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

    initialize_bloom();
    initialize_asteroids();
}

bool MainController::loop() {
    auto platform = get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KeyId::KEY_ESCAPE).is_down())
        return false;

    return true;
}

void MainController::draw_phoenix() {
    auto resources = get<engine::resources::ResourcesController>();
    auto phoenix   = resources->model("phoenix");
    auto shader    = resources->shader("planet");
    shader->use();

    glm::mat4 model = glm::mat4(1.0f);
    model           = translate(model, glm::vec3(-2.0f, 0.0f, -3.0f));
    model           = scale(model, glm::vec3(0.8f));
    model           = rotate(model, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    shader->set_mat4("model", model);

    phoenix->draw(shader);
}

void MainController::draw_spaceship() {
    auto resources = get<engine::resources::ResourcesController>();
    auto spaceship = resources->model("spaceship");
    auto shader    = resources->shader("planet");
    shader->use();

    glm::mat4 model = glm::mat4(1.0f);
    model           = translate(model, marsPos + glm::vec3(0.1f, 0.2f, 1.4f));
    model           = scale(model, glm::vec3(0.001f));
    model           = rotate(model, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    shader->set_mat4("model", model);

    spaceship->draw(shader);
}

void MainController::configure_planet() {
    auto resources = get<engine::resources::ResourcesController>();
    auto shader    = resources->shader("planet");
    shader->use();

    auto graphics = get<engine::graphics::GraphicsController>();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());

    auto camera = graphics->camera();
    shader->set_vec3("viewPos", camera->Position);

    set_spot_light(shader);
    set_star_light(shader);
    //set_rotation(shader);
}

void MainController::draw_csilla() {
    auto resources = get<engine::resources::ResourcesController>();
    auto mars      = resources->model("csilla");
    auto shader    = resources->shader("planet");
    shader->use();

    glm::mat4 model = glm::mat4(1.0f);
    model           = translate(model, marsPos);
    model           = scale(model, glm::vec3(0.1f));
    shader->set_mat4("model", model);

    mars->draw(shader);
}

void MainController::draw_terran() {
    auto resources = get<engine::resources::ResourcesController>();
    auto mars      = resources->model("terran");
    auto shader    = resources->shader("planet");
    shader->use();

    glm::mat4 model = glm::mat4(1.0f);
    model           = translate(model, glm::vec3(4.0f, 0.0f, -2.0f));
    model           = scale(model, glm::vec3(0.1f));
    shader->set_mat4("model", model);

    mars->draw(shader);
}

void MainController::draw_star() {
    auto resources = get<engine::resources::ResourcesController>();
    auto star      = resources->model("star");
    auto shader    = resources->shader("star");
    shader->use();

    auto graphics = get<engine::graphics::GraphicsController>();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    glm::mat4 model = glm::mat4(1.0f);
    model           = translate(model, starPos);
    model           = scale(model, glm::vec3(0.6f));
    shader->set_mat4("model", model);

    auto camera = graphics->camera();
    shader->set_vec3("viewPos", camera->Position);

    star->draw(shader);
}

void MainController::draw_asteroid() {
    auto resources = get<engine::resources::ResourcesController>();
    auto asteroid  = resources->model("asteroid");
    auto shader    = resources->shader("asteroid");
    shader->use();

    auto graphics = get<engine::graphics::GraphicsController>();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());

    set_star_light(shader);
    set_spot_light(shader);
    //set_rotation(shader);

    auto platform = get<engine::platform::PlatformController>();
    float angle   = fmod((platform->frame_time().current), 12000) / (12000.0f / 360);

    auto rotation = translate(glm::mat4(1.0f), marsPos);
    rotation      = rotate(rotation, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation      = translate(rotation, -marsPos);

    shader->set_mat4("moonRotation", rotation);

    auto camera = graphics->camera();
    shader->set_vec3("viewPos", camera->Position);

    for (unsigned int i = 0; i < asteroid->meshes().size(); i++) {
        glBindVertexArray(asteroid->meshes()[i].VAO());
        for (unsigned int j = 0; j < asteroid->meshes()[i].textures().size(); j++) {
            glActiveTexture(GL_TEXTURE0 + j);
            glBindTexture(GL_TEXTURE_2D, asteroid->meshes()[i].textures()[j]->id());
        }
        glDrawElementsInstanced(GL_TRIANGLES, asteroid->meshes()[i].num_of_indices(),
                                GL_UNSIGNED_INT,
                                0, amount);
        glBindVertexArray(0);
    }
}

void MainController::begin_draw() {
    engine::graphics::OpenGL::begin_bloom();

    engine::graphics::OpenGL::clear_buffers();

    configure_planet();
}

void MainController::draw_skybox() {
    auto resources = get<engine::resources::ResourcesController>();
    auto skybox    = resources->skybox("galaxy_skybox");
    auto shader    = resources->shader("skybox");
    auto graphics  = get<engine::graphics::GraphicsController>();
    graphics->draw_skybox(shader, skybox);
}

void MainController::draw() {
    draw_phoenix();
    draw_asteroid();
    draw_spaceship();
    draw_csilla();
    draw_terran();
    draw_skybox();
    draw_star();
}

void MainController::end_draw() {
    auto resources   = get<engine::resources::ResourcesController>();
    auto shaderBlur  = resources->shader("blur");
    auto shaderBloom = resources->shader("bloom");

    engine::graphics::OpenGL::end_bloom(shaderBlur, shaderBloom, bloom, exposure);

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

    if (platform->key(engine::platform::KeyId::KEY_SPACE).is_down() && !bloomKeyPressed) {
        bloom           = !bloom;
        bloomKeyPressed = true;
        spdlog::info("Bloom switched");
    }
    if (platform->key(engine::platform::KeyId::KEY_SPACE).is_up()) {
        bloomKeyPressed = false;
    }

    if (platform->key(engine::platform::KeyId::KEY_Q).is_down()) {
        spdlog::info("Exposure decreased");
        if (exposure > 0.0f)
            exposure -= 0.01f;
        else
            exposure = 0.0f;
    } else if (platform->key(engine::platform::KeyId::KEY_E).is_down()) {
        spdlog::info("Exposure increased");
        exposure += 0.01f;
    }
}

void MainController::initialize_asteroids() {
    modelMatrices = new glm::mat4[amount];
    auto platform = get<engine::platform::PlatformController>();
    srand(static_cast<unsigned int>(platform->frame_time().current)); // initialize random seed

    float radius = 1.0;
    float offset = 0.25f;
    for (unsigned int i = 0; i < amount; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model           = translate(model, marsPos);

        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle        = (float) i / (float) amount * 360.0f;
        float displacement = (rand() % (int) (2 * offset * 100)) / 100.0f - offset;
        float x            = sin(angle) * radius + displacement;
        displacement       = (rand() % (int) (2 * offset * 100)) / 100.0f - offset;
        float y            = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
        displacement       = (rand() % (int) (2 * offset * 100)) / 100.0f - offset;
        float z            = cos(angle) * radius + displacement;
        model              = glm::translate(model, glm::vec3(x, y, z));

        // 2. scale: Scale between 0.05 and 0.25f
        float scale = static_cast<float>((rand() % 20) / 2000.0 + 0.0025);
        model       = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = static_cast<float>((rand() % 360));
        model          = rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        // 4. now add to list of matrices
        modelMatrices[i] = model;
    }

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    auto resources = get<engine::resources::ResourcesController>();
    auto asteroid  = resources->model("asteroid");

    for (unsigned int i = 0; i < asteroid->meshes().size(); i++) {
        unsigned int VAO = asteroid->meshes()[i].VAO();
        glBindVertexArray(VAO);
        // set attribute pointers for matrix (4 times vec4)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) 0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }

}

void MainController::initialize_bloom() {
    auto resources   = get<engine::resources::ResourcesController>();
    auto platform    = get<engine::platform::PlatformController>();
    auto shaderBlur  = resources->shader("blur");
    auto shaderBloom = resources->shader("bloom");

    engine::graphics::OpenGL::initialize_bloom(platform->window()->width(),
                                               platform->window()->height(), shaderBlur, shaderBloom);

}

void MainController::set_spot_light(engine::resources::Shader *shader) {
    auto graphics = get<engine::graphics::GraphicsController>();
    auto camera   = graphics->camera();

    shader->set_vec3("light.position", camera->Position);
    shader->set_vec3("light.direction", camera->Front);
    shader->set_float("light.cutOff", glm::cos(glm::radians(10.5f)));
    shader->set_float("light.outerCutOff", glm::cos(glm::radians(12.5f)));

    // light properties
    shader->set_vec3("light.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
    // we configure the diffuse intensity slightly higher; the right lighting conditions differ with each lighting method and environment.
    // each environment and lighting type requires some tweaking to get the best out of your environment.
    shader->set_vec3("light.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
    shader->set_vec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    shader->set_float("light.constant", 1.0f);
    shader->set_float("light.linear", 0.35f);
    shader->set_float("light.quadratic", 0.44f);
}

void MainController::set_star_light(engine::resources::Shader *shader) {
    shader->set_vec3("lightPos", starPos);
    shader->set_vec3("lightColor", starColor);
}

void MainController::set_rotation(engine::resources::Shader *shader) {
    auto platform = get<engine::platform::PlatformController>();
    float angle   = fmod((platform->frame_time().current), 15000) / (15000.0f / 360);

    auto rotation = translate(glm::mat4(1.0f), starPos);
    rotation      = rotate(rotation, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation      = translate(rotation, -starPos);

    //marsPos = rotation * glm::vec4(marsPos, 1.0);

    shader->set_mat4("starRotation", rotation);
}

void MainController::poll_events() {
    const auto platform = get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KEY_F1).state() == engine::platform::Key::State::JustPressed) {
        cursor_enabled = !cursor_enabled;
        platform->set_enable_cursor(cursor_enabled);
    }
}
