# Exercise 3 - Reimplementation using Dask

#######################################################
###################### Libraries ######################

from ctypes import POINTER, Structure, c_bool, c_char_p, c_uint, c_void_p, byref, c_size_t
import ctypes

from python.helper.helper import *

from dask import compute
from pyspark import SparkContext
sc = SparkContext.getOrCreate()

#######################################################
################# Global Variables ####################

#Keeps all currently active queries
queries: c_void_p

# Keeps all currently available results that has not been returned yet
docs: list[Document] = []

########################################################
################## Cwrapper Functions ##################

core = ctypes.CDLL("libcore.so")

core.doc_str_to_doc_words.restype = c_void_p
core.doc_str_to_doc_words.argtypes = [c_char_p]

core.init_ids.restype = c_void_p

core.queries_size.restype = c_uint
core.queries_size.argtypes = [c_void_p]

core.add_ids.restype = None
core.add_ids.argtypes = [c_void_p, c_uint, c_void_p]

core.ids_to_array.restype = c_uint
core.ids_to_array.argtypes = [c_void_p, POINTER(POINTER(c_uint))]

core.start_query.restype = None
core.start_query.argtypes = [c_void_p, c_uint, c_char_p, c_uint, c_uint]

core.end_query.restype = None
core.end_query.argtypes = [c_void_p, c_uint]

core.match_query.restype = c_bool
core.match_query.argtypes = [c_void_p, c_uint, c_void_p]

core.init_queries.restype = c_void_p

#######################################################
################### Helper Functions ##################
def InitializeIndex():
    global queries
    queries = core.init_queries()
    return ErrorCode.EC_SUCCESS

def DestroyIndex():
    return ErrorCode.EC_SUCCESS


#######################################################
#################### Main Functions ###################

def StartQuery(query_id: QueryID, query_str: str, match_type: MatchType, match_dist: int) -> ErrorCode:
    
    core.start_query(queries, ctypes.c_uint(int(query_id)), query_str.encode(), ctypes.c_uint(int(match_type.value)), ctypes.c_uint(match_dist))
    return ErrorCode.EC_SUCCESS

def EndQuery(query_id: QueryID) -> ErrorCode:

    core.end_query(queries, ctypes.c_uint(int(query_id)))
    return ErrorCode.EC_SUCCESS

# Apache Spark - Could not serialize object: ValueError: ctypes objects containing pointers cannot be pickled
def MatchDocument_apache(doc_id: DocumentID, doc_str: str) -> ErrorCode:
    
    global queries

    #transfrom the document from char* to (void*)&std::set<std::string>
    doc_words: c_void_p = core.doc_str_to_doc_words(doc_str.encode())

    #returns a new set with c++ type (void*)&std::set<QueryID>
    ids: c_void_p = core.init_ids()

    #returns the number of unique query's in queries for the only reason to be used in the next line  
    size: int = int(core.queries_size(queries))
    
    indices_rdd = sc.parallelize(range(size))
    correct_ids_rdd = indices_rdd.filter(lambda i: core.match_query(queries, i, doc_words))

    correct_ids = correct_ids_rdd.collect()

    for i in correct_ids:
        core.add_ids(queries, i, ids)

    # After matching, convert ids to array and map to Python list
    ids_as_array = ctypes.POINTER(ctypes.c_uint)()
    n = core.ids_to_array(ids, ctypes.byref(ids_as_array))

    array = list(map(int, ids_as_array[:n]))
        
    doc = Document(doc_id, len(array), array)
    docs.append(doc)

    return ErrorCode.EC_SUCCESS

# Dask
def cache_doc_words(doc_str: str):
    if not hasattr(cache_doc_words, "_doc_cache"):
        cache_doc_words._doc_cache = {}

    if doc_str not in cache_doc_words._doc_cache:
        cache_doc_words._doc_cache[doc_str] = core.doc_str_to_doc_words(doc_str.encode('utf-8'))

    return cache_doc_words._doc_cache[doc_str]

def match_query_on_doc(queries, doc_words, query_idx: int):
    # Perform the match for a specific query
    return query_idx if core.match_query(queries, query_idx, doc_words) else None

def add_ids_to_core(queries, ids, local_ids):
    unique_ids = set()
    for query_idx in local_ids:
        if query_idx is not None:
            unique_ids.add(query_idx)

    for i in unique_ids:
        core.add_ids(queries, i, ids)

def convert_ids_to_list(ids):
    ids_as_array = ctypes.POINTER(ctypes.c_uint)()
    n = core.ids_to_array(ids, ctypes.byref(ids_as_array))
    return list(map(int, ids_as_array[:n]))

def MatchDocument(doc_id: DocumentID, doc_str: str) -> ErrorCode:
    global queries

    doc_words = cache_doc_words(doc_str)

    # Initialize IDs container
    ids = core.init_ids()

    # Localizing core functions to avoid lookups because it is called more than once
    size = core.queries_size(queries)
    
    # Use Dask to create tasks for query matching
    tasks = [match_query_on_doc(queries, doc_words, query_idx) for query_idx in range(size)]
    local_ids = compute(*tasks)

    add_ids_to_core(queries, ids, local_ids)

    array = convert_ids_to_list(ids)

    # Create Document object
    doc = Document(doc_id, len(array), array)
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
    