// Copyright Umit

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/thread/barrier.hpp>

std::mutex mtxQueue;
std::mutex mtxVector;
std::condition_variable cv;
std::atomic<int> data(0);
bool processed = false;

class Syscalls {
 public:
  int uniqueId;
  int pid;
  int timeline;

  Syscalls(int id, int tid, int timeline)
      : pid(tid), uniqueId(id), timeline(timeline) {}
};

class ComparePair {
 public:
  bool operator()(Syscalls *t1, Syscalls *t2) {
    return t1->uniqueId >= t2->uniqueId;
  }
};

std::priority_queue<Syscalls *, std::vector<Syscalls *>, ComparePair> pq;
std::unordered_map<std::thread::id, int> tidMap;

boost::barrier bar(3);

void worker_thread() {
  bar.wait();
  auto tid = tidMap[std::this_thread::get_id()];
  while (1) {
    std::unique_lock<std::mutex> lock(mtxQueue);
    cv.wait(lock, [&] { return pq.empty() || (pq.top()->pid == tid); });

    if (pq.empty()) {
      return;
    }

    auto top = pq.top();
    auto topPid = top->pid;
    pq.pop();

    lock.unlock();
    cv.notify_all();

    data += topPid;
  }
}

int main() {
  pq.push(new Syscalls(1, 1, 1));
  pq.push(new Syscalls(2, 2, 1));
  pq.push(new Syscalls(3, 1, 2));
  pq.push(new Syscalls(4, 2, 3));

  std::thread worker(worker_thread);
  tidMap[worker.get_id()] = 1;
  std::thread worker2(worker_thread);
  tidMap[worker2.get_id()] = 2;
  bar.wait();

  { std::lock_guard<std::mutex> lock(mtxQueue); }
  cv.notify_all();

  {
    std::unique_lock<std::mutex> lock(mtxQueue);
    cv.wait(lock, [] { return pq.empty(); });
  }

  worker.join();
  worker2.join();
  std::cout << "Back in main(), data = " << data << '\n';
}
