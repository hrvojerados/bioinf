#include <bits/stdc++.h>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <vector>

#include "common/random.h"
#include "common/timing.h"

#define bucketSize 4
#define bucketBitLength 12
#define fingerprintBitLength 16
#define maxMisses 8
#define expandConst 0.3
#define compressConst 0.4
#define N0 (1 << 12)

#define k1 (20 * (1 << bucketBitLength))
#define k2 (2 * bucketSize * (1 << bucketBitLength))

using namespace std;
using ull = unsigned long long;
using ul = unsigned long int; 

class Segment {
public:
//  ul numOfOverflownBuckets;
//  ul numOfElements;
  list<ul> buckets[1UL << bucketBitLength]; 
  list<ul> overflow[1UL << bucketBitLength];
//  Segment() : numOfOverflownBuckets(0), numOfElements(0) {};
  Segment() = default;
  ~Segment() = default;
};

template <typename T> class BambooFilter {
public:
  vector<Segment*> table;

  BambooFilter<T>(size_t segmentBitLength) 
    : table(1UL << segmentBitLength, nullptr),
    segmentBitLength(segmentBitLength),
    numOfItems(0),
    roundInd(0),
    nextSeg(0){};
  
  inline void getHashed(T item, ul &fingerprint, ul &bucketIndex, ul &segmentIndex) {
    const hash<T> h;
    fingerprint = (h(item) >> (32 - fingerprintBitLength)) % (1UL << fingerprintBitLength); // mod unnecessary?
    bucketIndex = h(item) % (1UL << bucketBitLength);
    segmentIndex = (h(item) >> bucketBitLength) % (1UL << (segmentBitLength + 1));
    if (segmentIndex >= table.size())
      segmentIndex -= (1UL << (segmentBitLength));
  }

  bool bfInsert(T item) {
    ul fingerprint, bucketIndex, segmentIndex;
    getHashed(item, fingerprint, bucketIndex, segmentIndex);
    return bfInsertHash(fingerprint, bucketIndex, segmentIndex);
  }
  bool bfInsertHash(ul fingerprint, ul bucketIndex, ul segmentIndex) {
    numOfItems++;
    if (this->table[segmentIndex] == nullptr) {
      table[segmentIndex] = new Segment();
    }
    Segment *seg = this->table[segmentIndex];
    // check if there's room in the first bucket, if yes great :D
    list<ul>::iterator it = seg->buckets[bucketIndex].begin();
    for (int i = 0; i < bucketSize; i++, it++) {
      if (it == seg->buckets[bucketIndex].end()) {
        seg->buckets[bucketIndex].push_back(fingerprint);
        //if we have too many overflown segments (condition) then we have to expand the table
        return true;
      }
    }
    // if there's no room check the alternative bucket!
    for (int i = 1; i < maxMisses; i++) {
      int altBucketIndex = (bucketIndex ^ fingerprint) % (1UL << bucketBitLength);
      list<ul>::iterator it = seg->buckets[altBucketIndex].begin();
      for (int j = 0; j < bucketSize; j++, it++) {
        if (it == seg->buckets[altBucketIndex].end()) {
          seg->buckets[altBucketIndex].push_back(fingerprint);
          return true;
        }
      }
      // if there's no space in the alternative bucket, place it somwhere almost random anyway
      // take the fingerprint that was there before and save it like it's new
      // fingerprint altbucket is now the main bucket (one that we already checked)
      // new alt bucket is calculated in the next loop iteration
      int rnd = fingerprint >> (fingerprintBitLength - 2);
      ul newfingerprint;

      it = seg->buckets[altBucketIndex].begin();
      for ( int i = 0; i<rnd; i++, it++);
      newfingerprint = *it;
      *it = fingerprint;
      fingerprint = newfingerprint;
      bucketIndex = altBucketIndex;
    }
    seg->overflow[bucketIndex].push_back(fingerprint);
    //expand condition
    if (numOfItems % k1 == 0) {
      bfExpand();
    }

    return true;
  }
  bool bfLookUp(T item) {
    ul fingerprint, bucketIndex, segmentIndex;
    getHashed(item, fingerprint, bucketIndex, segmentIndex);

    if (this->table[segmentIndex] == nullptr) {
      return false;
    }
    Segment *seg = this->table[segmentIndex];

    for (ul f : seg->buckets[bucketIndex]) {
      if (f == fingerprint) {
        return true;
      }
    }

    int altBucketIndex = (bucketIndex ^ fingerprint) % (1UL << bucketBitLength);
    for (ul f : seg->buckets[altBucketIndex]) {
      if (f == fingerprint) {
        return true;
      }
    }

    for (ul f : seg->overflow[bucketIndex]) {
      if (f == fingerprint)
        return true;
    }
    for (ul f : seg->overflow[altBucketIndex]) {
      if (f == fingerprint)
        return true;
    }
    return false;
  }
  
  bool bfDelete(T item) {
    ul fingerprint, bucketIndex, segmentIndex;
    getHashed(item, fingerprint, bucketIndex, segmentIndex);
    return bfDeleteHash(fingerprint, bucketIndex, segmentIndex);
  }

  bool bfDeleteHash(ul fingerprint, ul bucketIndex, ul segmentIndex) {
    if (this->table[segmentIndex] == nullptr) {
      return false;
    }
    Segment *seg = this->table[segmentIndex];

    for (auto it = seg->overflow[bucketIndex].begin();
        it != seg->overflow[bucketIndex].end();
        it++) {
      if (*it == fingerprint) {
        seg->overflow[bucketIndex].erase(it);
        //compress condition
        if (numOfItems % k2 == 0) {
          bfCompress();  
        }
        return true;
      }
    }
    ul altBucketIndex = (bucketIndex ^ fingerprint) % (1UL << bucketBitLength);
    for (auto it = seg->overflow[altBucketIndex].begin();
        it != seg->overflow[altBucketIndex].end();
        it++) {
      if (*it == fingerprint) {
        seg->overflow[altBucketIndex].erase(it);
        //compress condition
        if (numOfItems % k2 == 0) {
          bfCompress();  
        }
        return true;
      }
    }

    for (auto it = seg->buckets[bucketIndex].begin();
        it != seg->buckets[bucketIndex].end();
        it++) {
      if (*it == fingerprint) {
        seg->buckets[bucketIndex].erase(it);
        //compress condition
        if (numOfItems % k2 == 0) {
          bfCompress();  
        }
        return true;
      }
    }
    for (auto it = seg->buckets[altBucketIndex].begin();
        it != seg->buckets[altBucketIndex].end();
        it++) {
      if (*it == fingerprint) {
        seg->buckets[altBucketIndex].erase(it);
        //compress condition
        if (numOfItems % k2 == 0) {
          bfCompress();  
        }
        return true;
      }
    }
    
    return false;
  }

  void printBambooFilter() {
    cout << "BAMBOO FILTER\n";
    cout << "Number of bits reserved for number of segments: " << segmentBitLength << "\n";

    for (ul i = 0; i < (1 << segmentBitLength); i++) {
      cout << "=============== " << i << "\n";
      for (ul j = 0; j < (1 << bucketBitLength); j++) {
        if (table[i] == nullptr || table[i]->buckets[j].size() == 0)
          continue;
        cout << "------ " << j << "\n";
        for (ul f : table[i]->buckets[j]) {
          cout << f << " -> ";
        }
        cout << "nullptr ";
        cout << "overflow: ";
        for (ul f : table[i]->overflow[j]) {
          cout << f << " -> ";
        }
        cout << "nullptr\n";
      }
      cout << "===============\n";
    }
  }

private:
  ul numOfItems;
  ul segmentBitLength; 
  //bits (of the hash) reserved for segment enumeration
  // number of segments that have at least one item's fingerprint stored in it's overflow part
  // used for determining whether to expand the bamboo filter 
  //ul numOfOverflownSegs; 
  //number of empty segments (have no fingerprints stored inside)
  // used for determining whether to compress the bamboo filter 
  //ul numOfEmptySegs;
  //round of expansion
  ul roundInd;
  //next index to be split
  ul nextSeg;

  void bfExpand() {
    table.push_back(new Segment());
    Segment* seg = table[nextSeg];
    
    vector<list<ul>::iterator> toMoveFromBucket;
    vector<list<ul>::iterator> toMoveFromOverflow;
    vector<pair<list<ul>::iterator, ul>> toInsertAgain;
    for (ul i = 0; i < (1 << bucketBitLength); i++) {
      for (auto it = seg->buckets[i].begin();
          it != seg->buckets[i].end();
          it++) {
        if ((*it >> roundInd) % 2 == 1) {
          toMoveFromBucket.push_back(it); 
        }
      }      
      for (auto it = seg->overflow[i].begin();
          it != seg->overflow[i].end();
          it++) {
        if ((*it >> roundInd) % 2 == 1) {
          toMoveFromOverflow.push_back(it); 
        } else {
          toInsertAgain.push_back({it, i});
        }
      }
      for (auto it : toMoveFromOverflow) {
        bfInsertHash(*it, i, table.size() - 1);
        ///seg->overflow[i].erase(it);
      }
      for (auto it : toMoveFromOverflow) {
        bfDeleteHash(*it, i, nextSeg);
        ///seg->overflow[i].erase(it);
      }
      for (auto it : toMoveFromBucket) {
        bfInsertHash(*it, i, table.size() - 1);
      }
      for (auto it : toMoveFromBucket) {
        bfDeleteHash(*it, i, nextSeg);
      }
    }
    for (auto [it, i] : toInsertAgain) {
      bfDeleteHash(*it, i, nextSeg);
      //seg->overflow[i].erase(it);
    }
    for (auto [it, i] : toInsertAgain) { 
      bfInsertHash(*it, i, nextSeg);
    }

    nextSeg++;
    if (nextSeg == (1 << roundInd) * N0){
      segmentBitLength++;
      roundInd++;
      nextSeg = 0;
    }
    return;
  }
  void bfCompress() {
    if (table.size() == N0) {
      return;
    }
    Segment* seg = table[table.size() - 1];
    for (ul i = 0; i < (1 << bucketBitLength); i++) {
      for (auto it = seg->buckets[i].begin();
          it != seg->buckets[i].end();
          it++) {
        bfInsertHash(*it, i, table.size() - 1 - (1 << (segmentBitLength - 1)));
      }
      for (auto it = seg->overflow[i].begin();
          it != seg->overflow[i].end();
          it++) {
        bfInsertHash(*it, i, table.size() - 1 - (1 << (segmentBitLength - 1)));
      }
    }
    delete seg;
    table.resize(table.size() - 1);

    if (nextSeg == 0) {
      segmentBitLength--;
      roundInd--;
      nextSeg = (1 << roundInd) - 1;
    } else {
      nextSeg--;
    }
    return;
  }

};
int main() {
  BambooFilter<string> *bfTest = new BambooFilter<string>(4);
  bool result;
  //for (ull i = 0; i < 1e2; i++)
  //  result = bfTest->bfInsert("HelloWorldi");
  //result = bfTest->bfInsert("HelloWorld");
  //result = bfTest->bfInsert("HelloWorldj");
  //bfTest->printBambooFilter();
    size_t add_count = 65500;

    cout << "Prepare..." << endl;

    vector<string> to_add, to_lookup;
    GenerateRandom64(add_count, to_add, to_lookup);

    cout << "Begin test" << endl;

    BambooFilter<string> *bbf = new BambooFilter<string>(N0);

    auto start_time = NowNanos();

    for (uint64_t added = 0; added < add_count; added++)
    {
        bbf->bfInsert(to_add[added].c_str());
    }

    cout << ((add_count * 1000.0) / static_cast<double>(NowNanos() - start_time)) << endl;

    start_time = NowNanos();
    for (uint64_t added = 0; added < add_count; added++)
    {
        if (!bbf->bfLookUp(to_add[added].c_str()))
        {
            throw logic_error("False Negative");
        }
    }
    cout << ((add_count * 1000.0) / static_cast<double>(NowNanos() - start_time)) << endl;

    return 0;
}
