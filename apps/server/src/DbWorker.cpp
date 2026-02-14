#include "Pch.h"
#include "DbWorker.h"
#include "AuthDb.h"

#include "engine/ThreadManager.h"

 void DbWorker::Initalize(std::string dbPath, std::string schemaPath)
{
    _dbPath = std::move(dbPath);
    _schemaPath = std::move(schemaPath);
}

void DbWorker::Stop()
{
    {
        std::lock_guard<std::mutex> lock(_lock);
        _running.store(false, std::memory_order_release);
    }

    _cv.notify_one();
}

void DbWorker::Enqueue(Task task)
{
    {
        std::lock_guard<std::mutex> lock(_lock);
        _taskQueue.push(std::move(task));
    }

    _cv.notify_one();
}

void DbWorker::Run(std::stop_token st)
{
    // DB 스레드 전용 init
    AuthDb authDb;
    authDb.Initialize(_dbPath, _schemaPath);
    AuthService auth(authDb.GetDBHandle());

    while (_running.load(std::memory_order_acquire) && st.stop_requested() == false)
    {
        while(true)
        {
            Task task;

            {
                std::unique_lock<std::mutex> lock(_lock);
                _cv.wait(lock, [&]{ return _taskQueue.empty() == false || _running == false; });
                if (_running == false && _taskQueue.empty())
                    break;

                task = std::move(_taskQueue.front());
                _taskQueue.pop();
            }

            // 작업 실행(실패는 작업 내부에서 처리)
            task(auth);
        }
    }
}

std::function<void(std::stop_token)> DbWorker::MakeDbTask()
{
    return [this](std::stop_token st) {
        Run(st);
    };
}