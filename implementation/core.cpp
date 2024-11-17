#include "../include/core.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <iostream>
#include <unordered_set>
using namespace std;

class TrieNode {
public:
    unordered_map<char, TrieNode*> children;
    vector<QueryID> query_ids;
    bool is_end_of_word;

    TrieNode() : is_end_of_word(false) {}
};

class Trie {
public:
    TrieNode* root;

    Trie() {
        root = new TrieNode();
    }

    void insert(const string& word, QueryID query_id) {
        TrieNode* node = root;
        for (char ch : word) {
            if (node->children.find(ch) == node->children.end()) {
                node->children[ch] = new TrieNode();
            }
            node = node->children[ch];
        }
        node->is_end_of_word = true;
        node->query_ids.push_back(query_id);
    }

    vector<QueryID> search(const string& word) {
        TrieNode* node = root;
        for (char ch : word) {
            if (node->children.find(ch) == node->children.end()) {
                return {};
            }
            node = node->children[ch];
        }
        return node->is_end_of_word ? node->query_ids : vector<QueryID>{};
    }

    void print_words(TrieNode* node, string prefix) {
        if (node->is_end_of_word) {
            cout << "Word: " << prefix << ", Query IDs: ";
            for (auto qid : node->query_ids) {
                cout << qid << " ";
            }
            cout << endl;
        }
        for (auto& child : node->children) {
            print_words(child.second, prefix + child.first);
        }
    }

    void print_all_words() {
        print_words(root, "");
    }
};
Trie trie;

int EditDistance(char* a, int na, char* b, int nb) {
    int oo = 0x7FFFFFFF;
    static int T[2][MAX_WORD_LENGTH + 1];
    int ia, ib;
    int cur = 0;
    ia = 0;

    for (ib = 0; ib <= nb; ib++)
        T[cur][ib] = ib;

    cur = 1 - cur;

    for (ia = 1; ia <= na; ia++) {
        for (ib = 0; ib <= nb; ib++)
            T[cur][ib] = oo;

        int ib_st = 0;
        int ib_en = nb;

        if (ib_st == 0) {
            ib = 0;
            T[cur][ib] = ia;
            ib_st++;
        }

        for (ib = ib_st; ib <= ib_en; ib++) {
            int ret = oo;

            int d1 = T[1 - cur][ib] + 1;
            int d2 = T[cur][ib - 1] + 1;
            int d3 = T[1 - cur][ib - 1]; if (a[ia - 1] != b[ib - 1]) d3++;

            if (d1 < ret) ret = d1;
            if (d2 < ret) ret = d2;
            if (d3 < ret) ret = d3;

            T[cur][ib] = ret;
        }

        cur = 1 - cur;
    }

    int ret = T[1 - cur][nb];

    return ret;
}

unsigned int HammingDistance(char* a, int na, char* b, int nb) {
    int j, oo = 0x7FFFFFFF;
    if (na != nb) return oo;

    unsigned int num_mismatches = 0;
    for (j = 0; j < na; j++) if (a[j] != b[j]) num_mismatches++;

    return num_mismatches;
}

struct Query {
    QueryID query_id;
    char str[MAX_QUERY_LENGTH];
    MatchType match_type;
    unsigned int match_dist;
    string query_str;
};

struct Document {
    DocID doc_id;
    unsigned int num_res;
    QueryID* query_ids;
};

vector<Query> queries;
vector<Document> docs;

string clean_word(const string &word) {
    string cleaned;
    for (char ch : word) {
        if (isalpha(ch)) {
            cleaned += tolower(ch);
        }
    }
    return cleaned;
}

ErrorCode InitializeIndex() { 
    trie = Trie();  // Reinitialize the Trie
    queries.clear();
    docs.clear();
    return EC_SUCCESS; 
}

ErrorCode DestroyIndex() {
    for (auto& doc : docs) {
        if (doc.query_ids) free(doc.query_ids);
    }
    docs.clear();
    queries.clear();
    return EC_SUCCESS;
}

ErrorCode EndQuery(QueryID query_id) {
    unsigned int i, n = queries.size();
    for (i = 0; i < n; i++) {
        if (queries[i].query_id == query_id) {
            queries.erase(queries.begin() + i);
            break;
        }
    }
    return EC_SUCCESS;
}

ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist) {
    Query query;
    query.query_id = query_id;
    strcpy(query.str, query_str);
    query.match_type = match_type;
    query.match_dist = match_dist;
    query.query_str = string(query_str);

    queries.push_back(query);

    // Split the query string into words and insert each word into the trie
    istringstream iss(query.query_str);
    string word;
    while (iss >> word) {
        word = clean_word(word);
        trie.insert(word, query_id);
    }

    return EC_SUCCESS;
}

ErrorCode MatchDocument(DocID doc_id, const char* doc_str) {
    char cur_doc_str[MAX_DOC_LENGTH];
    strcpy(cur_doc_str, doc_str);

    unordered_set<unsigned int> unique_query_ids;
    istringstream iss(cur_doc_str);
    string word;
    vector<QueryID> query_ids;

    // Mark array to keep track of matched queries
    bool mark[1000000] = {0}; // Adjust size as needed

    while (iss >> word) {
        word = clean_word(word); // Clean the word before searching
        vector<QueryID> matched_query_ids = trie.search(word);
        for (auto qid : matched_query_ids) {
            mark[qid] = true;
        }
    }

    // Collect all marked query IDs
    for (unsigned int i = 0; i < 1000000; ++i) { // Adjust size as needed
        if (mark[i]) {
            query_ids.push_back(i);
        }
    }

    // Sort the query IDs in ascending order
    sort(query_ids.begin(), query_ids.end());

    Document doc;
    doc.doc_id = doc_id;
    doc.num_res = query_ids.size();
    doc.query_ids = nullptr;
    if (doc.num_res) {
        doc.query_ids = (unsigned int*)malloc(doc.num_res * sizeof(unsigned int));
        for (unsigned int i = 0; i < doc.num_res; i++) {
            doc.query_ids[i] = query_ids[i];
        }
    }
    docs.push_back(doc);

    return EC_SUCCESS;
}

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids) {
    if (docs.empty()) return EC_NO_AVAIL_RES;

    Document doc = docs.front();
    *p_doc_id = doc.doc_id;
    *p_num_res = doc.num_res;
    *p_query_ids = doc.query_ids;

    // Debug print to check the results being retrieved
    cout << "Retrieving results for Document ID: " << *p_doc_id << endl;
    cout << "Number of results: " << *p_num_res << endl;
    cout << "Query IDs: ";
    for (unsigned int i = 0; i < *p_num_res; i++) {
        cout << (*p_query_ids)[i] << " ";
    }
    cout << endl;

    // Remove the document from the list
    docs.erase(docs.begin());

    // Free memory after returning the result
    if (doc.query_ids) free(doc.query_ids);

    return EC_SUCCESS;
}

