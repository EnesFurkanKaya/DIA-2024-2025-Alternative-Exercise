#include "cwrapper.h"
#include <vector>
#include <string>
#include <set>
#include <cstring>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <math.h>

int max_size = 1000;

// Initialize the QueryCache
void* initializeCache() {
    return static_cast<void*>(new FrequencyCache(max_size));
}

// Free the allocated memory for the QueryCache
void freeCache(void* cache){
    if (cache != nullptr) {
        auto actual_cache = static_cast<FrequencyCache*>(cache);// Cast void* back to QueryCache*
        actual_cache->clear();  // Clear all the stored keys
        delete actual_cache;    // Free the allocated memory for the cache
    }
}

// Transfrom the document from char* to (void*)&std::unordered_set<std::string>
void *doc_str_to_doc_words_unorderedset(const char *doc_str)
{
    std::string cur_doc_str(doc_str);
	std::unordered_set<std::string>* document_words = new std::unordered_set<std::string>();

    std::istringstream stream(cur_doc_str);
    std::string word;

    while (stream >> word) {
        (*document_words).insert(word);
    }
    return (void*) document_words;
}

// Transfrom the document from char* to (void*)&std::set<std::string>
void *doc_str_to_doc_words(const char *doc_str)
{
    std::string cur_doc_str(doc_str);
	std::set<std::string>* document_words = new std::set<std::string>();

    std::istringstream stream(cur_doc_str);
    std::string word;

    while (stream >> word) {
        (*document_words).insert(word);
    }
    return (void*) document_words;
}

// Return a new set with c++ type (void*)&std::set<QueryID>
void *init_ids()
{
    set<QueryID>* ids = new set<QueryID>();
    return (void*) ids;
}

// Return the number of unique queries in Queries
unsigned int queries_size(void *queries)
{
    Queries* qs = (Queries*) queries;
    return qs->size();
}

// Get the query by index
void* query_by_index(void *queries, unsigned int index)
{
    return (void*) &(((Queries*) queries)->getQueryByIndex(index));
}

// Main Match Query-Document function for core.py
bool matchQuery(const Query& query, const set<string>& document_words){
    bool match;
    switch (query.match_type){
        case MT_EXACT_MATCH:
            for(const auto& query_word: query.words){
                if (document_words.find(query_word) == document_words.end()) {
                    return false;
                }
            }
            return true;
        case MT_HAMMING_DIST:
            for(const auto& query_word: query.words){
                match = false;
                for(const auto& doc_word: document_words){
                    if(hammingDistance(query_word, doc_word) <= query.match_dist){
                        match = true;
                        break;
                    }
                }
                if(!match) return false;
            } 
            return true;
            break;
        case MT_EDIT_DIST:
            for(const auto& query_word: query.words){
                match = false;
                for(const auto& doc_word: document_words){
                    if(editDistance(query_word, doc_word) <= query.match_dist){
                        match = true;
                        break;
                    }
                }
                if(!match) return false;
            }
            return true;
            break;
    }
    perror("query match type not allowed! ");
    return false;
}

bool match_query(void *queries, unsigned int index, void *document_words){
    return matchQuery(*(Query*)query_by_index(queries, index), *static_cast<std::set<std::string>*>(document_words));
}

// Main Optimized Match Query-Document function for opt_core.py
bool matchQuery_caching(const Query& query, const unordered_set<string>& document_words, void* cache){
    auto actual_cache = static_cast<FrequencyCache*>(cache);   
    for (const auto& query_word : query.words){
        bool match = false;
        unordered_set<string> matched_words;
        WordCacheKey key{query_word, query.match_type, query.match_dist};
        // Look up for the key in the cache
        auto cached_value = actual_cache->get(key);
        

        if (cached_value) {
            for (const auto& doc_word : *cached_value) {
                // If the document word is already matched (valid), no need to check again
                if (document_words.find(doc_word) != document_words.end()) {
                    match = true;
                    break;
                }
            }
        }
        // If there was a match found from the cache, move to the next query word
        if (match){
            continue;
        }
        // If any cache-hit is not found, calculate the distance and store the matching words
        switch(query.match_type){
            case MT_EXACT_MATCH:
                if (document_words.find(query_word) != document_words.end()){
                    matched_words.insert(query_word);
                    match = true;
                }
                break;
            case MT_HAMMING_DIST:
                for (const auto& doc_word : document_words) {
                    if (hammingDistance(query_word, doc_word) <= query.match_dist) {
                        matched_words.insert(doc_word);
                        match = true;
                        break;
                    }
                }
                break;
            case MT_EDIT_DIST:
                for (const auto& doc_word : document_words) {
                    if (abs(static_cast<int>(query_word.size()) - static_cast<int>(doc_word.size())) > query.match_dist) continue;
                    if (editDistance(query_word, doc_word) <= query.match_dist) {
                        matched_words.insert(doc_word);
                        match = true;
                        break;
                    }
                }
                break;   
        }
        if (!match){
            return false;
        }
        actual_cache->insert(key, matched_words);
    }
    return true;
}

bool match_query_caching(void *queries, unsigned int index, void *document_words, void* cache){
    return matchQuery_caching(*(Query*)query_by_index(queries, index), *static_cast<std::unordered_set<std::string>*>(document_words), cache);
}

// Add new ids
void add_ids(void *queries, unsigned int index, void *new_ids)
{
    Queries& casted_queries = *static_cast<Queries*>(queries);
    set<QueryID>& casted_ids = *static_cast<set<QueryID>*>(new_ids);
    casted_queries.addIDs(casted_ids, casted_queries.getQueryByIndex(index));
}

// converts C++ type std::set<QueryID> to C type unsigned int**
unsigned int ids_to_array(void *ids, unsigned int** result_array)
{
    std::set<QueryID>& ids2= *static_cast<std::set<QueryID>*>(ids);
    QueryID* array = (QueryID*) malloc(ids2.size() * sizeof(QueryID));
    int i = 0;
    for(auto it = ids2.begin(); it!= ids2.end(); it++, i++){
        array[i]=*it;
    }
    *result_array=array;
    return i;
}

// Start Query
void start_query(void *queries, QueryID query_id, const char *query_str, MatchType match_type, unsigned int match_dist)
{
    Queries& casted_queries = *static_cast<Queries*>(queries); 
    string str(query_str);
	Query query(match_type, match_dist, str);
	casted_queries.add(query_id, query);
}

// End Query
void end_query(void *queries, QueryID query_id)
{
    Queries& casted_queries = *static_cast<Queries*>(queries); 
	casted_queries.remove(query_id);
}

// Initialize empty new Queries
void *init_queries()
{
	Queries* queries = new Queries;
    return (void*) queries;
}
