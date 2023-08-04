# scheduler

## general

Manager for periodic tasks. Simple usage:

```
std::stop_source stop_src
scheduler::SyncScheduler scheduler { stop_src.get_token() };
scheduler.every(std::chrono::minutes {1}, []() { std::cout << "I'm alive" });
scheduler.run()
```

Method `run` blocks the current thread. It returns if no tasks were assigned or `stop_src.request_stop()` is called.
If `run` method shouldn't block the current thread. It can be wrapped in `jthread`

```
auto thread = std::jthread([&](std::stop_token st) {
        scheduler::SyncScheduler sh{ st };
        sh.every(std::chrono::milliseconds{ 2 }, []() { });
        sh.run();
        });
```