#pragma once

#include <kf/aliases.hpp>
#include <kf/sys.hpp>
#include <kf/tools/meta/Singleton.hpp>

#include "djc/Periphery.hpp"


namespace djc {

struct PongGame : kf::sys::Behavior, kf::tools::Singleton<PongGame> {
    friend struct Singleton<PongGame>;

private:
    using Canvas = kf::gfx::Canvas;

    struct Wall {
        enum class Side { Left, Right };

    private:

        kf::Pixel size{0}, range{0};
        kf::Pixel x{0}, y{0};

    public:

        void update(kf::f32 v) {
            y = range;
            y /= 2;
            y = static_cast<kf::Pixel>(
                y - static_cast<kf::Pixel>(v * kf::f32(range) * 0.5f)
            );
        }

        void render(Canvas &canvas) const {
            canvas.line(x, y, x, static_cast<kf::Pixel>(y + size));
        }

        void reset(const Canvas &canvas, Side side) {
            size = static_cast<kf::Pixel>(canvas.height() / 4);
            range = static_cast<kf::Pixel>(canvas.height() - size);

            x = static_cast<kf::Pixel>(canvas.width() / 20);
            if (Side::Right == side) {
                x = static_cast<kf::Pixel>(canvas.width() - x);
            }

            y = canvas.centerY();
        }

    };

    struct Ball {
    private:
        kf::Pixel radius{0};
        kf::Pixel x{0}, y{0};
        kf::Pixel speed_x{0}, speed_y{0};
    public:
        void update(const Canvas &canvas) {
            x = static_cast<kf::Pixel>(x + speed_x);
            y = static_cast<kf::Pixel>(y + speed_y);

            if (isOutOfCanvasX(canvas)) {
                speed_x = static_cast<kf::Pixel>(-speed_x);
            }

            if (isOutOfCanvasY(canvas)) {
                speed_y = static_cast<kf::Pixel>(-speed_y);
            }
        }

        void render(Canvas &canvas) const {
            canvas.dot(x, y);
            canvas.circle(x, y, radius, Canvas::Mode::FillBorder);
        }

        void reset(const Canvas &canvas) {
            radius = static_cast<kf::Pixel>(std::min(canvas.width(), canvas.height()) / 6);

            x = canvas.centerX();
            y = canvas.centerY();

            speed_x = 1;
            speed_y = 1;
        }

    private:
        [[nodiscard]] bool isOutOfCanvasX(const Canvas &canvas) const {
            return x < radius or x + radius > canvas.maxX();
        }

        [[nodiscard]] bool isOutOfCanvasY(const Canvas &canvas) const {
            return y < radius or y + radius > canvas.maxY();
        }

    };

    Wall left_wall{};
    Wall right_wall{};
    Ball ball{};

    Canvas scene_canvas;

public:
    void onEntry() override {
        ball.reset(scene_canvas);
        left_wall.reset(scene_canvas, Wall::Side::Left);
        right_wall.reset(scene_canvas, Wall::Side::Right);
    }

    void updateLayout(Canvas &root) override {
        scene_canvas = root;
    }

//    void display() override {
//        Behavior::display();
//
//        scene_canvas.line(0, 0, scene_canvas.maxX(), 0);
//        scene_canvas.line(0, scene_canvas.maxY(), scene_canvas.maxX(), scene_canvas.maxY());
//
//        ball.render(scene_canvas);
//        left_wall.render(scene_canvas);
//        right_wall.render(scene_canvas);
//    }

    void update() override {
        auto &periphery = Periphery::instance();

        ball.update(scene_canvas);

        left_wall.update(periphery.left_joystick.axis_y.read());
        right_wall.update(periphery.right_joystick.axis_y.read());
    }
};

}// namespace djc
