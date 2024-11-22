#include "cache.h"

Cache::Cache(const set<string>::iterator &it)
{
    requests = 0;
    hits = 0;
    doc_words_begin=it;
}

Value Cache::getHammingDistance(const string &query_word)
{
    requests++;
    auto it = hamming_map.find(query_word);
    if(it != hamming_map.end()){
        hits++;
        return it->second;
    }
    return Value(CACHE_DEFAULT, doc_words_begin);
}

void Cache::addHammingDistance(const string &query_word, const unsigned int dist, const set<string>::iterator &it)
{
    hamming_map[query_word] = Value(dist, it);
}

Value Cache::getEditDistance(const string &query_word)
{
    requests++;
    auto it = edit_map.find(query_word);
    if(it != edit_map.end()){
        hits++;
        return it->second;
    }
    return Value(CACHE_DEFAULT, doc_words_begin);
}

void Cache::addEditDistance(const string &query_word, const unsigned int dist, const set<string>::iterator &it)
{
    edit_map[query_word] = Value(dist, it);
}

float Cache::hitRate()
{
    if(requests == 0) return 0;
    return (float) hits / (float) requests;
}

unsigned int Cache::upperBoundHammingDistance(const string &query_word)
{
    unsigned int dist = CACHE_DEFAULT;
    for(unsigned int i = 0; i< UPPER_BOUND_QUERYS && i < hamming_map.size(); i++){
        auto it = hamming_map.begin();
        dist = min(dist, hammingDistance(query_word, it->first) + it->second.distance);
    }   
    return dist;
}

unsigned int Cache::upperBoundEditDistance(const string &query_word)
{
    unsigned int dist = upperBoundHammingDistance(query_word);
    for(unsigned int i = 0; i< UPPER_BOUND_QUERYS && i < edit_map.size(); i++){
        auto it = edit_map.begin();
        dist = min(dist, editDistance(query_word, it->first) + it->second.distance);
    } 
    return dist;
}
