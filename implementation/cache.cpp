#include "cache.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <iostream>
#include <list>
#include <string>
#include <utility>
#include <algorithm>

using namespace std;

bool WordCacheKey::operator==(const WordCacheKey& other) const{
    return word == other.word &&
            match_type == other.match_type &&
            match_dist == other.match_dist;
};

size_t std::hash<WordCacheKey>::operator()(const WordCacheKey& key) const {

    size_t hash_value = std::hash<string>()(key.word);
    hash_value ^= std::hash<int>()(key.match_type) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
    hash_value ^= std::hash<int>()(key.match_dist) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
    return hash_value;
    
}

FrequencyCache::FrequencyCache(size_t max_cache_size) : max_size(max_cache_size) {}

void FrequencyCache::insert(const WordCacheKey& key, const unordered_set<string>& value) {
    auto iter = cache_map.find(key);
    if (iter != cache_map.end()) {
        iter->second.second++;
        iter->second.first->second = value;
    } else {
        if (cache_list.size() >= max_size) {
            evict();
        }
        // Insert new entry
        cache_list.emplace_front(key, value);
        cache_map[key] = {cache_list.begin(), 1};
    }
}

unordered_set<string>* FrequencyCache::get(const WordCacheKey& key) {
    auto iter = cache_map.find(key);
    if (iter != cache_map.end()) {
        hit_count++;
        iter->second.second++;
        return &iter->second.first->second;
    } else {
        miss_count++;
        return nullptr;
    }
}

void FrequencyCache::clear() {
    cache_list.clear();
    cache_map.clear();
    hit_count = 0;
    miss_count = 0;
}

void FrequencyCache::evict() {
    auto to_evict = std::min_element(
        cache_map.begin(), cache_map.end(),
        [](const auto& a, const auto& b) { return a.second.second < b.second.second; });

    if (to_evict != cache_map.end()) {
        cache_list.erase(to_evict->second.first);
        cache_map.erase(to_evict);
    }
}