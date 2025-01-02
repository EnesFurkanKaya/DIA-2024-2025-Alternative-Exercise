#include "cache.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <iostream>

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