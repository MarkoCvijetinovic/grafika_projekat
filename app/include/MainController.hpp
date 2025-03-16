//
// Created by marko-cvijetinovic on 1/17/25.
//

#ifndef MAINCONTROLLER_HPP
#define MAINCONTROLLER_HPP

class MainController : public engine::core::Controller {
public:
    [[nodiscard]] std::string_view name() const override {
        return "MainController";
    }

private:
    void initialize() override;

    bool loop() override;

    void begin_draw() override;

    void draw_skybox();

    void draw() override;

    void end_draw() override;

    void update() override;

    void poll_events() override;

    void draw_backpack();

    void update_camera();

    bool cursor_enabled{true};
};

#endif //MAINCONTROLLER_HPP
