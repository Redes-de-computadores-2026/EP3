#include "reator.hpp"
#include <algorithm>
#include <cstdint>
#include <sys/select.h>
#include <unistd.h>
#include <functional>
#include <vector>
#include <cerrno>
#include <iostream>
#include <chrono>

bool TimerEntry::operator>(const TimerEntry& outro) const {
    return deadline > outro.deadline;
}

void Reator::registrar_fd(int fd, std::function<void()> callback) {
    fds_[fd] = callback;
}

void Reator::desregistrar_fd(int fd) {
    fds_.erase(fd);
}

void Reator::parar() {
    executando_ = false;
}

void Reator::agendar(uint32_t delay_ms, std::function<void()> callback) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay_ms);
    TimerEntry te;
    te.deadline = deadline;
    te.callback = callback;
    fila_timers_.push(te);
    return;
}

void Reator::executar() {
    executando_ = true;
    while (executando_) {
        fd_set rfds;
        FD_ZERO(&rfds);
        int max_fd = -1;
        
        for (auto const& [fd, callback] : fds_) {
            FD_SET(fd, &rfds);
            max_fd = std::max(max_fd, fd);
        }

        timeval tv;
        bool fila_vazia = fila_timers_.empty();
        if (!fila_vazia) {
            auto proxima_deadline = fila_timers_.top().deadline - std::chrono::steady_clock::now();
            if (proxima_deadline.count() < 0) {
                tv.tv_sec = 0;
                tv.tv_usec = 0;
            } else {
                auto us_total = std::chrono::duration_cast<std::chrono::microseconds>(proxima_deadline).count();
                tv.tv_sec = us_total / 1'000'000;
                tv.tv_usec = us_total % 1'000'000;
            }
        }
        int r = select(max_fd + 1, &rfds, nullptr, nullptr, fila_vazia ? nullptr : &tv);
        if (r < 0) {
            if (errno == EINTR) continue;
            else {
                std::cerr << "Erro no select" << std::endl;
                break;
            }
        }
        auto agora = std::chrono::steady_clock::now();
        while (!fila_timers_.empty() && fila_timers_.top().deadline <= agora) {
            auto cb = fila_timers_.top().callback;
            fila_timers_.pop();
            cb();
        }

        // esse código é necessário executar o callback de algum file descriptor aqui 
        // pode mutar o map de fds_, então primeiro copiamos a lista dos que devem ser executados e usamos abaixo
        std::vector<int> prontos;
        for (auto const& [fd, callback] : fds_) {
            if(FD_ISSET(fd, &rfds)) prontos.emplace_back(fd);
        }

        for (int fd_copia: prontos) {
            if (fds_.count(fd_copia) > 0) fds_[fd_copia]();
        }
    }

}
