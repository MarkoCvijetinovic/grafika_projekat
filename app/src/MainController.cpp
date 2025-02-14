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

void MainController::draw_phoenix_planet() {
    auto resources = get<engine::resources::ResourcesController>();
    auto planet    = resources->model("planet");
    auto shader    = resources->shader("planet");
    shader->use();

    auto graphics = get<engine::graphics::GraphicsController>();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    glm::mat4 model = glm::mat4(1.0f);
    model           = translate(model, glm::vec3(-2.0f, 0.0f, -3.0f));
    model           = scale(model, glm::vec3(0.6f));
    model           = rotate(model, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    shader->set_mat4("model", model);

    auto camera = graphics->camera();
    shader->set_vec3("viewPos", camera->Position);

    setSpotLight(shader);
    setStarLight(shader);

    planet->draw(shader);
}

void MainController::draw_spaceship() {
    auto resources = get<engine::resources::ResourcesController>();
    auto spaceship = resources->model("spaceship");
    auto shader    = resources->shader("planet");
    shader->use();

    auto graphics = get<engine::graphics::GraphicsController>();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    glm::mat4 model = glm::mat4(1.0f);
    model           = translate(model, marsPos + glm::vec3(0.1f, 0.2f, 1.4f));
    model           = scale(model, glm::vec3(0.001f));
    model           = rotate(model, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    shader->set_mat4("model", model);

    setStarLight(shader);

    auto camera = graphics->camera();
    shader->set_vec3("viewPos", camera->Position);

    spaceship->draw(shader);
}

void MainController::draw_mars() {
    auto resources = get<engine::resources::ResourcesController>();
    auto mars      = resources->model("mars");
    auto shader    = resources->shader("planet");
    shader->use();

    auto graphics = get<engine::graphics::GraphicsController>();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());
    glm::mat4 model = glm::mat4(1.0f);
    model           = translate(model, marsPos);
    model           = scale(model, glm::vec3(0.1f));
    model           = rotate(model, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    shader->set_mat4("model", model);

    auto camera = graphics->camera();
    shader->set_vec3("viewPos", camera->Position);

    setSpotLight(shader);
    setStarLight(shader);

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

    setStarLight(shader);
    setSpotLight(shader);

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
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    engine::graphics::OpenGL::clear_buffers();
}

void MainController::draw_skybox() {
    auto resources = get<engine::resources::ResourcesController>();
    auto skybox    = resources->skybox("galaxy_skybox");
    auto shader    = resources->shader("skybox");
    auto graphics  = get<engine::graphics::GraphicsController>();
    graphics->draw_skybox(shader, skybox);
}

void MainController::draw() {
    draw_phoenix_planet();
    draw_asteroid();
    draw_spaceship();
    draw_mars();
    draw_star();
    draw_skybox();
}

void MainController::end_draw() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto resources   = get<engine::resources::ResourcesController>();
    auto shaderBlur  = resources->shader("blur");
    auto shaderBloom = resources->shader("bloom");

    // 2. blur bright fragments with two-pass Gaussian Blur
    // --------------------------------------------------
    bool horizontal     = true, first_iteration = true;
    unsigned int amount = 10;
    shaderBlur->use();
    for (unsigned int i = 0; i < amount; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        shaderBlur->set_bool("horizontal", horizontal);
        glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
        // bind texture of other framebuffer (or scene if first iteration)
        renderQuad();
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
    // --------------------------------------------------------------------------------------------------------------------------
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderBloom->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
    shaderBloom->set_bool("bloom", bloom);
    shaderBloom->set_float("exposure", exposure);
    renderQuad();

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
    }
    if (platform->key(engine::platform::KeyId::KEY_SPACE).is_up()) {
        bloomKeyPressed = false;
    }

    if (platform->key(engine::platform::KeyId::KEY_Q).is_down()) {
        if (exposure > 0.0f)
            exposure -= 0.001f;
        else
            exposure = 0.0f;
    } else if (platform->key(engine::platform::KeyId::KEY_E).is_down()) {
        exposure += 0.001f;
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
    auto resources = get<engine::resources::ResourcesController>();
    auto platform  = get<engine::platform::PlatformController>();
    int SCR_WIDTH  = platform->window()->width();
    int SCR_HEIGHT = platform->window()->height();

    auto shaderBlur     = resources->shader("blur");
    auto shaderBloom    = resources->shader("bloom");
    auto shaderPlanet   = resources->shader("planet");
    auto shaderAsteroid = resources->shader("asteroid");

    // configure (floating point) framebuffers
    // ---------------------------------------
    //unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0,
                     GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        spdlog::error("Framebuffer not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    //unsigned int pingpongFBO[2];
    //unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            spdlog::error("Framebuffer not complete!");
    }

    //shaderPlanet->use();
    //shaderPlanet->set_int("texture_diffuse1", 0);
    //shaderAsteroid->use();
    //shaderAsteroid->set_int("texture_diffuse1", 0);
    shaderBlur->use();
    shaderBlur->set_int("image", 0);
    shaderBloom->use();
    shaderBloom->set_int("scene", 0);
    shaderBloom->set_int("bloomBlur", 1);

}

void MainController::setSpotLight(engine::resources::Shader *shader) {
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

void MainController::setStarLight(engine::resources::Shader *shader) {
    shader->set_vec3("lightPos", starPos);
    shader->set_vec3("lightColor", starColor);
}

void MainController::setRotation(engine::resources::Shader *shader) {
    auto platform = get<engine::platform::PlatformController>();
    float angle   = fmod((platform->frame_time().current), 15000) / (15000.0f / 360);

    auto rotation = translate(glm::mat4(1.0f), starPos);
    rotation      = rotate(rotation, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation      = translate(rotation, -starPos);

    shader->set_mat4("starRotation", rotation);
}

void MainController::poll_events() {
    const auto platform = get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KEY_F1).state() == engine::platform::Key::State::JustPressed) {
        cursor_enabled = !cursor_enabled;
        platform->set_enable_cursor(cursor_enabled);
    }
}

void MainController::renderQuad() {
    if (quadVAO == 0) {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
