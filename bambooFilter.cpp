#include <bits/stdc++.h>
#include <cstddef>
#include <vector>

#define bucketSize 4
#define bucketLength 12
#define segLength 4

using namespace std;
using ull = unsigned long long;
using ul = unsigned long int; // ":/"

class Segment {
public:
  ul buckets[1UL << bucketLength][bucketSize];
  Segment *overflow;
};

template <typename T> class BambooFilter {
public:
  vector<Segment *> table;

  BambooFilter<T>(size_t segmentLength)
      : table(1UL << segmentLength, nullptr), segmentLength(segmentLength){};

  bool bfInsert(T item) {}
  bool bfLookUp(T item) {}
  bool bfErase(T item) {}

private:
  int segmentLength;

  void bfExpand() {
    for (int i = 0; i < (1 << segmentLength); i++) {
    }
  }
  void bfCompress() {}
};
int main() {}
