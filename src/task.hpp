// src/task.hpp
#pragma once

#include <future>
#include <thread>
#include <utility>
#include <exception>

template<typename T>
class Task {
public:
    Task() = default;

    // launch of assynchrоnic computation in the thread
    template<typename F>
    static Task start(F&& func) {
        Task t;
        t._future = t._promise.get_future();
        t._thread = std::thread(
            [p = std::move(t._promise), f = std::forward<F>(func)]() mutable {
                try {
                    p.set_value(f());
                } catch (...) {
                    try {
                        p.set_exception(std::current_exception());
                    } catch (...) {
                        // ignoring of double errors
                    }
                }
            });
        return t;
    }

    Task(Task&& other) noexcept
        : _thread(std::move(other._thread))
        , _promise()
        , _future(std::move(other._future)) {}

    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (_thread.joinable()) _thread.join();
            _thread = std::move(other._thread);
            _future = std::move(other._future);
        }
        return *this;
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    ~Task() {
        if (_thread.joinable()) _thread.join();
    }

    T get() {
        if (_thread.joinable()) _thread.join();
        return _future.get();
    }

private:
    std::thread _thread;
    std::promise<T> _promise;
    std::future<T> _future;
};