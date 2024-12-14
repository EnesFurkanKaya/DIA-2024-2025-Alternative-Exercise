#include "cwrapper.h"

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

void *init_cache(void *document_words){
    std::set<std::string>* string_set = static_cast<std::set<std::string>*>(document_words);
    Cache* cache = new Cache((*string_set).begin());
    return (void*) cache;
}

void *init_ids()
{
    set<QueryID>* ids = new set<QueryID>();
    return (void*) ids;
}

unsigned int queries_size(void *queries)
{
    Queries* qs = (Queries*) queries;
    return qs->size();
}

void* query_by_index(void *queries, unsigned int index)
{
    return (void*) &(((Queries*) queries)->getQueryByIndex(index));
}

bool matchQuery2(const Query& query, const set<string>& document_words, Cache& cache){
	unsigned int distance;
	switch (query.match_type){
		case MT_EXACT_MATCH:
			for(const string& query_word: query.words){
				if (document_words.find(query_word) == document_words.end()) {
					return false;
				}
			}
			return true;
		case MT_HAMMING_DIST:
			for(const string& query_word: query.words){

				// cache lookup for minimal distance computed so far
				CacheValue cacheValue = cache.getHammingDistance(query_word);
				distance = cacheValue.distance;
				auto it = cacheValue.position;

				// checks if the cached distance is small enough
				if(distance <= query.match_dist) continue;

				// checks if for the cached value already the entire document is searched
				if(it == document_words.end()) return false;

				// compare query_word with the remaining part of the document and update cache
				for(;it!=document_words.end(); ++it) {
					distance = min(distance, hammingDistance(query_word, it->data()));
					if(distance <= query.match_dist) break;
				}
				cache.addHammingDistance(query_word, distance, it);

				// check if the minimal distance between query_word and any word of the document is small enough
				if(distance > query.match_dist)return false;
			} 
			return true;
		case MT_EDIT_DIST:
			for(const string& query_word: query.words){
	
				// cache lookup for minimal distance computed so far
				CacheValue cacheValue = cache.getEditDistance(query_word);
				distance = cacheValue.distance;
				auto it = cacheValue.position;

				// checks if for the cached value already the entire document is searched
				if(distance <= query.match_dist) continue;

				// compare query_word with the remaining part of the document and update cache
				if(it == document_words.end()) return false;
	
				// compare query_word with the remaining part of the document and update cache
				for(;it!=document_words.end(); ++it) {
					distance = min(distance, editDistance(query_word, it->data()));
					if(distance <= query.match_dist) break;
				}
				cache.addEditDistance(query_word, distance, it);

				// check if the minimal distance between query_word and any word of the document is small enough
				if(distance > query.match_dist)return false;
			}
			return true;
	}
	perror("query match type not allowed! ");
	return false;
}

bool match_query(void *queries, unsigned int index, void *document_words, void *cache)
{

    return matchQuery2(*(Query*)query_by_index(queries, index), *static_cast<std::set<std::string>*>(document_words), *static_cast<Cache*>(cache));

}

void add_ids(void *queries, unsigned int index, void *new_ids)
{
    Queries& casted_queries = *static_cast<Queries*>(queries);
    set<QueryID>& casted_ids = *static_cast<set<QueryID>*>(new_ids);
    casted_queries.addIDs(casted_ids, casted_queries.getQueryByIndex(index));
}

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

void start_query(void *queries, QueryID query_id, const char *query_str, MatchType match_type, unsigned int match_dist)
{
    Queries& casted_queries = *static_cast<Queries*>(queries); 
    string str(query_str);
	Query query(match_type, match_dist, str);
	casted_queries.add(query_id, query);
}

void end_query(void *queries, QueryID query_id)
{
    Queries& casted_queries = *static_cast<Queries*>(queries); 
	casted_queries.remove(query_id);
}

void *init_queries()
{
	Queries* queries = new Queries;
    return (void*) queries;
}
