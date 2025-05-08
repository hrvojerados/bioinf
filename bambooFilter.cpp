#include <bits/stdc++.h>
#include <vector>

#define lb 8
#define ls 2

using namespace std;
using ull = unsigned long long;

using fingerprint = unsigned char; // ":/"
                                   //
class Segment {
public:
  fingerprint buckets[1 << lb][1 << lb];
};

class BambooFilter {
public:
  vector<Segment> table;
};
int main() {}
