#include "../include/core.h"
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <unordered_set>
#include <sstream>
using namespace std; 

int min(int a, int b, int c) {
    if (a <= b && a <= c) return a;
    if (b <= a && b <= c) return b;
    return c;
}

// Computes edit distance between two strings
unsigned int table[MAX_WORD_LENGTH+1];
unsigned int EditDistance(string a, string b){

    for(unsigned int i = 0; i <= a.size(); i++){
		table[i]=i;
	}

	unsigned int left = 0;
	unsigned int cur;
	for(unsigned int i = 1; i <= b.size(); i++){
		left = i;
		for(unsigned int j = 1; j <= a.size(); j++){
			if(a[j - 1] == b[i - 1]){
				cur = table[j - 1];
			} else {
				cur = std::min(std::min(table[j-1] + 1, table[j] + 1), left + 1);
			}
			table[j-1] = left;
			left = cur;
		}
		table[a.size()]=left;
	}
	return left;
}

// Computes hamming distance between two strings with equal length
unsigned int HammingDistance(string a, string b){

    if (a.size() != b.size()) return INT_MAX;

    unsigned int distance = 0;
    for(int i =0; i < a.size(); i++){
        if(a[i] != b[i]){
            distance++;
        }
    }
    return distance;
}

struct Query {
    unsigned int query_id;
    MatchType match_type;
    vector<string> keywords;
    int distance;
};

struct Document { 
    unsigned int doc_id;
    vector<unsigned int> query_ids;
    unsigned int num_matches;
};

ErrorCode InitializeIndex(){return EC_SUCCESS;}
ErrorCode DestroyIndex(){return EC_SUCCESS;}

vector<Query> active_queries;
vector<Document> all_documents;

// ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)	
ErrorCode StartQuery(QueryID id, const char* query_str, MatchType match_type, unsigned int match_dist){
    Query new_query;

    istringstream ss(query_str);
    string keyword;

    while(ss >> keyword){
        new_query.keywords.push_back(keyword);
    }

    new_query.query_id = id;
    new_query.match_type = match_type;
    new_query.distance = match_dist;

    active_queries.push_back(new_query);

    return EC_SUCCESS;
}

// Remove this query from the active query set
ErrorCode EndQuery(QueryID id){

    auto end_query = find_if(active_queries.begin(), active_queries.end(), [id](const Query& query){
        return query.query_id == id;
        });

    if (end_query != active_queries.end()){
        active_queries.erase(end_query);
    }

    return EC_SUCCESS;
}

// Match documents with each active query
ErrorCode MatchDocument(DocID doc_id, const char* doc_str){

    Document new_document;
    new_document.doc_id = doc_id;
    new_document.num_matches = 0;

    // tokenize the query
    vector<string> doc_tokens;
    istringstream doc_stream(doc_str);
    string token;
    while(doc_stream >> token){
        doc_tokens.push_back(token);
    }

    for(const Query& query : active_queries){
        bool perfect_match = true;
        //cout << "Matching the query: " << query.query_id << "\n";
        for(const string& keyword : query.keywords){
            bool match = false;
            //cout << "Matching the word: " << keyword << "\n";
            if(query.match_type == MT_EXACT_MATCH){
                match = find(doc_tokens.begin(), doc_tokens.end(), keyword) != doc_tokens.end();
            }
            else if(query.match_type == MT_EDIT_DIST){
                // Check all strings inside the document and compare them with the query. 1 if any match is found
                match = any_of(doc_tokens.begin(), doc_tokens.end(), [&](const string &doc_word){
                    return EditDistance(doc_word, keyword) <= query.distance;
                });
            }
            else{
                // Check all strings inside the document and compare them with the query. 1 if any match is found
                match = any_of(doc_tokens.begin(), doc_tokens.end(), [&](const string &doc_word){
                    return HammingDistance(doc_word, keyword) <= query.distance;
                });
            }
            if (!match){
                //cout << "Word " << keyword << " is NOT satisfied. \n";
                perfect_match = false;
                break;
            }
            else{
                //cout << "Word " << keyword << " is satisfied. \n";
            }
        }

        if(perfect_match){
            //cout << "All words inside the query " << query.query_id << " are perfectly satisfied. \n";
            new_document.query_ids.push_back(query.query_id);
        }
    }

    all_documents.push_back(new_document);
    return EC_SUCCESS;
}

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids) {
    if (all_documents.empty()) return EC_NO_AVAIL_RES;

    Document doc = all_documents.front();
    *p_query_ids = new unsigned int[doc.query_ids.size()];
    copy(doc.query_ids.begin(), doc.query_ids.end(), *p_query_ids); // Copy data from the vector
    *p_num_res = doc.query_ids.size();
    *p_doc_id = doc.doc_id;

    all_documents.erase(all_documents.begin());

    return EC_SUCCESS;
}