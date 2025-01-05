# Exercise 2 - Reimplementation with optimized algorithms

#######################################################
###################### Libraries ######################

from ctypes import POINTER, Structure, c_bool, c_char_p, c_uint, c_void_p, byref
import ctypes

from python.helper.helper import *

#######################################################
################# Global Variables ####################

#Keeps all currently active queries
queries: c_void_p
#Keeps the cache
cache: c_void_p

# Keeps all currently available results that has not been returned yet
docs: list[Document] = []

########################################################
################## Cwrapper Functions ##################

core = ctypes.CDLL("libcore.so")

core.init_ids.restype = c_void_p

core.queries_size.restype = c_uint
core.queries_size.argtypes = [c_void_p]

core.query_by_index.restype = c_void_p
core.query_by_index.argtypes = [c_void_p, c_uint]

core.add_ids.restype = None
core.add_ids.argtypes = [c_void_p, c_uint, c_void_p]

core.ids_to_array.restype = c_uint
core.ids_to_array.argtypes = [c_void_p, POINTER(POINTER(c_uint))]

core.start_query.restype = None
core.start_query.argtypes = [c_void_p, c_uint, c_char_p, c_uint, c_uint]

core.end_query.restype = None
core.end_query.argtypes = [c_void_p, c_uint]

core.initializeCache.restype = c_void_p
core.initializeCache.argtypes = None

core.freeCache.restype = None
core.freeCache.argtypes = [c_void_p]

core.doc_str_to_doc_words_unorderedset.restype = c_void_p
core.doc_str_to_doc_words_unorderedset.argtypes = [c_char_p]

core.match_query_caching.restype = c_bool
core.match_query_caching.argtypes = [c_void_p, c_uint, c_void_p, c_void_p]

core.init_queries.restype = c_void_p

#######################################################
################### Helper Functions ##################
def InitializeIndex():
    global queries
    global cache

    cache = core.initializeCache()
    queries = core.init_queries()
    return ErrorCode.EC_SUCCESS

def DestroyIndex():
    global cache
    core.freeCache(cache)
    return ErrorCode.EC_SUCCESS

#######################################################
#################### Main Functions ###################

def StartQuery(query_id: QueryID, query_str: str, match_type: MatchType, match_dist: int) -> ErrorCode:
    core.start_query(queries, ctypes.c_uint(int(query_id)), query_str.encode(), ctypes.c_uint(int(match_type.value)), ctypes.c_uint(match_dist))
    return ErrorCode.EC_SUCCESS

def EndQuery(query_id: QueryID) -> ErrorCode:

    core.end_query(queries, ctypes.c_uint(int(query_id)))
    return ErrorCode.EC_SUCCESS

def cache_doc_words(doc_str: str):
    if not hasattr(cache_doc_words, "_doc_cache"):
        cache_doc_words._doc_cache = {}

    if doc_str not in cache_doc_words._doc_cache:
        cache_doc_words._doc_cache[doc_str] = core.doc_str_to_doc_words_unorderedset(doc_str.encode('utf-8'))

    return cache_doc_words._doc_cache[doc_str]

def MatchDocument(doc_id: DocumentID, doc_str: str) -> ErrorCode:
    
    global queries
    global cache
    
    # Cache the document words
    doc_words = cache_doc_words(doc_str)

    # Returns a new set with c++ type (void*)&std::set<QueryID>
    ids: c_void_p = core.init_ids()

    # Returns the number of unique queries in queries
    size: int = int(core.queries_size(queries))
    
    # Iterate over all unique queries
    for i in range(size):
        if core.match_query_caching(queries, i, doc_words, cache):
            # Adds matched query ids to variable ids
            core.add_ids(queries, i, ids)

    # Converts C++ type std::set<QueryID> to C type unsigned int**
    ids_as_array = ctypes.POINTER(ctypes.c_uint)()
    n = core.ids_to_array(ids, ctypes.byref(ids_as_array))

    # Converts C type unsigned int** to Python list
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
    