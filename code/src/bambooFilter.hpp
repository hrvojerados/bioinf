// bambooFilter.hpp
#pragma once

#include <bits/stdc++.h>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <type_traits>
#include <fstream>
#include <iostream>
#include <vector>
#include <boost/container/small_vector.hpp>
#include <random>

#include "../common/random.h"
#include "../common/timing.h"
#include "../common/BOBHash.h"
#define bucketSize 4
#define bucketBitLength 10
#define fingerprintBitLength 16
#define maxMisses 8 // sto je vece to je bolji lookup, ali je stoga sporiji insert
#define initialSegBitLength 6
#define N0 (1 << initialSegBitLength)

#define	FORCE_INLINE inline __attribute__((always_inline))
#define k1 (1 << (bucketBitLength))
#define k2 (2 * bucketSize * (1 << bucketBitLength))


using namespace std;
using ull = unsigned long long;
using u = u_int32_t; 
using SmallVec = boost::container::small_vector<u, 4>;

class Segment {
public:
  SmallVec buckets[1U << bucketBitLength]; 
  list<u> overflow[1U << bucketBitLength];
  Segment() = default;
  ~Segment() = default;
};

class BambooFilter {
public:
  vector<Segment*> table;

  BambooFilter(size_t segmentBitLength) 
    : table(1U << segmentBitLength, nullptr),
    segmentBitLength(segmentBitLength),
    numOfItems(0),
    roundInd(0),
    nextSeg(0){
      for (u i = 0; i < (1U << segmentBitLength); i++) {
        table[i] = new Segment();
      }
    };
  
  FORCE_INLINE void getHashed(const char* item, u &fingerprint, u &bucketIndex, u &segmentIndex) const {
    const u hash = BOBHash::run(item, strlen(item), 3);
    fingerprint = (hash >> (32 - fingerprintBitLength)); // mod unnecessary?
    bucketIndex = hash & ((1U << bucketBitLength) - 1);
    segmentIndex = (hash >> bucketBitLength) & ((1U << (segmentBitLength + 1)) - 1);
    if (segmentIndex >= table.size())
      segmentIndex -= (1U << (segmentBitLength));
  }

  bool bfInsert(const char* item) {
    numOfItems++;
    u fingerprint, bucketIndex, segmentIndex;
    getHashed(item, fingerprint, bucketIndex, segmentIndex);
    bfInsertHash(fingerprint, bucketIndex, segmentIndex);
    //expand condition  
    if (numOfItems % k1 == 0) {
      bfExpand();
    }
    return true; 
  }
  FORCE_INLINE bool bfInsertHash(u fingerprint, u bucketIndex, u segmentIndex) {
    Segment *seg = this->table[segmentIndex];
    // check if there's room in the first bucket, if yes great :D
    if (seg->buckets[bucketIndex].size() < 4) {
        seg->buckets[bucketIndex].push_back(fingerprint);
        return true;
    } 
    // if there's no room check the alternative bucket!
    for (int i = 1; i < maxMisses; i++) {
      int altBucketIndex = (bucketIndex ^ fingerprint) & ((1U << bucketBitLength) - 1);
      if (seg->buckets[altBucketIndex].size() < 4) {
          seg->buckets[altBucketIndex].push_back(fingerprint);
          return true;
      } 
      // if there's no space in the alternative bucket, place it somwhere almost random anyway
      // take the fingerprint that was there before and save it like it's new
      // fingerprint altbucket is now the main bucket (one that we already checked)
      // new alt bucket is calculated in the next loop iteration
      u rnd = fingerprint >> (fingerprintBitLength - 2);
      u newfingerprint = seg->buckets[altBucketIndex][rnd];
      seg->buckets[altBucketIndex][rnd] = fingerprint;
      fingerprint = newfingerprint;
      bucketIndex = altBucketIndex;
    }
    seg->overflow[bucketIndex].push_back(fingerprint);
    return true;
  }
  FORCE_INLINE bool bfLookUp(const char* item) {
    u fingerprint, bucketIndex, segmentIndex;
    getHashed(item, fingerprint, bucketIndex, segmentIndex);

    Segment *seg = this->table[segmentIndex];

    for (u f : seg->buckets[bucketIndex]) {
      if (f == fingerprint) {
        return true;
      }
    }

    constexpr u bitMask = (1U << bucketBitLength) - 1;
    const u altBucketIndex = (bucketIndex ^ fingerprint) & bitMask;
    for (u f : seg->buckets[altBucketIndex]) {
      if (f == fingerprint) {
        return true;
      }
    }
    if (find(
          seg->overflow[bucketIndex].begin(),
          seg->overflow[bucketIndex].end(),
          fingerprint) != seg->overflow[bucketIndex].end()) {
      return true;
    }
    if (find(
          seg->overflow[altBucketIndex].begin(),
          seg->overflow[altBucketIndex].end(),
          fingerprint) != seg->overflow[altBucketIndex].end()) {
      return true;
    }
    return false;
  }
  
