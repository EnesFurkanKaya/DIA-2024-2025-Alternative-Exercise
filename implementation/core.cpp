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

// Computes edit distance between two strings - levenshtein
// Complexity: O(n * m)
unsigned int EditDistance(const char* a, const char* b){
    
    int a_len = strlen(a);
    int b_len = strlen(b);

    int **distance = (int **)malloc((a_len + 1) * sizeof(int *));
    for (int i = 0; i <= a_len; i++) {
        distance[i] = (int *)malloc((b_len + 1) * sizeof(int));
    }

    // Case: |b| = 0
    for (int i=0; i<= a_len; i++){
        distance[i][0] = i;
    }
    // Case: |a| = 0
    for (int i=0; i<= b_len; i++){
        distance[0][i] = i;
    }

    for (int i = 1; i <= a_len; i++){
        for (int j = 1; j <= b_len; j++){
            int minimum = min(distance[i-1][j], distance[i][j-1], distance[i-1][j-1]);
            // Case: head(a) = head(b)
            if (a[i-1] == b[j-1]){
                distance[i][j] = minimum;
            }
            // Other 3 cases:
            else{
                distance[i][j] = minimum + 1;
            }
        }
    }

    int result = distance[a_len][b_len];

    for (int i = 0; i <= a_len; i++) {
        free(distance[i]);
    }
    free(distance);

    return result;
}

// Computes hamming distance between two strings with equal length - levenshtein
unsigned int HammingDistance(const char* a, const char* b){
    int a_len = strlen(a);
    int b_len = strlen(b);

    if (a_len != b_len){
        if(a_len > b_len){
            return a_len;
        }
        return b_len;
    }
    unsigned int distance = 0;
    for(int i =0; i < a_len; i++){
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
    unsigned int* query_ids;
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
    std::string keyword;

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
    new_document.query_ids = nullptr;
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
        cout << "Matching the query: " << query.query_id << "\n";
        for(const string& keyword : query.keywords){
            bool match = false;
            cout << "Matching the word: " << keyword << "\n";
            if(query.match_type == MT_EXACT_MATCH){
                match = find(doc_tokens.begin(), doc_tokens.end(), keyword) != doc_tokens.end();
            }
            else if(query.match_type == MT_EDIT_DIST){
                // Check all strings inside the document and compare them with the query. 1 if any match is found
                match = any_of(doc_tokens.begin(), doc_tokens.end(), [&](const string &doc_word){
                    return EditDistance(doc_word.c_str(), keyword.c_str()) <= query.distance;
                });
            }
            else{
                // Check all strings inside the document and compare them with the query. 1 if any match is found
                match = any_of(doc_tokens.begin(), doc_tokens.end(), [&](const string &doc_word){
                    return HammingDistance(doc_word.c_str(), keyword.c_str()) <= query.distance;
                });
            }
            if (!match){
                cout << "Word " << keyword << " is NOT satisfied. \n";
                perfect_match = false;
                break;
            }
            else{
                cout << "Word " << keyword << " is satisfied. \n";
            }
        }

        if(perfect_match){
            cout << "All words inside the query " << query.query_id << " is perfectly satisfied. \n";
            new_document.query_ids = (unsigned int*)realloc(new_document.query_ids, (new_document.num_matches + 1) * sizeof(unsigned int));
            new_document.query_ids[new_document.num_matches] = query.query_id;
            new_document.num_matches++;
        }
    }

    all_documents.push_back(new_document);
    return EC_SUCCESS;
}

/*
ErrorCode GetNextAvailRes(){

}
*/

void TestStartQuery() {
    cout << "Testing StartQuery...\n";
    // Start a few queries
    StartQuery((QueryID)1, "apple fruit orange", MT_EDIT_DIST, (unsigned int)0);
    StartQuery((QueryID)2, "apple banana", MT_EDIT_DIST, (unsigned int)2);

    // Test that the queries were added
    assert(active_queries.size() == 2);
    assert(active_queries[0].query_id == 1);
    assert(active_queries[1].query_id == 2);

    cout << "Active Query 1:\n";
    for (const string &keyword : active_queries[0].keywords) {
        cout << keyword << " ";
    }
    cout << "\n";

    cout << "Active Query 2:\n";
    for (const string &keyword : active_queries[1].keywords) {
        cout << keyword << " ";  // Print each keyword separated by a space
    }
    cout << "\n";

    cout << "StartQuery passed!\n";
}

void TestEndQuery() {
    cout << "Testing EndQuery...\n";
    // End a query
    EndQuery((QueryID)2);

    // Test that the query was removed
    assert(active_queries.size() == 1);
    assert(active_queries[0].query_id == 1);
    
    cout << "EndQuery passed!\n";
}

void TestMatchDocument() {
    cout << "Testing MatchDocument...\n";
    // Test matching with some documents
    MatchDocument((DocID)1001, "apple orange fruit");
    cout << "Document 1001 matched.\n"; 
    MatchDocument((DocID)1002, "apple fruitt oorange");

    // Test that documents were added to the all_documents vector
    assert(all_documents.size() == 2);
    assert(all_documents[0].doc_id == 1001);
    assert(all_documents[1].doc_id == 1002);

    // Check if the correct query IDs are associated with each document
    assert(all_documents[0].num_matches > 0);  // First document should have at least one match
    assert(all_documents[1].num_matches > 0);  // Second document should have at least one match

    cout << "MatchDocument passed!\n";
}

int main(){
    // Clear vectors before starting tests
    active_queries.clear();
    all_documents.clear();

    // Run tests
    TestStartQuery();
    TestEndQuery();
    TestMatchDocument();

    std::cout << "All tests passed successfully!\n";
    return 0;
}


