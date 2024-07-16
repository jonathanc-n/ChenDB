#pragma once

#include <algorithm>
#include <random>

class Random {
  private:
    enum : uint64_t {
        A = 16807
    };
    uint32_t seed_;

  public:
    uint32_t Next() {
      uint64_t product = seed_ * A;
      
      seed_ = static_cast<uint32_t>((product >> 31) + (product & M));

      if (seed_ > M) {
        seed_ -= M;
      }
      return seed_;
    }

    // Returns a Random instance for use by the current thread without 
    // additional locking
    static Random* GetTLSInstance();
}