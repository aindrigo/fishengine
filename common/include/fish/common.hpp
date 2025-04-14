#pragma once

#if defined(_WIN32)
#define FISH_WINDOWS
#if defined(_WIN64)
#define FISH_WINDOWS_64
#endif
#elif defined(__unix__)
#define FISH_UNIX
#if defined(__linux__)
#define FISH_LINUX
#elif defined(__APPLE__)
#define FISH_XNU
#endif
#else
#define FISH_UNIX
#define FISH_LINUX
#warning Could not find OS type. Defaulting to UNIX / Linux
#endif

#include <format> // IWYU pragma: keep
#include <stdexcept> // IWYU pragma: keep

#define FISH_ASSERT(expr, msg) static_cast<bool>(expr) ? void(0) : throw std::runtime_error("Assertion `" #expr "' failed: " msg)
#define FISH_ASSERTF(expr, msg, ...) static_cast<bool>(expr) ? void(0) : throw std::runtime_error(std::format("Assertion `" #expr "' failed: " msg, __VA_ARGS__))
