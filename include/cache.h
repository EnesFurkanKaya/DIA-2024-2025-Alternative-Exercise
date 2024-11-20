#include <string>
#include "core.h"
#include <unordered_map>
#include <unordered_map>
#include <optional>
#include <set>

using namespace std;

#define CACHE_DEFAULT INT32_MAX

template<typename Key, typename Value>
class Cache{
private:
    unordered_map<Key, Value> map;

protected:
    Value defaultValue;
    Cache();

    Value get(const Key& key);

    void add(const Key& key, const Value& value);

    void clear();
};



struct DocKey{
    const string& word;
    const MatchType matchtype;

    DocKey(const string& w, const MatchType mt) : word(w), matchtype(mt) {};

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

class DocCache:Cache<DocKey, unsigned int>{
public:
    DocCache();

    unsigned int get(const string &query_word, const MatchType matchtype);

    void add(const string &query_word, const MatchType matchtype, const unsigned int dist);

    void clear();
};