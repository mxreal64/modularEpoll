# modularEpoll
A zero-copy, zero-allocation C++23 module wrapper for Linux epoll. Enforces compile-time structural packing to feed kernel event notifications directly into isolated user space without heap allocation stutters or legacy macro leaks. pretty cool, right?
