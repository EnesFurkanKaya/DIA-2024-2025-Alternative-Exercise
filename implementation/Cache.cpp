#include "Cache.h"

template <typename Key, typename Value>
LFUCache<Key, Value>::LFUCache()
{
    LFUCache::capacity = CACHE_SIZE;
    LFUCache::hits = 0;
    LFUCache::requests = 0;
}

template <typename Key, typename Value>
unsigned int LFUCache<Key, Value>::get(const Key &key)
{
    requests++;
    auto it = LFUCache::key_to_dist.find(key);
    if(it==LFUCache::key_to_dist.end()){
        return CACHE_DEFAULT;
    }else {
        hits++;
        LFUCache::update_freq(key);
        return LFUCache::key_to_dist[key];
    }
}

template <typename Key, typename Value>
void LFUCache<Key, Value>::add(const Key &key, const Value &value)
{
    if(LFUCache::key_to_dist.find(key) == LFUCache::key_to_dist.end()){
        if(LFUCache::key_to_dist.size()>=CACHE_SIZE){
            LFUCache::removeOneKey();
        }
        LFUCache::freq_to_key[1].insert(key);
        LFUCache::key_to_freq[key] = 1;
        LFUCache::key_to_dist[key] = value;
    }
}

template <typename Key, typename Value>
void LFUCache<Key, Value>::update_freq(const Key &key)
{
    unsigned int freq;
    if(LFUCache::key_to_freq.find(key)==LFUCache::key_to_freq.end()) freq = 0;
    else freq = LFUCache::key_to_freq[key];

    if (LFUCache::freq_to_key.find(freq) == LFUCache::freq_to_key.end()) {
        set<Key> new_key_set;
        LFUCache::freq_to_key[freq] = new_key_set;
    }
    set<Key>& keys = LFUCache::freq_to_key[freq];
    keys.erase(keys.find(key));

    if (LFUCache::freq_to_key.find(freq + 1) == LFUCache::freq_to_key.end()) {
        set<Key> new_key_set;
        LFUCache::freq_to_key[freq + 1] = new_key_set;
    }
    LFUCache::freq_to_key[freq + 1].insert(key);

    LFUCache::key_to_freq[key] = freq +1;
}

template <typename Key, typename Value>
void LFUCache<Key, Value>::removeOneKey()
{
    unsigned int i = 1;
    for(; i < LFUCache::key_to_dist.size(); i++){
        auto it = LFUCache::freq_to_key.find(i);
        if(it != LFUCache::freq_to_key.end()) break;
    }

    std::set<Key>& keys = LFUCache::freq_to_key[i];
    Key old_key = *keys.begin();
    keys.erase(keys.begin());
    LFUCache::key_to_dist.erase(LFUCache::key_to_dist.find(old_key));
    LFUCache::key_to_freq.erase(LFUCache::key_to_freq.find(old_key));
}

DocCache::DocCache()
{
}

unsigned int DocCache::get(string &query_word, MatchType matchtype)
{
    DocKey key;
    key.word = query_word;
    key.matchtype = matchtype;
    return LFUCache::get(key);
}

void DocCache::add(string &query_word, MatchType matchtype, unsigned int dist)
{
    DocKey key;
    key.word = query_word;
    key.matchtype = matchtype;
    LFUCache::add(key, dist);
}