  bool bfDelete(const char* item) {
    u fingerprint, bucketIndex, segmentIndex;
    getHashed(item, fingerprint, bucketIndex, segmentIndex);
    if (bfDeleteHash(fingerprint, bucketIndex, segmentIndex)) {
      numOfItems--;
      //compress condition
      if (numOfItems % k2 == 0) {
        bfCompress();  
      }
      return true;
    }
    return false;
  }

  FORCE_INLINE bool bfDeleteHash(u fingerprint, u bucketIndex, u segmentIndex) {
    Segment *seg = this->table[segmentIndex];

    for (auto it = seg->overflow[bucketIndex].begin();
      it != seg->overflow[bucketIndex].end();
        it++) {
      if (*it == fingerprint) {
        seg->overflow[bucketIndex].erase(it);
        return true;
      }
    }
    
    u altBucketIndex = (bucketIndex ^ fingerprint) & ((1U << bucketBitLength) - 1);
    for (auto it = seg->overflow[altBucketIndex].begin();
        it != seg->overflow[altBucketIndex].end();
        it++) {
      if (*it == fingerprint) {
        seg->overflow[altBucketIndex].erase(it);
        return true;
      }
    }

    for (auto it = seg->buckets[bucketIndex].begin();
        it != seg->buckets[bucketIndex].end();
        it++) {
      if (*it == fingerprint) {
        seg->buckets[bucketIndex].erase(it);
        return true;
      }
    }
    for (auto it = seg->buckets[altBucketIndex].begin();
        it != seg->buckets[altBucketIndex].end();
        it++) {
      if (*it == fingerprint) {
        seg->buckets[altBucketIndex].erase(it);
        return true;
      }
    }
    
    return false;
  }

  void printBambooFilter() {
    cout << "BAMBOO FILTER\n";
    cout << "Number of bits reserved for number of segments: " << segmentBitLength << "\n";

    for (u i = 0; i < table.size(); i++) {
      cout << "=============== " << i << "\n";
      for (u j = 0; j < (1 << bucketBitLength); j++) {
        if (table[i] == nullptr || table[i]->buckets[j].size() == 0)
          continue;
        cout << "------ " << j << " seg=" << i <<  "\n";
        for (u f : table[i]->buckets[j]) {
          cout << f << " -> ";
        }
        cout << "nullptr ";
        cout << "overflow: ";
        for (u f : table[i]->overflow[j]) {
          cout << f << " -> ";
        }
        cout << "nullptr\n";
      }
      cout << "===============" << i <<"\n";
    }
    cout << "Number Of Items: " << numOfItems << "\n";
    cout << "Expansion const: " << nextSeg << "\n";
  }
  void printBambooFilterToFile(const string& filename = "output.txt") {
    ofstream ofs(filename);
    if (!ofs) {
        cerr << "Error: could not open file " << filename << " for writing.\n";
        return;
    }

    ofs << "BAMBOO FILTER\n";
    ofs << "Number of bits reserved for number of segments: "
        << segmentBitLength << "\n";

    for (u i = 0; i < table.size(); i++) {
        ofs << "=============== " << i << "\n";
        for (u j = 0; j < (1u << bucketBitLength); j++) {
            if (table[i] == nullptr || table[i]->buckets[j].empty())
                continue;
            ofs << "------ " << j << " seg=" << i << "\n";
            for (u f : table[i]->buckets[j]) {
                ofs << f << " -> ";
            }
            ofs << "nullptr ";
            ofs << "overflow: ";
            for (u f : table[i]->overflow[j]) {
                ofs << f << " -> ";
            }
            ofs << "nullptr\n";
        }
        ofs << "===============" << i << "\n";
    }

    ofs << "Number Of Items: " << numOfItems << "\n";
    ofs << "Expansion const: " << nextSeg << "\n";

    ofs.close();
}

private:
  u numOfItems;
  u segmentBitLength; 
  //bits (of the hash) reserved for segment enumeration
  // number of segments that have at least one item's fingerprint stored in it's overflow part
  // used for determining whether to expand the bamboo filter 
  //ul numOfOverflownSegs; 
  //number of empty segments (have no fingerprints stored inside)
  // used for determining whether to compress the bamboo filter 
  //ul numOfEmptySegs;
  //round of expansion
  u roundInd;
  //next index to be split
  u nextSeg;

