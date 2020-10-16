/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
 */

// Copyright Umit

#include <atomic>
#include <boost/thread/barrier.hpp>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

// #define VERBOSE

std::mutex mtxQueue;
std::condition_variable cvEnter;
std::mutex mtxVector;
std::condition_variable cvExit;
std::mutex correctVector;
std::mutex mtxCurrent;

std::atomic<int> data(0);

class Syscalls {
public:
  int uniqueId;
  int pid;
  int startTime;
  int endTime;

  Syscalls(int id, int tid, int startTime, int endTime)
      : pid(tid), uniqueId(id), startTime(startTime), endTime(endTime) {}
};

class ComparePair {
public:
  bool operator()(Syscalls *t1, Syscalls *t2) {
    return t1->uniqueId >= t2->uniqueId;
  }
};

std::unordered_map<std::thread::id, int> tidMap;
std::priority_queue<Syscalls *, std::vector<Syscalls *>, ComparePair> pq;
std::vector<Syscalls *> currentRunning;

boost::barrier bar(3);

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(1, 10);

void worker_thread() {
  bar.wait();
  auto tid = tidMap[std::this_thread::get_id()];
  while (1) {
    std::unique_lock<std::mutex> lock(mtxQueue);
    cvEnter.wait(lock, [&] { return pq.empty() || (pq.top()->pid == tid); });

    if (pq.empty()) {
      return;
    }

    auto top = pq.top();
    auto topPid = top->pid;
    pq.pop();

    std::unique_lock<std::mutex> lockVector(mtxVector);
    cvExit.wait(lockVector, [top] {
      while (1) {
        bool counterExample = false;
        for (auto s : currentRunning) {
          if (s->endTime < top->startTime) {
#ifdef VERBOSE
            std::cout << tid << " " << top->startTime << " " << top->endTime
                      << " violates \n"
                      << s->pid << " " << s->startTime << " " << s->endTime
                      << "\n";
#endif
            // return false;
            counterExample = true;
            break;
          }
        }
        if (counterExample == false) {
          break;
        }
      }
      return true;
    });

    currentRunning.push_back(top);

    lock.unlock();
    cvEnter.notify_all();

    lockVector.unlock();
    cvExit.notify_all();

#ifdef VERBOSE
    std::cout << std::this_thread::get_id() << " " << topPid << " "
              << top->startTime << " " << top->endTime << " started"
              << "\n";
#endif
    if (tid == 2) {
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    // std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
    data += topPid;

    mtxCurrent.lock();
    auto pos = std::find(currentRunning.begin(), currentRunning.end(), top);
    currentRunning.erase(pos);
    mtxCurrent.unlock();

#ifdef VERBOSE
    std::cout << std::this_thread::get_id() << " " << topPid << " "
              << top->startTime << " " << top->endTime << " finished "
              << currentRunning.size() << "\n";
#else
    std::cout << topPid << " " << top->startTime << " " << top->endTime << "\n";
#endif
    cvExit.notify_all();
  }
}

int main() {
  pq.push(new Syscalls(1, 1, 1, 3));
  pq.push(new Syscalls(2, 2, 1, 2));
  pq.push(new Syscalls(3, 1, 3, 4));
  pq.push(new Syscalls(4, 2, 3, 4));
  pq.push(new Syscalls(5, 1, 5, 5));
  pq.push(new Syscalls(6, 2, 5, 6));
  pq.push(new Syscalls(7, 1, 5, 6));
  pq.push(new Syscalls(8, 2, 6, 8));

  std::thread worker(worker_thread);
  tidMap[worker.get_id()] = 1;
  std::thread worker2(worker_thread);
  tidMap[worker2.get_id()] = 2;
  bar.wait();

  // { std::lock_guard<std::mutex> lock(mtxQueue); }
  // cvEnter.notify_all();

  {
    std::unique_lock<std::mutex> lock(mtxQueue);
    cvEnter.wait(lock, [] { return pq.empty(); });
  }

  worker.join();
  worker2.join();
  std::cout << "Back in main(), data = " << data << '\n';
}
