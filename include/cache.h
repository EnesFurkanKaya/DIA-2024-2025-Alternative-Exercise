#include <string>
#include "core.h"
#include <unordered_map>
#include <map>
#include <optional>
#include <set>

using namespace std;

#define CACHE_DEFAULT INT32_MAX

struct Key{
    const string& word;
    const MatchType matchtype;

    Key(const string& w, const MatchType mt) : word(w), matchtype(mt) {};

    bool operator==(const Key& other) const {
        return word == other.word && matchtype == other.matchtype;
    }

    bool operator<(const Key& other) const {
        if(word < other.word)return true;
        else if (word > other.word)return false;
        else if (matchtype < other.matchtype) return true;
        else return false;
    }
};

namespace std {
    template <>
    struct hash<Key> {
        size_t operator()(const Key& key) const {
            size_t h1 = std::hash<std::string>()(key.word);
            size_t h2 = std::hash<int>()(static_cast<int>(key.matchtype));
            return h1 ^ (h2 << 1);
        }
    };
}

class Cache{
    unordered_map<Key, unsigned int> map;
    unsigned int hits;
    unsigned int requests;

public:
    Cache();

    unsigned int get(const string &query_word, const MatchType matchtype);

    void add(const string &query_word, const MatchType matchtype, const unsigned int dist);

    void clear();

    float hitRate();
};
