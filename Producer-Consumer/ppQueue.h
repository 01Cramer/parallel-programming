#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>

template <class T>
class ppQueue
{
protected:

    std::queue<T> m_Queue;

    std::mutex m_Mutex;
    std::condition_variable m_ConditionVariable;

public:
    ppQueue() {};

    void EnqueueWait(const T& Data);
    T DequeueWait();
};

template <class T>
void ppQueue<T>::EnqueueWait(const T& Data)
{
    std::unique_lock<std::mutex> LockManager(m_Mutex);
    m_Queue.push(Data);
    m_ConditionVariable.notify_all();
}

template <class T>
T ppQueue<T>::DequeueWait()
{
    T Data;
    std::unique_lock<std::mutex> LockManager(m_Mutex);
    while (m_Queue.size() <= 0) {
        m_ConditionVariable.wait(LockManager, [&] { return m_Queue.size() > 0; });
    }
    Data = m_Queue.front();
    m_Queue.pop();
    m_ConditionVariable.notify_all();
    return Data;
}
