#ifndef CACHE_H
#define CACHE_H

#include "core.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

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

#endif