#include "cache.h"
template <typename Key, typename Value>
inline Cache<Key, Value>::Cache()
{
    Cache::defaultValue=Value();
}

template <typename Key, typename Value>
Value Cache<Key, Value>::get(const Key &key)
{
    auto it = Cache::map.find(key);
    if(it != Cache::map.end())return it->second;
    return Cache::defaultValue;
}

template <typename Key, typename Value>
void Cache<Key, Value>::add(const Key &key, const Value &value)
{
    Cache::map[key] = value;
}

template <typename Key, typename Value>
void Cache<Key, Value>::clear()
{
    Cache::map.clear();
}

DocCache::DocCache()
{
    Cache::defaultValue = CACHE_DEFAULT;
}

unsigned int DocCache::get(const string &query_word, const MatchType matchtype)
{
    DocKey key = DocKey(query_word, matchtype);
    return Cache::get(key);
}

void DocCache::add(const string &query_word, const MatchType matchtype, const unsigned int dist)
{
    DocKey key = DocKey(query_word, matchtype);
    Cache::add(key, dist);
}

void DocCache::clear()
{
    Cache::clear();
}
