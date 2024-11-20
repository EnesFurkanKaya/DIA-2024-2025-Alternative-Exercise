
#ifndef __SIGMOD_QUERIES_H_
#define __SIGMOD_QUERIES_H_

#include <map>
#include <core.h>
#include <set>
#include <string>

using namespace std;

struct Query
{
	MatchType match_type;
	unsigned int match_dist;
	set<string> words;


	Query();
	Query(MatchType match_type, unsigned int match_dist, set<string>& words);
	Query(MatchType match_type, unsigned int match_dist, string& words);


	bool operator<(const Query& other) const {
        if(match_type < other.match_type)return true;
		else if (match_type > other.match_type) return false;
		else if(match_dist < other.match_dist)return true;
		else if (match_dist > other.match_dist) return false;
		else return words < other.words;
	}
};

class Queries{
private:
    map<QueryID,Query> id_to_query;
    map<Query,set<QueryID>> query_to_queryIDs;
	set<Query> queries; 

public:

	void add(const QueryID& id, const Query& query);

	void remove(const QueryID& id);

	set<QueryID>& getIDs(const Query& query);

	Query& getQuery(const QueryID& id);

	set<Query>& getAllQuerys();
};

#endif