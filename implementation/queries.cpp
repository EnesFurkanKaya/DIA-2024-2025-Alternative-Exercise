#include <map>
#include <core.h>
#include <set>
#include <string>
#include <sstream>

#include "queries.h"
using namespace std;
	
	void Queries::add(const QueryID& id, const Query& query){
		Queries::query_to_queryIDs[query].insert(id);
		Queries::id_to_query[id] = query;
		Queries::queries.insert(query);
	}

	void Queries::remove(const QueryID& id){
		Query& query = Queries::id_to_query[id];
		set<QueryID>& ids = Queries::query_to_queryIDs[query];
		Queries::id_to_query.erase(Queries::id_to_query.find(id));
		ids.erase(ids.find(id));
	}

	set<QueryID>& Queries::getIDs(const Query& query){
		return Queries::query_to_queryIDs[query];
	}

	void Queries::addIDs(set<QueryID>& queries, const Query& query){
		set<QueryID>& new_queries = Queries::query_to_queryIDs[query];
		queries.insert(new_queries.begin(), new_queries.end());
	}

	Query& Queries::getQuery(const QueryID& id){
		return Queries::id_to_query[id];
	}

	set<Query>& Queries::getAllQuerys(){
		return Queries::queries;
	}

    unsigned int Queries::size()
    {
        return Queries::queries.size();
    }

    Query &Queries::getQueryByIndex(const unsigned int index)
    {
    	auto it = Queries::queries.begin();
    	std::advance(it, index);
    	return const_cast<Query&>(*it);
    }

    Query::Query()
    {
    }

    Query::Query(MatchType match_type, unsigned int match_dist, set<string>& words)
    {
		Query::match_type = match_type;
		Query::match_dist = match_dist;
		Query::words = words;
    }

    Query::Query(MatchType match_type, unsigned int match_dist, string& words)
    {
		Query::match_type = match_type;
		Query::match_dist = match_dist;
		istringstream stream(words);
    	string word;

    	while (stream >> word) {
        	Query::words.insert(word);
    	}
    }