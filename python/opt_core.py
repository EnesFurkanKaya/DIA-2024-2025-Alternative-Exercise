# Exercise 1 - Reimplementation for exercise 3

#######################################################
###################### Libraries ######################

from ctypes import POINTER, c_bool, c_char_p, c_uint, c_void_p
import ctypes
from helper.helper import *

#######################################################
################# Global Variables ###################

#Keeps all currently active queries
queries: c_void_p
# Keeps all currently available results that has not been returned yet
docs: list[Document] = []
core = ctypes.CDLL("../libcore.so")
core.doc_str_to_doc_words.restype = c_void_p
core.doc_str_to_doc_words.argtypes = [c_char_p]

core.init_cache.restype = c_void_p
core.init_cache.argtypes = [c_void_p]

core.init_ids.restype = c_void_p

core.queries_size.restype = c_uint
core.queries_size.argtypes = [c_void_p]

core.query_by_index.restype = c_void_p
core.query_by_index.argtypes = [c_void_p, c_uint]

core.match_query.restype = c_bool
core.match_query.argtypes = [c_void_p, c_uint, c_void_p, c_void_p]

core.add_ids.restype = None
core.add_ids.argtypes = [c_void_p, c_uint, c_void_p]

core.ids_to_array.restype = c_uint
core.ids_to_array.argtypes = [c_void_p, POINTER(POINTER(c_uint))]

core.start_query.restype = None
core.start_query.argtypes = [c_void_p, c_uint, c_char_p, c_uint, c_uint]

core.end_query.restype = None
core.end_query.argtypes = [c_void_p, c_uint]

core.init_queries.restype = c_void_p

#######################################################
################### Helper Functions ##################
def InitializeIndex():
    global queries
    queries = core.init_queries()
    return ErrorCode.EC_SUCCESS

def DestroyIndex():
	return ErrorCode.EC_SUCCESS

def editDistance(a: str, b: str) -> int:
    table = [e for e in range(len(a)+1)]
    left = 0
    for i in range(1, len(b)+1):
        left = i
        for j in range(1, len(a)+1):
            if(a[j - 1] == b[i - 1]):
                cur = table[j - 1]
            else:
                cur = min(min(table[j-1] + 1, table[j] + 1), left + 1)
            table[j-1] = left
            left = cur
        table[len(a)]=left
    return left

def hammingDistance(a: str, b: str) -> int:
    if(len(a)!=len(b)):
        return maxsize
    sum = 0
    for i in range(len(a)):
        if(a[i]!=b[i]):
            sum+=1
    return sum

def matchQuery(query: Query, doc_words: list[str]) -> bool:
    if(query.match_type.value == MatchType.MT_EXACT_MATCH.value):
        # Return true if all words in doc_words exact match with a word in query.words
        return all(any(doc_word == query_word for doc_word in doc_words) for query_word in query.words)
    elif(query.match_type.value == MatchType.MT_HAMMING_DIST.value):
    	# Return true if all words in query.words hamming match with any word in doc_words
        return all(any(hammingDistance(query_word, doc_word) <= query.match_dist for doc_word in doc_words) for query_word in query.words)
    elif(query.match_type.value == MatchType.MT_EDIT_DIST.value):
    	# Return true if all words in query.words edit match with any word in doc_words
        return all(any(editDistance(query_word, doc_word) <= query.match_dist for doc_word in doc_words) for query_word in query.words)
    else:
        raise Exception("Unknown MatchType")
#######################################################
#################### Main Functions ###################

def StartQuery(query_id: QueryID, query_str: str, match_type: MatchType, match_dist: int) -> ErrorCode:
    core.start_query(queries, ctypes.c_uint(int(query_id)), query_str.encode(), ctypes.c_uint(int(match_type.value)), ctypes.c_uint(match_dist))
    return ErrorCode.EC_SUCCESS

def EndQuery(query_id: QueryID) -> ErrorCode:
    core.end_query(queries, ctypes.c_uint(int(query_id)))
    return ErrorCode.EC_SUCCESS

def MatchDocument(doc_id: DocumentID, doc_str: str) -> ErrorCode:

    global queries

    #transfrom the document from char* to (void*)&std::set<std::string>
    doc_words: c_void_p = core.doc_str_to_doc_words(doc_str.encode())

    #returns a new set with c++ type (void*)&std::set<QueryID>
    ids: c_void_p = core.init_ids()

    #return a new cache with c++ type (void*)&Cache
    cache: c_void_p = core.init_cache(doc_words)

    #returns the number of unique query's in queries for the only reason to be used in the next line  
    size: int = int(core.queries_size(queries))

    #iterate over all unique query's
    for i in range(size):
        # check if the query matches with the current document.
        # core.match_query is the c++ version of the python function matchQuery
        # and needs:
        #  - queries, i to find the query
        #  - doc_words and cache for the matching
        # it returns a bool
        if(core.match_query(queries, i, doc_words, cache)):
            # add the queryID's related to the query at index i in 
            # queries to our already exisiting collection of id's
            core.add_ids(queries, i, ids)

    #converts C++ type std::set<QueryID> to C type unsigned int**
    ids_as_array = ctypes.POINTER(ctypes.c_uint)()
    n = core.ids_to_array(ids, ctypes.byref(ids_as_array))

    #converts C type unsigned int** to python list
    array = list(map(int, ids_as_array[:n]))

    doc = Document(doc_id, n, array)
    docs.append(doc)
    return ErrorCode.EC_SUCCESS

# Get the first undeliverd resuilt from "docs" and return it
def GetNextAvailRes() -> tuple[ErrorCode, DocumentID, int, tuple[int]]:
	if not docs:
		return ErrorCode.EC_NO_AVAIL_RES, 0, 0, None
	
	doc: Document = docs[0]
	p_doc_id: int = doc.doc_id
	p_num_res: int = doc.num_res
	p_query_ids: tuple[int] = doc.query_ids

	docs.pop(0)

	return ErrorCode.EC_SUCCESS, p_doc_id, p_num_res, p_query_ids