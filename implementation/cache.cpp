#include "cache.h"
Cache::Cache()
{
    requests = 0;
    hits = 0;
}

unsigned int Cache::get(const string &query_word, const MatchType matchtype)
{
    requests++;
    auto it = map.find(Key(query_word, matchtype));
    if(it != map.end()){
        hits++;
        return it->second;
    }
    return CACHE_DEFAULT;
}

void Cache::add(const string &query_word, const MatchType matchtype, const unsigned int dist)
{
    map[Key(query_word, matchtype)] = dist;
}

void Cache::clear()
{
    map.clear();
}

float Cache::hitRate()
{
    if(requests == 0) return 0;
    return (float) hits / (float) requests;
}
