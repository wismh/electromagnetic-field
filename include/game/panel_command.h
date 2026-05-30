#pragma once

#include <engine/ui/command.h>

namespace game {

// Unbound until bind_to; can_execute() is false until then.
class PanelCommand final : public engine::ui::ICommand {
public:
    [[nodiscard]] bool can_execute() const override {
        return call_ != nullptr;
    }

    void execute() override {
        call_(object_);
    }

    template<class T, void (T::*Method)()>
    void bind_to(T& object) {
        object_ = &object;
        call_ = &Thunk<T, Method>::call;
    }

private:
    template<class T, void (T::*Method)()>
    struct Thunk {
        static void call(void* object) {
            (static_cast<T*>(object)->*Method)();
        }
    };

    void* object_ = nullptr;
    void (*call_)(void*) = nullptr;
};

}
