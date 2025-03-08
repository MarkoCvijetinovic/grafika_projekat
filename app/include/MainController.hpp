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

    void set_rotation(engine::resources::Shader *shader, int speed);

    void alter_star();

    unsigned int m_amount      = 2000;
    glm::mat4 *m_modelMatrices = nullptr;

    glm::vec3 m_starPos   = glm::vec3(-5.0f, 0.0f, -8.0f);
    glm::vec3 m_starColor = glm::vec3(1.0f, 0.9f, 0.6f);

    glm::vec3 m_csillaPos = glm::vec3(0.5f, 0.0f, -1.0f);
    int m_csillaSpeed     = 10000;

    bool m_bloom           = true;
    bool m_bloomKeyPressed = false;
    float m_exposure       = 1.5f;

    glm::vec3 m_spotLightColor = glm::vec3(0.7f, 0.7f, 0.7f);
    float m_terranScale        = 0.1f;
    float m_starLuminocity     = 1.8f;
    bool m_starKeyPressed      = false;
};

#endif //MAINCONTROLLER_HPP
