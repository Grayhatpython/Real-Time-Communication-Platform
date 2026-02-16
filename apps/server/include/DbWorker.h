#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "AuthService.h"

class DbWorker
{
public:
    using Task = std::function<void(AuthService&)>;

    void Initalize(std::string dbPath, std::string schemaPath);
    void Stop();
    void Enqueue(Task task);

    std::function<void(std::stop_token)> MakeDbTask();

private:
    void Run(std::stop_token st);

private:
    std::string _dbPath;
    std::string _schemaPath;

    std::atomic<bool>           _running{true};
    
    std::mutex                  _lock;
    std::condition_variable     _cv;
    
    std::queue<Task>            _taskQueue;
};