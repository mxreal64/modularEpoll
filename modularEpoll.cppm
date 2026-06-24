module;
// Restrict messy global C headers to the global module fragment
#include <sys/epoll.h>
#include <unistd.h> 

export module modularEpoll;
import std;

namespace InternalEpoll {
    // Statically capture macro values safely inside the translation unit
    constexpr uint32_t tmp_in  = EPOLLIN;
    constexpr uint32_t tmp_out = EPOLLOUT;
    constexpr uint32_t tmp_err = EPOLLERR;
    constexpr uint32_t tmp_et  = EPOLLET;
    
    constexpr int ctl_add = EPOLL_CTL_ADD;
    constexpr int ctl_mod = EPOLL_CTL_MOD;
    constexpr int ctl_del = EPOLL_CTL_DEL;
}

// Purge them completely so they cannot pollute user space
#undef EPOLLIN
#undef EPOLLOUT
#undef EPOLLERR
#undef EPOLLET
#undef EPOLL_CTL_ADD
#undef EPOLL_CTL_MOD
#undef EPOLL_CTL_DEL

export namespace Async {

    // Strongly-typed flags mapping directly to system events
    enum class EventFlags : uint32_t {
        Read          = InternalEpoll::tmp_in,
        Write         = InternalEpoll::tmp_out,
        Error         = InternalEpoll::tmp_err,
        EdgeTriggered = InternalEpoll::tmp_et
    };

    // Bitwise operator stack for clean event mapping combinations
    constexpr EventFlags operator|(EventFlags a, EventFlags b) noexcept {
        return static_cast<EventFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }
    
    constexpr EventFlags operator&(EventFlags a, EventFlags b) noexcept {
        return static_cast<EventFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    constexpr EventFlags operator~(EventFlags a) noexcept {
        return static_cast<EventFlags>(~static_cast<uint32_t>(a));
    }

    constexpr bool has_flag(EventFlags mask, EventFlags flag) noexcept {
        return (static_cast<uint32_t>(mask) & static_cast<uint32_t>(flag)) != 0;
    }

    // A modern, macro-isolated equivalent to system event structs
    struct [[gnu::packed]] Event {
        EventFlags events;
        void* user_data;
    };

    class EventLoop {
    private:
        int epoll_fd_ = -1;

    public:
        EventLoop() {
            epoll_fd_ = ::epoll_create1(0);
            if (epoll_fd_ == -1) {
                throw std::runtime_error("Failed to create epoll instance.");
            }
        }

        ~EventLoop() {
            if (epoll_fd_ != -1) ::close(epoll_fd_);
        }

        // Enforce strict single-ownership rules
        EventLoop(const EventLoop&) = delete;
        EventLoop& operator=(const EventLoop&) = delete;
        
        // Clean move semantics using standard memory-state exchange logic
        EventLoop(EventLoop&& other) noexcept : epoll_fd_(std::exchange(other.epoll_fd_, -1)) {}
        
        EventLoop& operator=(EventLoop&& other) noexcept {
            if (this != &other) {
                if (epoll_fd_ != -1) ::close(epoll_fd_);
                epoll_fd_ = std::exchange(other.epoll_fd_, -1);
            }
            return *this;
        }

        [[nodiscard]] bool add_fd(int fd, EventFlags flags, void* user_data) noexcept {
            ::epoll_event ev{};
            ev.events = static_cast<uint32_t>(flags);
            ev.data.ptr = user_data; 
            return ::epoll_ctl(epoll_fd_, InternalEpoll::ctl_add, fd, &ev) == 0;
        }

        [[nodiscard]] bool modify_fd(int fd, EventFlags flags, void* user_data) noexcept {
            ::epoll_event ev{};
            ev.events = static_cast<uint32_t>(flags);
            ev.data.ptr = user_data; 
            return ::epoll_ctl(epoll_fd_, InternalEpoll::ctl_mod, fd, &ev) == 0;
        }

        [[nodiscard]] bool remove_fd(int fd) noexcept {
            return ::epoll_ctl(epoll_fd_, InternalEpoll::ctl_del, fd, nullptr) == 0;
        }

        // True zero-allocation, zero-copy high-performance waiting mechanism
        [[nodiscard]] int wait(std::chrono::milliseconds timeout, std::span<Event> event_buffer) noexcept {
            // Compile-time layout check ensures your struct maps 1:1 down into system memory alignment
            static_assert(sizeof(Event) == sizeof(::epoll_event), "Event layout mismatch with system epoll_event");

            int t_ms = (timeout.count() < 0) ? -1 : static_cast<int>(timeout.count());
            
            // Direct zero-copy pass-through: Kernel writes directly into user space memory
            return ::epoll_wait(
                epoll_fd_, 
                reinterpret_cast<::epoll_event*>(event_buffer.data()), 
                static_cast<int>(event_buffer.size()), 
                t_ms
            );
        }
    };
}
