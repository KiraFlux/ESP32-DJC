#pragma once

#include <kf/aliases.hpp>
#include <kf/sys.hpp>
#include <kf/tools/meta/Singleton.hpp>

#include "djc/Periphery.hpp"

namespace djc {

struct PongGame : kf::sys::Behavior, kf::tools::Singleton<PongGame> {
    friend struct Singleton<PongGame>;

private:
    struct Wall {

        enum class Side {
            Left,
            Right,
        };

        kf::Joystick &joy;
        kf::Pixel size{0}, range{0};
        kf::Pixel x{0}, y{0};

        void update(const kf::gfx::Canvas &canvas) {
            y = range;
            y /= 2;
            y -= range * (joy.axis_y.read() * 0.5);
        }

        void render(kf::gfx::Canvas &canvas) {
            canvas.line(x, y, x, y + size);
        }

        void reset(const kf::gfx::Canvas &canvas, Side side) {
            size = canvas.height() / 4;
            range = canvas.height() - size;
            y = canvas.centerY();
            x = 6;

            if (Side::Right == side) {
                x = canvas.width() - x;
            }
        }

    } left_wall{Periphery::instance().left_joystick}, right_wall{Periphery::instance().right_joystick};

    struct Ball {
        kf::Pixel radius{0};
        kf::Pixel x{0}, y{0};
        kf::Pixel speed_x{0}, speed_y{0};

        void update(const kf::gfx::Canvas &canvas) {
            x += speed_x;
            y += speed_y;

            if (isOutOfCanvasX(canvas)) {
                speed_x = -speed_x;
            }

            if (isOutOfCanvasY(canvas)) {
                speed_y = -speed_y;
            }
        }

        void render(kf::gfx::Canvas &canvas) {
            canvas.dot(x, y);
            canvas.circle(x, y, radius, kf::gfx::Canvas::Mode::FillBorder);
        }

        void reset(const kf::gfx::Canvas &canvas) {
            radius = std::min(canvas.width(), canvas.height()) >> 4;

            x = canvas.centerX();
            y = canvas.centerY();

            speed_x = 1;
            speed_y = 1;
        }

    private:
        bool isOutOfCanvasX(const kf::gfx::Canvas &canvas) const {
            return x < radius or x + radius > canvas.maxX();
        }

        bool isOutOfCanvasY(const kf::gfx::Canvas &canvas) const {
            return y < radius or y + radius > canvas.maxY();
        }

    } ball{};

    kf::gfx::Canvas canvas;

public:
    void onEntry() override {
        ball.reset(canvas);
        left_wall.reset(canvas, Wall::Side::Left);
        right_wall.reset(canvas, Wall::Side::Right);
    }

    void updateLayout(kf::gfx::Canvas &root) override {
        auto [up, down] = root.splitVertically<2>({1, 7});

        canvas = down;
    }

    void display() override {
        Behavior::display();
        
        canvas.line(0, 0, canvas.maxX(), 0);
        canvas.line(0, canvas.maxY(), canvas.maxX(), canvas.maxY());

        ball.render(canvas);
        left_wall.render(canvas);
        right_wall.render(canvas);
    }

    void update() override {
        ball.update(canvas);
        left_wall.update(canvas);
        right_wall.update(canvas);
    }
};

}// namespace djc
