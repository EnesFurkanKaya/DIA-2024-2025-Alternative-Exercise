from enum import Enum

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
QueryID = NewType('QueryID', int)

# Document ID type.
DocumentID = NewType('DocumentID', int)


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

class Document:
    def __init__(self, id: DocumentID, num_res: int, query_ids: set[int]):
        self.doc_id = id
        self.num_res = num_res
        self.query_ids = query_ids


class Query:
    def __init__(self, match_type: MatchType, match_dist: int, query_str: Union[Set[str], str]):
        self.match_type = match_type
        self.match_dist = match_dist
        self.words = set()
        if isinstance(query_str, set):
            self.words = query_str
        elif isinstance(query_str, str):
            self.words = set(query_str.split())
    
    def operator(self, other):
        if self.match_type < other.match_type:
            return True
        elif self.match_type > other.match_type:
            return False
        elif self.match_dist < other.match_dist:
            return True
        elif self.match_dist > other.match_dist:
            return False
        else:
            return self.words < other.words

class Queries:
    def __init__(self):
        self.id_to_query = {}
        self.query_to_queryIDs = defaultdict(set)
        self.queries = set()
    
    def add(self, id: QueryID, query: Query):
        self.id_to_query[id] = query
        self.query_to_queryIDs[query].add(id)
        self.queries.add(query)
    
    def remove(self, id: QueryID):
        query = self.id_to_query.pop(id, None)
        if query:
            self.query_to_queryIDs[query].remove(id)
            self.queries.remove(query)
        
    def getIDs(self, query: Query):
        return self.query_to_queryIDs.get(query, set())
    
    def getQuery(self, id: QueryID):
        return self.id_to_query.get(id)
    
    def getAllQuerys(self):
        return self.queries
    

