/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <atomic>
#include <chrono>
#include <cinttypes>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <pthread.h>
#include <sched.h>
#include <sys/syscall.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {
auto constexpr RUNNING_TIME = std::chrono::milliseconds(1000);
}

static long GetTid()
{
    return syscall(SYS_gettid);
}

struct Semaphore {
public:
    explicit Semaphore(int count = 0) : count_(count) {}
    ~Semaphore() {}

    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore(Semaphore&&) = delete;
    Semaphore& operator=(Semaphore&&) = delete;

    void Down()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (count_ <= 0) {
            cv_.wait(lock);
        }
        count_--;
    }

    void Up()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        count_++;
        cv_.notify_one();
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    volatile int count_;
};

struct Controller {
    using Clock = std::chrono::steady_clock;

    static Controller& GetInstance()
    {
        static Controller instance;
        return instance;
    }

    void StartWorkers(int n)
    {
        running_ = true;
        printf("[%ld] start %d workers...\n", GetTid(), n);
        for (int i = 0; i < n; i++) {
            workerSems_.push_back(std::make_shared<Semaphore>());
            threads_.emplace_back(std::thread(&Controller::Work, this, i));
        }
        doneSem_ = std::make_shared<Semaphore>();
        workerSems_[0]->Up();
    }

    void SetSwitchCount(int count)
    {
        swtichCount_ = count;
    }

    void WaitForDone()
    {
        if (doneSem_) {
            doneSem_->Down();
        }
    }

    void StopWorkers(int n)
    {
        running_ = false;
        for (auto& sem : workerSems_) {
            sem->Up();
        }

        printf("[%ld] wating %d workers...\n", GetTid(), n);
        for (int i = 0; i < n; i++) {
            threads_[i].join();
        }
    }

private:
    void Work(int id)
    {
        std::string name = "worker-" + std::to_string(id);
        pthread_setname_np(pthread_self(), name.c_str());
        printf("[%ld] %s start!\n", GetTid(), name.c_str());

        cpu_set_t cpuSet;
        CPU_ZERO(&cpuSet);
        CPU_SET(id, &cpuSet);
        sched_setaffinity(GetTid(), sizeof(cpuSet), &cpuSet);

        while (running_) {
            auto sem = workerSems_[id];
            if (!sem) {
                break;
            }

            // wait for swith to this thread
            sem->Down();
            if (!running_) {
                break;
            }

            // busy loop
            uint64_t count = 0;
            auto stopTime = Clock::now() + RUNNING_TIME;
            while (Clock::now() < stopTime) {
                count++;
            }
            printf("[%ld] busy loop count = %" PRIu64 "!\n", GetTid(), count);
            if (--swtichCount_ == 0) {
                doneSem_->Up();
                break;
            }

            // wakeup next thread
            auto nextId = static_cast<unsigned int>(id + 1) % workerSems_.size();
            auto nextSem = workerSems_[nextId];
            if (nextSem) {
                nextSem->Up();
            }
        }
        printf("[%ld] %s exit!\n", GetTid(), name.c_str());
    }

private:
    std::vector<std::thread> threads_;
    std::vector<std::shared_ptr<Semaphore>> workerSems_;
    std::atomic<bool> running_ = false;
    std::atomic<int> swtichCount_ = false;
    std::shared_ptr<Semaphore> doneSem_;
};

int main(int argc, char* argv[])
{
    const int n = std::thread::hardware_concurrency();
    const int m = ((argc > 1) ? atoi(argv[1]) : n); // switch times
    if (n <= 1 || m <= 1) {
        printf("unexpected m or n: %d, %d\n", m, n);
        return 1;
    }

    pthread_setname_np(pthread_self(), "main");
    Controller::GetInstance().SetSwitchCount(m);
    Controller::GetInstance().StartWorkers(n);

    printf("[%ld] perform %d times switch...\n", GetTid(), m);
    Controller::GetInstance().WaitForDone();

    Controller::GetInstance().StopWorkers(n);
    printf("all %d workers done!\n", n);
    return 0;
}
