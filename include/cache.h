#include <string>
#include "core.h"
#include <unordered_map>
#include <map>
#include <optional>
#include <set>
#include <distance.h>


using namespace std;

#define CACHE_DEFAULT INT32_MAX
#define UPPER_BOUND_QUERYS 5

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
struct Value{
    unsigned int distance;
    set<string>::iterator next_doc_word;

    Value() = default;
    Value(const unsigned int dist, const set<string>::iterator &ndw):distance(dist),next_doc_word(ndw){};
};
class Cache{
    unordered_map<string, Value> hamming_map;
    unordered_map<string, Value> edit_map;
    set<string>::iterator doc_words_begin; 

    unsigned int hits;
    unsigned int requests;

public:
    Cache(const set<string>::iterator &it);

    Value getHammingDistance(const string &query_word);

    void addHammingDistance(const string &query_word, const unsigned int dist, const set<string>::iterator &it);

    Value getEditDistance(const string &query_word);

    void addEditDistance(const string &query_word, const unsigned int dist, const set<string>::iterator &it);

    float hitRate();

    unsigned int upperBoundHammingDistance(const string &query_word);
    unsigned int upperBoundEditDistance(const string &query_word);

};