#include <bits/stdc++.h>
#include "../src/bambooFilter.hpp"
using u = unsigned;



int main(int argc, char* argv[]) {
  ofstream out("output/test2.txt");
  string pathToGenome = argv[1];
  u k = (u) stoi(argv[2]);
  u numOfAdditions = (u) stoi(argv[3]);

  ifstream file(pathToGenome);
  string header;
  getline(file, header);

  string line;
  string oversizedWindow = "";
  vector<string> kMers; 
  while (getline(file, line)) {
    oversizedWindow += line;
    if (oversizedWindow.length() < k)
      continue;
    kMers.push_back(oversizedWindow.substr(0, k));
    oversizedWindow.erase(0, 1);
  }
  const string bases = "ATGC";
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> dist(0, 3);
  
  vector<string> randomKmers;
  for (int i = 0; i < numOfAdditions; i++) {
    string kMer = "";
    for (int j = 0; j < k; j++) {
      kMer += bases[dist(gen)]; 
    }
    randomKmers.push_back(kMer);
  } 

  BambooFilter* bf = new BambooFilter(initialSegBitLength);
  out << "Insert rate: ";
  auto start_time = NowNanos();
  for (auto itm : kMers) {
    bf->bfInsert(itm.c_str());
  }
  out << ((kMers.size() * 1000.0) / static_cast<double>(NowNanos() - start_time)) << "\n";
  int positive = 0;
  int negative = 0;
  out << "Lookup rate: ";
  start_time = NowNanos();
  for (auto itm : randomKmers) {
    if (bf->bfLookUp(itm.c_str())) positive++;
    else negative++;
  }
  out << ((randomKmers.size() * 1000.0) / static_cast<double>(NowNanos() - start_time)) << "\n";
  out << "positives: " << positive << "\n";
  out << "negatives: " << negative << "\n";

  delete bf;
}
