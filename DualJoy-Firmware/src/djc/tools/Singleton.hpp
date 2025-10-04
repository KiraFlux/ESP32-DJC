#pragma once

namespace djc {

template<typename T> struct Singleton {

    static T &instance() {
        static T instance{};
        return instance;
    }

    Singleton(const Singleton &) = delete;

protected:
    Singleton() = default;

    ~Singleton() = default;
};

}// namespace djc
