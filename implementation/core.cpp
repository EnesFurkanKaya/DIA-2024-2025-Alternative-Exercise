// Old solution without cwrappers pure python

#include "../include/core.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>
#include <climits>
#include <iostream>
#include <sstream>
#include <set>
#include <algorithm>
#include <queue>

#include "queries.h"
#include <cache.h>
#include <distance.h>

using namespace std;
///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all query ID results associated with a dcoument
struct Document
{
	DocID doc_id;
	unsigned int num_res;
	QueryID* query_ids;
};

///////////////////////////////////////////////////////////////////////////////////////////////

// Keeps all currently active queries
Queries queries;

// Keeps all currently available results that has not been returned yet
queue<Document> docs;

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode InitializeIndex(){return EC_SUCCESS;}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode DestroyIndex(){return EC_SUCCESS;}

///////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Converts a string consisting of multiple words separated by spaces into a list of strings,
 * where each element is a single word.
 */
void string_to_words(const string& str, set<string>& words) {
    istringstream stream(str);
    string word;

    while (stream >> word) {
        words.insert(word);
    }
}

ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
{
	string str(query_str);
	Query query(match_type, match_dist, str);
	queries.add(query_id, query);

	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Remove this query from the active query set
ErrorCode EndQuery(QueryID query_id)
{
	queries.remove(query_id);
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief checks if every word in the query does match any word in the document
 * 
 * @return true if all words in query match any word in document,
 * @return false otherwise
 */
bool matchQuery(const Query& query, const set<string>& document_words, Cache& cache){
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

ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{

	//transform one big document string to set of word-strings
	string cur_doc_str(doc_str);
	set<string> document_words;
	string_to_words(cur_doc_str, document_words);

	//init cache
	Cache cache = Cache(document_words.begin());


	//match all queries
	set<QueryID> ids;
	for(const Query &query: queries.getAllQuerys()){

		//match query (true for match and false for no match)
		if(matchQuery(query, document_words, cache)){

			//store query_ids of matching queries
			set<QueryID> &set_ids = queries.getIDs(query);
			ids.insert(set_ids.begin(), set_ids.end());
		}
	}

	//create document object
	Document document;
	document.doc_id=doc_id;
	document.num_res=ids.size();
	document.query_ids = (QueryID*) malloc(document.num_res * sizeof(QueryID));
	unsigned int i = 0;
    for (const QueryID& id : ids) {
        document.query_ids[i++] = id;
    }

	//add document
	docs.push(document);

	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Get the first undeliverd resuilt from "docs" and return it
ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids)
{	
	*p_doc_id=0; *p_num_res=0; *p_query_ids=0;
	if(docs.empty()) return EC_NO_AVAIL_RES;
	
	Document document = docs.front();
	*p_doc_id=document.doc_id;
	*p_num_res= document.num_res;
	*p_query_ids=document.query_ids;
	
	docs.pop();
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
