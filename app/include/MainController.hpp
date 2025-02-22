//
// Created by marko-cvijetinovic on 1/17/25.
//

#ifndef MAINCONTROLLER_HPP
#define MAINCONTROLLER_HPP

#include <engine/core/Engine.hpp>

class MainController : public engine::core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "MainController";
    }

private:
    void initialize() override;

    bool loop() override;

    void begin_draw() override;

    void draw() override;

    void end_draw() override;

    void update() override;

    void poll_events() override;

    void draw_skybox();

    void draw_phoenix();

    void draw_csilla();

    void draw_terran();

    void draw_spaceship();

    void configure_planet();

    void draw_star();

    void draw_asteroid();

    void update_camera();

    bool cursor_enabled{true};

    void initialize_asteroids();

    void initialize_bloom();

    void set_spot_light(engine::resources::Shader *shader);

    void set_star_light(engine::resources::Shader *shader);

    void set_rotation(engine::resources::Shader *shader);

    void renderQuad();

    unsigned int amount      = 2000;
    glm::mat4 *modelMatrices = nullptr;

    glm::vec3 starPos   = glm::vec3(-5.0f, 0.0f, -8.0f);
    glm::vec3 starColor = glm::vec3(1.0f, 0.9f, 0.6f);

    glm::vec3 marsPos = glm::vec3(1.0f, 0.0f, -1.0f);

    unsigned int hdrFBO;
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    unsigned int colorBuffers[2];

    unsigned int quadVAO = 0;
    unsigned int quadVBO;

    bool bloom           = true;
    bool bloomKeyPressed = false;
    float exposure       = 1.5f;
};

#endif //MAINCONTROLLER_HPP
