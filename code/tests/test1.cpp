#include <bits/stdc++.h>
#include <random>
#include <fstream>

#include "../common/random.h"
#include "../common/timing.h"
#include "../common/BOBHash.h"
#include "../src/bambooFilter.hpp"

int main(int argc, char* argv[]) {

  BambooFilter *bfTest = new BambooFilter(initialSegBitLength);
  bool result;
  size_t add_count = stoi(argv[1]);
  ofstream out("output/test1.txt");

  out << "Prepare..." << endl;

  vector<string> to_add, to_lookup;
  GenerateRandom64(add_count, to_add, to_lookup);

  out << "Begin test" << endl;

  BambooFilter *bbf = new BambooFilter(initialSegBitLength);
  
  out << "Insertion rate:\n";
  auto start_time = NowNanos();

  for (uint64_t added = 0; added < add_count; added++) {

      bbf->bfInsert(to_add[added].c_str());
  }
  out << ((add_count * 1000.0) / static_cast<double>(NowNanos() - start_time)) << endl;

  out << "Lookup rate:\n";
  start_time = NowNanos();
  for (uint64_t added = 0; added < add_count; added++)
  {
      if (!bbf->bfLookUp(to_add[added].c_str()))
      {
        u fingerprint, bucketIndex, segmentIndex;
        throw logic_error("False Negative");
      }
  }
  out << ((add_count * 1000.0) / static_cast<double>(NowNanos() - start_time)) << endl;
  
  delete bbf;
}
