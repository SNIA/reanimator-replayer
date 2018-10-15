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

std::mutex m;
std::condition_variable cv;
std::atomic<int> data(0);
bool processed = false;

class ComparePair {
public:
  bool operator()(std::pair<int, int> t1, std::pair<int, int> t2) {
    return t1.first >= t2.first;
  }
};

std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
                    ComparePair>
    pq;

std::unordered_map<std::thread::id, int> tidMap;

boost::barrier bar(3);

void worker_thread() {
  bar.wait();
  auto tid = tidMap[std::this_thread::get_id()];
  while (1) {
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [&] { return pq.empty() || pq.top().second == tid; });

    if (pq.empty()) {
      return;
    }

    auto top = pq.top().second;
    pq.pop();

    lk.unlock();
    cv.notify_all();

    data += top;

    // std::cout << "Worker thread " << std::this_thread::get_id()
    //           << " data " << data <<"\n";
  }
}

int main() {
  pq.push(std::make_pair(1, 1));
  pq.push(std::make_pair(2, 2));
  pq.push(std::make_pair(3, 1));
  pq.push(std::make_pair(4, 2));

  std::thread worker(worker_thread);
  tidMap[worker.get_id()] = 1;
  std::thread worker2(worker_thread);
  tidMap[worker2.get_id()] = 2;
  bar.wait();

  { std::lock_guard<std::mutex> lk(m); }
  cv.notify_all();

  {
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [] { return pq.empty(); });
  }

  worker.join();
  worker2.join();
  std::cout << "Back in main(), data = " << data << '\n';
}
