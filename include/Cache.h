#include <string>
#include "core.h"
#include <unordered_map>
#include <set>

using namespace std;

#define CACHE_SIZE 100000
#define CACHE_DEFAULT INT32_MAX

template<typename Key, typename Value>
class LFUCache{
private:
    unordered_map<Key, Value> key_to_dist;
    unordered_map<unsigned int, set<Key>> freq_to_key;
    unordered_map<Key, unsigned int> key_to_freq;
    unsigned int capacity;
    unsigned int hits;
    unsigned int requests;

    void update_freq(const Key& key);

    void removeOneKey();


protected:
    LFUCache();

    unsigned int get(const Key& key);

    void add(const Key& key, const Value& value);
};

struct DocKey{
    string word;
    MatchType matchtype;

    DocKey() = default;

    bool operator==(const DocKey& other) const {
        return word == other.word && matchtype == other.matchtype;
    }

    bool operator<(const DocKey& other) const {
        if(word < other.word)return true;
        else if (word > other.word)return false;
        else if (matchtype < other.matchtype) return true;
        else return false;
    }
};

namespace std {
    template <>
    struct hash<DocKey> {
        size_t operator()(const DocKey& key) const {
            size_t h1 = std::hash<std::string>()(key.word);
            size_t h2 = std::hash<int>()(static_cast<int>(key.matchtype));
            return h1 ^ (h2 << 1);
        }
    };
}

class DocCache:LFUCache<DocKey, unsigned int>{
public:
    DocCache();

    unsigned int get(string &query_word, MatchType matchtype);

    void add(string &query_word, MatchType matchtype, unsigned int dist);

};