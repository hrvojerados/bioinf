#include <bits/stdc++.h>
#include <random>
#include <fstream>
#include <unordered_map>

#include "../common/random.h"
#include "../common/timing.h"
#include "../common/BOBHash.h"
#include "../src/bambooFilter.hpp"

int main() {
  BambooFilter *bfTest = new BambooFilter(initialSegBitLength);
  bool result;
  size_t add_count = 100000;
  ofstream out("output/test3.txt");


  vector<string> to_add, to_lookup;
  GenerateRandom64(add_count, to_add, to_lookup);


  BambooFilter *bbf = new BambooFilter(initialSegBitLength);
  unordered_map<string, bool> map;


  for (uint64_t added = 0; added < add_count; added++) {
      map[to_add[added]] = true;
      bbf->bfInsert(to_add[added].c_str());
  }
  
  unsigned falsePositives = 0;
  unsigned total = 0;
  for (uint64_t added = 0; added < add_count; added++) {
    if (map[to_lookup[added]]) {
      continue;
    } else {
      total++;
      if (bbf->bfLookUp(to_lookup[added].c_str())) falsePositives++;
    }
  }

  out << "false positive percentage: " << (((double) falsePositives) / ((double) total)) * 100 << "\n"; 

  delete bbf;
}
