#include <bits/stdc++.h>
#include <cstddef>
#include <vector>

#define bucketSize 4
#define bucketLength 12
#define segLength 4
#define fingerprintLength 16
#define maxMisses 5

using namespace std;
using ull = unsigned long long;
using ul = unsigned long int; // ":/"

class Segment {
public:
  ul buckets[1UL << bucketLength][bucketSize]={0}; //vjv treba fixat ovo al za sada nek bude tako
  Segment *overflow;
};

template <typename T> class BambooFilter {
public:
  
  vector<Segment *> table;
  
  inline void getHashed(T item, ul& fingerprint, ul& bucketIndex, ul& segmentIndex){
    const hash<T> h;
    fingerprint = (h(item) >> (bucketLength + segLength)) % (1UL<<fingerprintLength);
    bucketIndex = h(item) % (1UL<<bucketLength);
    segmentIndex = (h(item) >> (bucketLength)) % (1UL<<segLength);
}

  // potencijalna optimizacija, incijaliziraj odmah fiksnu veličinu vektora
  BambooFilter<T>(size_t segmentLength)
      : table(1UL << segmentLength, nullptr), segmentLength(segmentLength){};

  

  bool bfInsert(T item) {
      //TODO: prije returnova pozvati expandove ako su potrebni
      ul fingerprint, bucketIndex, segmentIndex;
      getHashed(item, fingerprint, bucketIndex, segmentIndex);
      
      if (this->table[segmentIndex] == nullptr) { 
        table[segmentIndex] = new Segment();
      } 
      Segment* seg = this->table[segmentIndex];

      //provjeri jel ima mjesta u prvom bucketu, ak ima super :D
      for (int bi=0; bi<bucketSize; bi++) {
        if (seg->buckets[bucketIndex][bi] == 0) {
          seg->buckets[bucketIndex][bi] = fingerprint;
          cout << (seg->buckets[bucketIndex][bi]);
          return true;
        }
      }
      //ak nema mjesta u prvom provjeri drugi bucket!
      for (int i=1; i<maxMisses; i++) {
        int altBucketIndex = (bucketIndex ^ fingerprint) % (1UL<<bucketLength);
        for (int bi=0; bi<bucketSize; bi++) {
          if (seg->buckets[altBucketIndex][bi] == 0) {
            seg->buckets[altBucketIndex][bi] = fingerprint;
            cout << (seg->buckets[altBucketIndex][bi]);
            return true;
          }
      }
      //ak nema u drugom, ubaci ga na (skoro)random mjesto anyway, 
      //uzmi fingerprint koji je prije bio tamo i spremi ga kao novi fingerprint
      //altbucket je sada main bucket (onaj koji smo vec provjerili)
      //novi alt bucket se racuna u sljedecoj iteraciji for petlje
        int bi = fingerprint >> (fingerprintLength-2);
        ul newfingerprint = seg->buckets[altBucketIndex][bi];
        seg->buckets[altBucketIndex][bi] = fingerprint;
        bucketIndex = altBucketIndex;
        fingerprint = newfingerprint;

      }
      //ak smo failali, return false
      //TODO: spremiti stvari u overflow segmente
      return false;
    }
  bool bfLookUp(T item) {
      ul fingerprint, bucketIndex, segmentIndex;
      getHashed(item, fingerprint, bucketIndex, segmentIndex);
      if (this->table[segmentIndex] == nullptr) { 
        table[segmentIndex] = new Segment();
      } 
      Segment* seg = this->table[segmentIndex];

      for (int bi=0; bi<bucketSize; bi++) {
        if (seg->buckets[bucketIndex][bi] == fingerprint) {
          return true;
        }
      }
      int altBucketIndex = (bucketIndex ^ fingerprint) % (1UL<<bucketLength);
      for (int bi=0; bi<bucketSize; bi++) {
        if (seg->buckets[altBucketIndex][bi] == fingerprint) {
          return true;
        }
      }
      return false;
      //TODO: srediti overflow (once again :D)
  }
  bool bfDelete(T item) {
    ul fingerprint, bucketIndex, segmentIndex;
      getHashed(item, fingerprint, bucketIndex, segmentIndex);

      if (this->table[segmentIndex] == nullptr) { 
        table[segmentIndex] = new Segment();
      } 
      Segment* seg = this->table[segmentIndex];

      for (int bi=0; bi<bucketSize; bi++) {
        if (seg->buckets[bucketIndex][bi] == fingerprint) {
          seg->buckets[bucketIndex][bi] = 0; //kako definiramo praznu stvar?
          return true;
        }
      }
      int altBucketIndex = (bucketIndex ^ fingerprint) % (1UL<<bucketLength);
      for (int bi=0; bi<bucketSize; bi++) {
        if (seg->buckets[altBucketIndex][bi] == fingerprint) {
          seg->buckets[altBucketIndex][bi] =0; //opet mora bit prazno
          return true;
        }
      }
      return false;
      //TODO: srediti overflow (twice again :D)
  }

private:
  int segmentLength;

  void bfExpand() {
    for (int i = 0; i < (1 << segmentLength); i++) {
    }
  }
  void bfCompress() {}
};
int main() {
    BambooFilter<string>* bfTest = new BambooFilter<string>(segLength); 
    bool result;
    
    result = bfTest->bfInsert ("HelloWorldi");
    if (result) {
      cout << "Success";
    } else {
      cout << "Fail";
    }
    result = bfTest->bfInsert ("HelloWorld");
    if (result) {
      cout << "Success";
    } else {
      cout << "Fail";
    }
    result = bfTest->bfLookUp ("HelloWorld");
    if (result) {
      cout << "Success";
    } else {
      cout << "Fail";
    }
    result = bfTest->bfDelete ("HelloWorldj");
    if (result) {
      cout << "Success";
    } else {
      cout << "Fail";
    }
    result = bfTest->bfLookUp ("HelloWorldj");
    if (result) {
      cout << "Success";
    } else {
      cout << "Fail";
    }
}
