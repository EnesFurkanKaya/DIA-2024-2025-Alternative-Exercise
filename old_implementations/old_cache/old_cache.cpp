// Old caching attempt
#include "old_cache.h"

CacheValue::CacheValue()
{
}

Cache::Cache(const set<string>::iterator &it)
{
    requests = 0;
    hits = 0;
    doc_words_begin=it;
}

CacheValue Cache::getHammingDistance(const string &query_word)
{
    requests++;
    auto it = hamming_map.find(query_word);
    if(it != hamming_map.end()){
        hits++;
        return it->second;
    }
    return CacheValue(DEFAULT_DISTANCE, doc_words_begin);
}

void Cache::addHammingDistance(const string &query_word, const unsigned int dist, const set<string>::iterator &it)
{
    hamming_map[query_word] = CacheValue(dist, it);
}

CacheValue Cache::getEditDistance(const string &query_word)
{
    requests++;
    auto it = edit_map.find(query_word);
    if(it != edit_map.end()){
        hits++;
        return it->second;
    }
    return CacheValue(DEFAULT_DISTANCE, doc_words_begin);
}

void Cache::addEditDistance(const string &query_word, const unsigned int dist, const set<string>::iterator &it)
{
    edit_map[query_word] = CacheValue(dist, it);
}

float Cache::hitRate()
{
    if(requests == 0) return 0;
    return (float) hits / (float) requests;
}