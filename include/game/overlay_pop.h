#pragma once

namespace game {

// Must match the `transition` durations in help.css, welcome.css, and scenes.css.
inline constexpr float kOverlayPopSeconds = 0.2f;

// One painted frame at scale 0, then scale 1: a CSS transition snaps on the first paint.
class OverlayPop {
public:
    bool open();
    bool close();
    bool tick(float dt);

    [[nodiscard]] bool is_open() const {
        return phase_ != Phase::Closed;
    }
    [[nodiscard]] bool closing() const {
        return phase_ == Phase::Closing;
    }
    [[nodiscard]] bool grown() const {
        return phase_ == Phase::Open;
    }
    [[nodiscard]] const char* scale() const {
        return grown() ? "scale(1)" : "scale(0)";
    }
    [[nodiscard]] const char* dim() const {
        return grown() ? "1" : "0";
    }

private:
    enum class Phase { Closed, Priming, Open, Closing };

    Phase phase_ = Phase::Closed;
    int prime_hold_ = 0;
    float close_elapsed_ = 0.f;
};

inline bool OverlayPop::open() {
    if (phase_ == Phase::Open || phase_ == Phase::Priming) {
        return false;
    }
    if (phase_ == Phase::Closing) {  // reopen mid-close: jump to full size
        phase_ = Phase::Open;
        return true;
    }
    phase_ = Phase::Priming;
    prime_hold_ = 1;
    return true;
}

inline bool OverlayPop::close() {
    if (phase_ == Phase::Closed || phase_ == Phase::Closing) {
        return false;
    }
    if (phase_ == Phase::Priming) {  // not grown yet: hide, don't reverse
        phase_ = Phase::Closed;
        prime_hold_ = 0;
        return true;
    }
    phase_ = Phase::Closing;
    close_elapsed_ = 0.f;
    return true;
}

inline bool OverlayPop::tick(float dt) {
    if (phase_ == Phase::Priming) {
        if (prime_hold_ > 0) {
            --prime_hold_;
            return false;
        }
        phase_ = Phase::Open;
        return true;
    }
    if (phase_ == Phase::Closing) {
        close_elapsed_ += dt > 0.f ? dt : 0.f;
        // Past the CSS duration: tick runs a frame ahead of the paint clock.
        if (close_elapsed_ >= kOverlayPopSeconds + 0.08f) {
            phase_ = Phase::Closed;
            return true;
        }
    }
    return false;
}

}
