#ifndef CACHE_H
#define CACHE_H

#include <string>
#include "core.h"
#include <unordered_map>
#include <map>
#include <optional>
#include <set>
#include <distance.h>


using namespace std;

#define DEFAULT_DISTANCE INT32_MAX

/**
 * @brief Container for results for Cache get requests
 * 
 */
struct CacheValue{

    // the minimal distance between the key word and all words of the document compared so far
    unsigned int distance;

    //the last compared word in the document
    set<string>::iterator position; 

    CacheValue();
    CacheValue(const unsigned int dist, const set<string>::iterator &ndw):distance(dist),position(ndw){};
};

class Cache{
private:
    unordered_map<string, CacheValue> hamming_map; //stores Cache-elements for hamming distance
    unordered_map<string, CacheValue> edit_map; //stores Cache-elements for edit distance
    set<string>::iterator doc_words_begin; // its the value of CacheValue.position for an cache miss (has to be the begin of the document-word-set)

    unsigned int hits; //stores cache hits for the hitrate evaluation
    unsigned int requests; //stores all get requests for the hitrate evaluation

public:

    /**
     * @brief Construct a new Cache object
     * 
     * @param begin of the document (document_words.begin())
     */
    Cache(const set<string>::iterator &begin);

    /**
     * @brief Get
     * 1. minimal hamming distance of query_word and all words of the document compared so far 
     * 2. position of the last compared word of the document
     * 
     * @param query_word
     * @return CacheValue 
     */
    CacheValue getHammingDistance(const string &query_word);

    /**
     * @brief Add 
     *  1. minmal hamming distance of query_word and all words of the document compared so far
     *  2. position of the last compared word of the document to the cache
     * 
     * @param query_word that has to be compared with all words of the document
     * @param min_distance between document_words.begin() and position (inclusiv)
     * @param position of the last compared element in document_words
     */
    void addHammingDistance(const string &query_word, const unsigned int min_distance, const set<string>::iterator &position);

    /**
     * @brief Get
     * 1. minimal edit distance of query_word and all words of the document compared so far 
     * 2. position of the last compared word of the document
     * 
     * @param query_word
     * @return CacheValue 
     */
    CacheValue getEditDistance(const string &query_word);

    /**
     * @brief Add 
     *  1. minmal edit distance of query_word and all words of the document compared so far
     *  2. position of the last compared word of the document to the cache
     * 
     * @param query_word that has to be compared with all words of the document
     * @param min_distance between document_words.begin() and position (inclusiv)
     * @param position of the last compared element in document_words
     */
    void addEditDistance(const string &query_word, const unsigned int distance, const set<string>::iterator &position);

    /**
     * @brief successful get requests / all get requests
     * 
     * @return float
     */
    float hitRate();

};

#endif