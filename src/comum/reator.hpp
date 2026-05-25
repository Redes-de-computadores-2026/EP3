#pragma once
#include <functional>
#include <cstdint>
#include <map>
#include <queue>
#include <chrono>

struct TimerEntry {
    std::chrono::steady_clock::time_point deadline;
    std::function<void()> callback;
    bool operator>(const TimerEntry&) const;
};

class Reator {
    std::map<int, std::function<void()>> fds_;
    std::priority_queue<TimerEntry, std::vector<TimerEntry>, std::greater<TimerEntry>> fila_timers_;
    bool executando_ = false;
public:

    void registrar_fd(int fd, std::function<void()> callback);
    void desregistrar_fd(int fd);
    void agendar(uint32_t delay_ms, std::function<void()> callback);
    void executar();
    void parar();
};