  inline void bfExpand() {
    //printf("start\n");
    table.push_back(new Segment());
    Segment* seg = table[nextSeg];
    
    vector<pair<u, u>> toMoveFromBucket;
    vector<pair<list<u>::iterator, u>> toMoveFromOverflow;
    vector<pair<list<u>::iterator, u>> toInsertAgain;
    for (u bi = 0; bi < (1 << bucketBitLength); bi++) {
      for (u j = 0;
          j < seg->buckets[bi].size();
          j++) {
        if ((seg->buckets[bi][j] >> roundInd) & 1) {
          toMoveFromBucket.push_back({seg->buckets[bi][j], bi}); 
        }
      }      
      for (auto it = seg->overflow[bi].begin();
          it != seg->overflow[bi].end();
          it++) {
        if ((*it >> roundInd) & 1) {
          toMoveFromOverflow.push_back({it, bi}); 
        } else {
          toInsertAgain.push_back({it, bi});
        }
      }
    }
    for (auto [it, i] : toMoveFromOverflow) {
      bfInsertHash(*it, i, table.size() - 1);
    }
    for (auto [val, i] : toMoveFromBucket) {
      bfInsertHash(val, i, table.size() - 1);
    }
        
    for (auto [it, i] : toMoveFromOverflow) {
      seg->overflow[i].erase(it);
    }

    for (auto [val, bi] : toMoveFromBucket) {
      u tmp = val;
      seg->buckets[bi].erase(
          remove_if(
            seg->buckets[bi].begin(),
            seg->buckets[bi].end(),
            [&](const u& x) {
              if (x == tmp) return true;
              return false;
            }),
          seg->buckets[bi].end());
    }
    vector<pair<u, u>> tmp;
    for (auto [it, i] : toInsertAgain) {
      tmp.push_back({*it, i});
      seg->overflow[i].erase(it);
    }
    for (auto [item, i] : tmp) { 
      bfInsertHash(item, i, nextSeg);
    }
    
    

    nextSeg++;
    if (nextSeg == (1U << roundInd) * N0){
      segmentBitLength++;
      roundInd++;
      nextSeg = 0;
    }

    return;
  }
  inline void bfCompress() {
    if (table.size() == N0) {
      return;
    }
    Segment* seg = table[table.size() - 1];
    for (u i = 0; i < (1 << bucketBitLength); i++) {
      for (auto it = seg->buckets[i].begin();
          it != seg->buckets[i].end();
          it++) {
        bfInsertHash(*it, i, table.size() - 1 - (1U << (segmentBitLength - 1)));
      }
      for (auto it = seg->overflow[i].begin();
          it != seg->overflow[i].end();
          it++) {
        bfInsertHash(*it, i, table.size() - 1 - (1U << (segmentBitLength - 1)));
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

