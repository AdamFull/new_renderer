#pragma once

#include <chrono>

namespace edge::utl {
    class Stopwatch {
        using timepoint_t = std::chrono::steady_clock::time_point;
    public:
        Stopwatch() noexcept {
            sp = std::chrono::high_resolution_clock::now();
        }

        template<class _Ty>
        _Ty stop() {
            auto now = std::chrono::high_resolution_clock::now();
            auto secs = std::chrono::duration<_Ty>(now - sp).count();
            sp = now;
            return secs;
        }
    private:
        timepoint_t sp;
    };

    template<class _Ty, class _KTy>
    class FrameCounter {
    public:
        void update(_Ty dt) {
            if (accumulator > static_cast<_Ty>(1)) {
                mean_fps = counter;
                mean_time = accumulator / static_cast<_Ty>(counter);

                accumulator = static_cast<_Ty>(0);
                counter = static_cast<_KTy>(0);
            }
            else {
                accumulator += dt;
                counter += static_cast<_KTy>(1);
            }
        }

        const _Ty& delta_time() const {
            return mean_time;
        }

        const _KTy& fps() const {
            return mean_fps;
        }
    private:
        _Ty accumulator{};
        _KTy counter{};
        _Ty mean_time{};
        _KTy mean_fps{};
    };

    using fps_counter32 = FrameCounter<float, uint32_t>;
    using fps_counter64 = FrameCounter<double, uint64_t>;
}