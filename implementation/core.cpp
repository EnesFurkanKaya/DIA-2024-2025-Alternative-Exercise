/*
 * core.cpp version 1.0
 * Copyright (c) 2013 KAUST - InfoCloud Group (All Rights Reserved)
 * Author: Amin Allam
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

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

//LFUCache lfu_cache;
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
 */
bool matchQuery(const Query& query, const set<string>& doc_words, Cache& cache){
	switch (query.match_type){
		case MT_EXACT_MATCH:
			for(const string& query_word: query.words){
				if (doc_words.find(query_word) == doc_words.end()) {
					return false;
				}
			}
			return true;
		case MT_HAMMING_DIST:
			for(const string& query_word: query.words){
				unsigned int dist;

				//cache lookup
				Value cacheValue = cache.getHammingDistance(query_word);
				dist = cacheValue.distance;
				auto it = cacheValue.next_doc_word;
				if(dist <= query.match_dist) continue;
				if(it == doc_words.end()) return false;

				//has to be computed
				for(;it!=doc_words.end(); ++it) {
					dist = min(dist, hammingDistance(query_word, it->data()));
					if(dist <= query.match_dist) break;
				}
				cache.addHammingDistance(query_word, dist, it);
				if(dist > query.match_dist)return false;
			} 
			return true;
		case MT_EDIT_DIST:
			for(const string& query_word: query.words){
				unsigned int dist;

				//cache lookup
				Value cacheValue = cache.getEditDistance(query_word);
				dist = cacheValue.distance;
				auto it = cacheValue.next_doc_word;
				if(dist <= query.match_dist) continue;
				if(it == doc_words.end()) return false;
	
				//has to be computed
				for(;it!=doc_words.end(); ++it) {
					dist = min(dist, editDistance(query_word, it->data()));
					if(dist <= query.match_dist) break;
				}
				cache.addEditDistance(query_word, dist, it);
				if(dist > query.match_dist)return false;
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
	set<string> doc_words;
	string_to_words(cur_doc_str, doc_words);

	Cache cache = Cache(doc_words.begin());
	//matching
	set<QueryID> ids;
	for(const Query &query: queries.getAllQuerys()){
		if(matchQuery(query, doc_words, cache)){
			set<QueryID> &set_ids = queries.getIDs(query);
			ids.insert(set_ids.begin(), set_ids.end());
		}
	}

	//create document object
	Document doc;
	doc.doc_id=doc_id;
	doc.num_res=ids.size();
	doc.query_ids = (QueryID*) malloc(doc.num_res * sizeof(QueryID));
	unsigned int i = 0;
    for (const QueryID& id : ids) {
        doc.query_ids[i++] = id;
    }
	docs.push(doc);
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Get the first undeliverd resuilt from "docs" and return it
ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids)
{	
	*p_doc_id=0; *p_num_res=0; *p_query_ids=0;
	if(docs.empty()) return EC_NO_AVAIL_RES;
	
	Document doc = docs.front();
	*p_doc_id=doc.doc_id;
	*p_num_res= doc.num_res;
	*p_query_ids=doc.query_ids;
	
	docs.pop();
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
