#pragma once

#include <memory>

namespace edge::utl {
    class NonCopyable {
    protected:
        NonCopyable() = default;
        virtual ~NonCopyable() = default;

    public:
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
    };

    class NonMovable {
    protected:
        NonMovable() = default;
        virtual ~NonMovable() = default;

    public:
        NonMovable(NonMovable&&) noexcept = delete;
        NonMovable& operator=(NonMovable&&) noexcept = delete;
    };

    class NonCopyMovable : public NonCopyable, public NonMovable {
    protected:
        NonCopyMovable() = default;
        virtual ~NonCopyMovable() = default;
    };

    template <class _Ty>
    class Singleton : public NonCopyMovable {
    public:
        static inline const std::unique_ptr<_Ty>& GetInstance() {
            static std::unique_ptr<_Ty> _instance;
            if (!_instance)
                _instance = std::make_unique<_Ty>();
            return _instance;
        }
    };
}