#ifndef CACHE_H
#define CACHE_H

#include "core.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <string>
#include <utility>
#include <algorithm>

using namespace std;

struct WordCacheKey {
    string word;
    MatchType match_type;
    unsigned int match_dist;

    bool operator==(const WordCacheKey& other) const;
};

namespace std {
    template<>
    struct hash<WordCacheKey> {
        size_t operator()(const WordCacheKey& key) const;
    };
}

// The Least Frequently Used (LFU) Cache 
class FrequencyCache{
    public:
        explicit FrequencyCache(size_t max_cache_size);
        // Insert a key-value pair into the cache
        void insert(const WordCacheKey& key, const unordered_set<string>& value);
        // Retrieve the value associated with a key
        unordered_set<string>* get(const WordCacheKey& key);
        // Clear the Cache
        void clear();
    private:
        using CacheList = list<pair<WordCacheKey, unordered_set<string>>>;
        using CacheMap = unordered_map<WordCacheKey, pair<CacheList::iterator, int>>;

        // Stores cache entries with frequency tracking
        CacheList cache_list;
        // Maps keys to list iterators and usage frequency
        CacheMap cache_map;  
        size_t max_size;

        size_t hit_count;
        size_t miss_count;

        // Helper function to evict the least frequently used entry
        void evict();
};

#endif