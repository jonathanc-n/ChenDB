#pragma once

#include <algorithm>
#include <random>

class Random {
  private:
    enum : uint64_t {
        A = 16807
    };
  public:
    uint32_t Next() {
      uint64_t product = seed_ * A;
      
      seed_ = static_cast<uint32_t>((product >> 31) + (product & M));

      if (seed_ > M) {
        seed_ -= M;
      }
      return seed_;
    }
}