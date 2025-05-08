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
  // potencijalna optimizacija, incijaliziraj odmah fiksnu veličinu vektora
  vector<Segment *> table;

  BambooFilter<T>(size_t segmentLength)
      : table(1UL << segmentLength, nullptr){};

  bool bfInsert(T item) {}
  bool bfLookUp(T item) {}
  bool bfErase(T item) {}

private:
  void bfExpand() {}
  void bfCompress() {}
};
int main() {}
