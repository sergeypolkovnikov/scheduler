#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include <algorithm>
#include <ranges>
#include "scheduler.h"

TEST_CASE("stop", "[common]") {
    size_t call_count = 0;
    auto thread = std::jthread([&](std::stop_token st) {
        scheduler::SyncScheduler sh{ st };
        sh.every(std::chrono::milliseconds{ 2 }, [&]() { ++call_count; });
        sh.run();
        });
    const auto start = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
    thread = std::jthread{};
    const auto end = std::chrono::steady_clock::now();
    REQUIRE(call_count == 1);
    REQUIRE(end - start < std::chrono::milliseconds{ 3 });
}

TEST_CASE("empty", "[common]") {
    std::stop_source ss;
    scheduler::SyncScheduler sh{ ss.get_token() };
    sh.run(); // it shouldn't block. Immedeatly stop
}

TEST_CASE("seq", "[common]") {
    size_t magic_num = 0;
    auto thread = std::jthread([&](std::stop_token st) {
        scheduler::SyncScheduler sh{ st };
        sh.every(std::chrono::milliseconds{ 2 }, [&]() { ++magic_num; });
        sh.every(std::chrono::milliseconds{ 2 }, [&]() { magic_num *= 2; });
        sh.run();
        });
    std::this_thread::sleep_for(std::chrono::milliseconds{ 3 });
    thread = std::jthread{};
    REQUIRE(magic_num == 6);
}
