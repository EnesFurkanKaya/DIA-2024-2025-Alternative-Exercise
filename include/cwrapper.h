#include <string>
#include <set>
#include <sstream>
#include <cache.h>
#include <queries.h>
#include <unordered_set>

extern "C" {
void* doc_str_to_doc_words(const char* doc_str);

void* init_cache(void *document_words);

void* init_ids();

unsigned int queries_size(void *queries);

void* query_by_index(void *queries, unsigned int index);

bool match_query(void* queries, unsigned int index, void* document_words, void* cache);

void add_ids(void* queries, unsigned int index, void* new_ids);

unsigned int ids_to_array(void* ids, unsigned int** result_array);

void start_query(void* queries, QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist);

void end_query(void* queries, QueryID query_id);

void* init_queries();

bool match_query_og(void *queries, unsigned int index, void *document_words);

bool matchQuery3(const Query& query, const unordered_set<string>& document_words);

void* InitializeCache();

void freeCache(void* cache);

void *doc_str_to_doc_words_unorderedset(const char *doc_str);

bool match_query_caching(void *queries, unsigned int index, void *document_words, void* cache);

bool matchQuery_caching(const Query& query, const unordered_set<string>& document_words, void* cache);
}