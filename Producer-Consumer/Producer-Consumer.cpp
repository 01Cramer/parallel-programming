#include <iostream>
#include <thread>
#include <future>
#include "ppQueue.h"

constexpr static int32_t TestArraySize = 24;

struct ppTestObject
{
    int32_t m_Idx;
    int32_t m_Array[TestArraySize];
};

int32_t ThreadFunctionProducer(ppQueue<ppTestObject*>* Queue, int32_t NumTestObjects)
{
    std::thread::id ThreadID = std::this_thread::get_id();
    std::cout << "THRD ID=" << ThreadID << " START\n";

    //production
    std::cout << "THRD ID=" << ThreadID << " production started\n";
    for (int32_t Idx = 0; Idx < NumTestObjects; Idx++)
    {
        ppTestObject* TestObject = new ppTestObject;
        TestObject->m_Idx = Idx;
        for (int32_t i = 0; i < TestArraySize; i++) { TestObject->m_Array[i] = Idx + i; }
        Queue->EnqueueWait(TestObject);
        std::cout << "THRD ID=" << ThreadID << " enqueued " << Idx << '\n';
    }

    //signal end
    ppTestObject* LastObject = new ppTestObject;
    LastObject->m_Idx = -1;
    Queue->EnqueueWait(LastObject);

    std::cout << "THRD ID=" << ThreadID << " FINISH\n";
    return EXIT_SUCCESS;
}

int32_t ThreadFunctionConsumer(ppQueue<ppTestObject*>* Queue)
{
    std::thread::id ThreadID = std::this_thread::get_id();
    std::cout << "THRD ID=" << ThreadID << " START\n";

    std::cout << "THRD ID=" << ThreadID << " consumption started\n";
    int32_t Counter = 0;
    while (1)
    {
        ppTestObject* TestObject = Queue->DequeueWait();
        int32_t Idx = TestObject->m_Idx;
        delete TestObject;
        std::cout << "THRD ID=" << ThreadID << " dequeued " << Idx << '\n';
        if (Idx == -1) { break; }
        if (Idx != Counter) {
            std::cout << "THRD ID=" << ThreadID << " something is wrong Idx=" << Idx << " Cnt=" << Counter << '\n';
        }
        Counter++;
    }

    std::cout << "THRD ID=" << ThreadID << " FINISH\n";
    return Counter;
}

int32_t ProducerConsumer()
{
    ppQueue<ppTestObject*> Queue;

    std::cout << "MAIN Creating new thread\n";
    std::packaged_task<int32_t()> PackagedTaskProducer(std::bind(ThreadFunctionProducer, &Queue, 10));
    std::packaged_task<int32_t()> PackagedTaskConsumer(std::bind(ThreadFunctionConsumer, &Queue));
    std::future<int32_t> FutureProducer = PackagedTaskProducer.get_future();
    std::future<int32_t> FutureConsumer = PackagedTaskConsumer.get_future();
    std::thread ThreadProducer = std::thread(std::move(PackagedTaskProducer));
    std::thread ThreadConsumer = std::thread(std::move(PackagedTaskConsumer));

    while (1)
    {
        std::future_status Status = FutureConsumer.wait_for(std::chrono::milliseconds(500));
        if (Status == std::future_status::ready) { break; }
    }

    int32_t Result = FutureConsumer.get();
    ThreadProducer.join();
    ThreadConsumer.join();

    std::cout << "MAIN RESULT=" << Result << '\n';
    std::cout << "MAIN Done\n";
    return EXIT_SUCCESS;
}

int main()
{
    ProducerConsumer();
}
