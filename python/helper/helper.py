from dataclasses import dataclass
from enum import Enum
from sys import maxsize
from typing import NewType, Set, Union
from collections import defaultdict

# Maximum document length in characters.
MAX_DOC_LENGTH = 1 << 22

# Maximum word length in characters.
MAX_WORD_LENGTH = 31

# Minimum word length in characters.
MIN_WORD_LENGTH = 4

# Maximum number of words in a query.
MAX_QUERY_WORDS = 5

# Maximum query length in characters.
MAX_QUERY_LENGTH = ((MAX_WORD_LENGTH+1)*MAX_QUERY_WORDS)

# Query ID type.
QueryID = int

# Document ID type.
DocumentID = int


# Error codes:
class ErrorCode(Enum):
    EC_SUCCESS = 0
    EC_NO_AVAIL_RES = 1
    EC_FAIL = 2

# Matching types:
class MatchType(Enum):
    MT_EXACT_MATCH = 0
    MT_HAMMING_DIST = 1
    MT_EDIT_DIST = 2

@dataclass(frozen=True)
class Document:
    doc_id: DocumentID
    num_res: int
    query_ids: tuple[int]

@dataclass(frozen=True)
class Query:
    match_type: MatchType
    match_dist: int
    words: tuple[str]

class Queries:
    id_to_query: dict[QueryID, Query]
    query_to_queryIDs: defaultdict[Query, set[QueryID]]

    def __init__(self):
        self.id_to_query = {}
        self.query_to_queryIDs = defaultdict(set)
        self.queries = set()
    
    def add(self, id: QueryID, query: Query) -> None:
        self.id_to_query[id] = query
        self.query_to_queryIDs[query].add(id)
        self.queries.add(query)
    
    def remove(self, id: QueryID) -> None:
        query: Query = self.id_to_query.pop(id, None)
        if (query != None):
            queryIDs: set[QueryID] = self.query_to_queryIDs.get(query)
            queryIDs.remove(id)
            if(len(queryIDs)==0):
                self.queries.remove(query)
        
    def getIDs(self, query: Query) -> set[QueryID]:
        return self.query_to_queryIDs.get(query, set())
    
    def getQuery(self, id: QueryID) -> Query:
        return self.id_to_query.get(id)
    
    def getAllQuerys(self) -> set[Query]:
        return self.queries
    

