#include <bits/stdc++.h>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <vector>

#define bucketSize 4
#define bucketBitLength 12
#define fingerprintBitLength 16
#define maxMisses 5
#define expandConst 0.5
#define compressConst 0.5
using namespace std;
using ull = unsigned long long;
using ul = unsigned long int; 

class Segment {
public:
  ul numOfOverflownBuckets;
  ul numOfElements;
  list<ul> buckets[1UL << bucketBitLength]; 
  list<ul> overflow[1UL << bucketBitLength];
  Segment() : numOfOverflownBuckets(0), numOfElements(0) {};
  ~Segment() = default;
};

template <typename T> class BambooFilter {
public:
  vector<Segment*> table;

  inline void getHashed(T item, ul &fingerprint, ul &bucketIndex, ul &segmentIndex) {
    const hash<T> h;
    fingerprint = (h(item) >> (bucketBitLength + segmentBitLength)) % (1UL << fingerprintBitLength); // mod unnecessary?
    bucketIndex = h(item) % (1UL << bucketBitLength);
    segmentIndex = (h(item) >> (bucketBitLength)) % (1UL << segmentBitLength);
  }

  BambooFilter<T>(size_t segmentBitLength) 
    : table(1UL << segmentBitLength, nullptr),
    segmentBitLength(segmentBitLength),
    numOfOverflownSegs(0),
    numOfEmptySegs(1<<segmentBitLength){};

  bool bfInsert(T item) {
    ul fingerprint, bucketIndex, segmentIndex;
    getHashed(item, fingerprint, bucketIndex, segmentIndex);

    if (this->table[segmentIndex] == nullptr) {
      table[segmentIndex] = new Segment();
    }
    Segment *seg = this->table[segmentIndex];
    if (seg->numOfElements == 0)
      numOfEmptySegs--;
    seg->numOfElements++;
    // check if there's room in the first bucket, if yes great :D
    list<ul>::iterator it = seg->buckets[bucketIndex].begin();
    for (int i = 0; i < bucketSize; i++) {
      if (it == seg->buckets[bucketIndex].end()) {
        seg->buckets[bucketIndex].push_back(fingerprint);
        //if we have too many overflown segments (condition) then we have to expand the table
        if (numOfOverflownSegs > expandConst * (1UL << segmentBitLength))
          bfExpand();
        return true;
      }
    }
    // if there's no room check the alternative bucket!
    for (int i = 1; i < maxMisses; i++) {
      int altBucketIndex = (bucketIndex ^ fingerprint) % (1UL << bucketBitLength);
      list<ul>::iterator it = seg->buckets[altBucketIndex].begin();
      for (int j = 0; j < bucketSize; j++) {
        if (it == seg->buckets[altBucketIndex].end()) {
          seg->buckets[altBucketIndex].push_back(fingerprint);
          //if we have too many overflown segments (condition) then we have to expand the table
          if (numOfOverflownSegs > expandConst * (1UL << segmentBitLength))
            bfExpand();
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
      *it = fingerprint;
      bucketIndex = altBucketIndex;
      fingerprint = newfingerprint;
    }
    if (seg->numOfOverflownBuckets == 0) {
      numOfOverflownSegs++;
    }
    seg->numOfOverflownBuckets++;
    seg->overflow[bucketIndex].push_back(fingerprint);
    //if we have too many overflown segments (condition) then we have to expand the table
    if (numOfOverflownSegs > expandConst * (1UL << segmentBitLength))
      bfExpand();

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
    return false;
  }
  bool bfDelete(T item) {
    ul fingerprint, bucketIndex, segmentIndex;
    getHashed(item, fingerprint, bucketIndex, segmentIndex);

    if (this->table[segmentIndex] == nullptr) {
      table[segmentIndex] = new Segment();
    }
    Segment *seg = this->table[segmentIndex];
    seg->numOfElements--;
    if (seg->numOfElements == 0)
      numOfEmptySegs++;

    for (auto it = seg->buckets[bucketIndex].begin();
        it != seg->buckets[bucketIndex].end();
        it++) {
      if (*it == fingerprint) {
        seg->buckets[bucketIndex].erase(it);
        //if we have too many empty segments (condition) then we have to compress the table
        if (numOfEmptySegs > compressConst * (1UL << segmentBitLength)) {
          bfCompress();  
        }
        return true;
      }
    }
    int altBucketIndex = (bucketIndex ^ fingerprint) % (1UL << bucketBitLength);
    for (auto it = seg->buckets[altBucketIndex].begin();
        it != seg->buckets[altBucketIndex].end();
        it++) {
      if (*it == fingerprint) {
        seg->buckets[altBucketIndex].erase(it);
        //if we have too many empty segments (condition) then we have to compress the table
        if (numOfEmptySegs > compressConst * (1UL << segmentBitLength)) {
          bfCompress();  
        }
        return true;
      }
    }
    for (auto it = seg->overflow[bucketIndex].begin();
        it != seg->overflow[bucketIndex].end();
        it++) {
      if (*it == fingerprint) {
        seg->overflow[bucketIndex].erase(it);
        seg->numOfOverflownBuckets--;
        if (seg->numOfOverflownBuckets == 0)
          numOfOverflownSegs--;
        //if we have too many empty segments (condition) then we have to compress the table
        if (numOfEmptySegs > compressConst * (1UL << segmentBitLength)) {
          bfCompress();  
        }
        return true;
      }
    }
    
    return false;
  }

  void printBambooFilter() {
    cout << "BAMBOO FILTER\n";
    cout << "Number of empty segments: " << numOfEmptySegs << "\n";
    cout << "Number of overflown segments: " << numOfOverflownSegs << "\n";
    cout << "Number of bits reserved for number of segments: " << segmentBitLength << "\n";

    for (ul i = 0; i < (1 << segmentBitLength); i++) {
      cout << "=============== " << i << "\n";
      for (ul j = 0; j < (1 << bucketBitLength); j++) {
        if (table[i]->buckets[j].size() == 0)
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
        cout << "nullptr ";
      }
      cout << "===============\n";
    }
  }

private:
  ul segmentBitLength; 
  //bits (of the hash) reserved for segment enumeration
  // number of segments that have at least one item's fingerprint stored in it's overflow part
  // used for determining whether to expand the bamboo filter 
  ul numOfOverflownSegs; 
  //number of empty segments (have no fingerprints stored inside)
  // used for determining whether to compress the bamboo filter 
  ul numOfEmptySegs;
  //TODO numOfEmptySegs numOfOverflownSegs
  void bfExpand() {
    // we iterate over every segment and generate a new segment for each existing segment
    // after creating each segment we shrink the fingerprints by one bit by dividing them by 2 and then we redistribute them
    for (int i = 0; i < (1 << segmentBitLength); i++) {
      Segment* newSeg = new Segment();
      table.push_back(newSeg);
      //initially, new segment is empty
      numOfEmptySegs++;
      Segment* seg = table[i];
      //iterate over the buckets of current segment (i-th segment) 
      for (int j = 0; j < (1 << bucketBitLength); j++) {
        //we need to remember the figerprints that need to be deleted from the current (i-th) segment
        vector<list<ul>::iterator> toDelete1;

        //now we iterate over the current (j-th) bucket
        for (auto it = seg->buckets[j].begin();
            it != seg->buckets[j].end();
            it++) {
          ul fingerprint = *it;
          //we shrink the fingerprint
          *it /= 2;
          //if the shrunk bit is 1 place the fingerprint in the new segment
          //otherwise it can stay where it is 
          if (fingerprint % 2) {
            //check if newSegment stoped being empty (was empty and now it obviously won't be)
            if (newSeg->numOfElements == 0)
              numOfEmptySegs--;
            newSeg->numOfElements++;
            newSeg->buckets[j].push_back(*it); 
            toDelete1.push_back(it);
            //we don't have to worry about overlowing newSeg since there aren't enough elements to do that YET
          }
        }
        //now we delete the mentioned fingerprints
        for (auto it : toDelete1) { 
          seg->buckets[j].erase(it);
          seg->numOfElements--;
        }
        
        //we have to do the same for the elements in the overflow but now be have to worry about overflowing newSeg
        vector<list<ul>::iterator> toDelete2;
        for (auto it = seg->overflow[j].begin();
            it != seg->overflow[j].end();
            it++) {
          ul fingerprint = *it;
          *it /= 2;
          if (fingerprint % 2) {
            //if newSeg was empty we have to adjust num of empty segments
            if (newSeg->numOfElements == 0)
              numOfEmptySegs--;
            
            //check if we will overflow newSeg
            if (newSeg->buckets[j].size() < bucketSize)
              //no overflow, add it in the bucket
              newSeg->buckets[j].push_back(*it); 
            else {
              //overflow happend, add it to the overflow
              //if it wasn't already overflown adjust numOfOverflownBuckets and numOfOverflownSegs
              if (newSeg->numOfOverflownBuckets == 0)
                numOfOverflownSegs++;
              if (newSeg->overflow[j].size() == 0)
                newSeg->numOfOverflownBuckets++;
              newSeg->overflow[j].push_back(*it);
              
            }
            newSeg->numOfElements++;
            toDelete2.push_back(it);
          }
        } 
        //newSeg is now done
        //we turn out attention to seg
        for (auto it : toDelete2) { 
          seg->overflow[j].erase(it);
          seg->numOfElements--;
        }
        //we now need to move elements from the overflow to the bucket
        vector<list<ul>::iterator> toDelete3;
        for (auto it = seg->overflow[j].begin();
            it != seg->overflow[j].end();
            it++) {
          //if there's space in the bucket move the fingerprint to the bucket
          if (seg->buckets[j].size() < bucketSize) {
            seg->buckets[j].push_back(*it);
            toDelete3.push_back(it);
          } else {
            //if there's no space in the bucket we can stop
            break;
          }
        }
  
        for (auto it : toDelete3) {
          seg->overflow[j].erase(it);
        }
        if (seg->overflow[j].size() == 0 && toDelete3.size() != 0){
          seg->numOfOverflownBuckets--;
          if (seg->numOfOverflownBuckets == 0)
            numOfOverflownSegs--;
        }
        //and now seg is done too
      }
    }
    //adjust segmentBitLength
    segmentBitLength++;
  }
  void bfCompress() {
    //we iterate over the segments we want to remove (we iterate from the back)
    //we have to also expand the fingerptints by multiplying them by 2 and also, depending in which segment they are add 1
    for (ul i = (1 << segmentBitLength) - 1;
        i >= (1 << (segmentBitLength - 1));
        i--) {
      //segmetn to be deleted
      Segment* toDeleteSeg = table[i];
      //segment that will stay and will store fingerprint of the deleted segment
      Segment* toMergeSeg = table[i - (1 << (segmentBitLength - 1))];
      //check if we are deleting an empty segment 
      if (toDeleteSeg->numOfElements == 0)
        numOfEmptySegs--;
      //check if we are deleting an overflown segment (N.B. an empty segment certainly won't be overflown)
      else if (toDeleteSeg->numOfOverflownBuckets != 0)
        numOfOverflownSegs--;

      //we need to expand the fingerprints in toMergeSeg
      for (ul j = 0; j < (1 << bucketBitLength); j++) {
        for (auto it = toMergeSeg->buckets[j].begin();
            it != toMergeSeg->buckets[j].end();
            it++) {
          *it *= 2;
        }
        for (auto it = toMergeSeg->overflow[j].begin();
            it != toMergeSeg->overflow[j].end();
            it++) {
          *it *= 2;
        }

        //now we need to move fingerprints from the segment that we will delete to toMergeSeg-

        // is toMergeSeg- overflown (initially set to false) 
        bool isOverflown = false;
        // is toMergeSeg overflown right now  
        const bool wasOverflown = (toMergeSeg->numOfOverflownBuckets != 0);
        // is toMergeSeg empty right now
        const bool wasEmpty = (toMergeSeg->numOfElements == 0);
        for (auto it = toDeleteSeg->buckets[j].begin();
            it != toDeleteSeg->buckets[j].end();
            it++) {
          //try to place it in the bucket and if not place it in the overflow
          if (toMergeSeg->buckets[j].size() < bucketSize) {
            //expend and add
            toMergeSeg->buckets[j].push_back(*it * 2 + 1);
            toMergeSeg->numOfElements++;
          } else {
            isOverflown = true;
            if (toMergeSeg->overflow[j].size() == 0)
              toMergeSeg->numOfOverflownBuckets++;
            toMergeSeg->overflow[j].push_back(*it * 2 + 1);
            toMergeSeg->numOfElements++;
          }
        }
        // if we just overflew toMergeSeg- adjust numOfOverflownSegs 
        if (!wasOverflown && isOverflown)
          numOfOverflownSegs++;
        // if toMergeSeg was empty adjust numOfEmptySeg
        if (wasEmpty && (toMergeSeg->numOfElements != 0))
          numOfEmptySegs--;

      }
      delete toDeleteSeg;
    }
    //adjust segmentBitLength
    segmentBitLength--;
    table.resize(1 << (segmentBitLength));
  }
};
int main() {
  BambooFilter<string> *bfTest = new BambooFilter<string>(4);
  bool result;

  result = bfTest->bfInsert("HelloWorldi");
  result = bfTest->bfInsert("HelloWorld");
  result = bfTest->bfLookUp("HelloWorld");
  result = bfTest->bfDelete("HelloWorldj");
  result = bfTest->bfLookUp("HelloWorldj");
  bfTest->printBambooFilter();
}
