# Exercise 1 - Reimplementation for exercise 3

#######################################################
######################## TODO #########################

# 1. Test with test driver for correctness - Status: Not Tested

#######################################################
###################### Libraries ######################

from ctypes import POINTER, Structure, c_bool, c_char_p, c_uint, c_void_p, byref, c_size_t
import ctypes

from helper.helper import *
from pyspark.sql import SparkSession
from pyspark import SparkConf, SparkContext
from dask import delayed, compute
from dask.distributed import Client

#######################################################
################# Global Variables ###################

#Keeps all currently active queries
queries: c_void_p

query_dict = {}
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
core.match_query.argtypes = [c_void_p, c_uint, c_void_p, c_char_p]

core.add_ids.restype = None
core.add_ids.argtypes = [c_void_p, c_uint, c_void_p]

core.ids_to_array.restype = c_uint
core.ids_to_array.argtypes = [c_void_p, POINTER(POINTER(c_uint))]

core.start_query.restype = None
core.start_query.argtypes = [c_void_p, c_uint, c_char_p, c_uint, c_uint]

core.end_query.restype = None
core.end_query.argtypes = [c_void_p, c_uint]

core.match_query_apache.restype = c_bool
core.match_query_apache.argtypes = [c_void_p, c_size_t, c_uint, c_uint, c_void_p, c_size_t]

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

#######################################################
#################### Main Functions ###################

def StartQuery(query_id: QueryID, query_str: str, match_type: MatchType, match_dist: int) -> ErrorCode:
    global query_dict

    query_dict[query_id] = (query_str.split(), match_type.value, match_dist)
    
    core.start_query(queries, ctypes.c_uint(int(query_id)), query_str.encode(), ctypes.c_uint(int(match_type.value)), ctypes.c_uint(match_dist))
    return ErrorCode.EC_SUCCESS

def EndQuery(query_id: QueryID) -> ErrorCode:
    global query_dict

    del query_dict[query_id]

    core.end_query(queries, ctypes.c_uint(int(query_id)))
    return ErrorCode.EC_SUCCESS

    """_summary_
       we wrapped c++ code in c functions, wrapped every used c++ class with void pointers
       and have now this trash function that noone understands BUT we are 0.8 as fast as the c++ code
       and need less then 1 second instead of 5 minutes.
    """

def does_query_match(query_words, match_type, match_dist, doc_words):
    """
    Check if all words in the query match the document using match_single_word.
    """
    c_strings = (ctypes.c_char_p * len(doc_words))(*[s.encode('utf-8') for s in doc_words])
    b_strings = (ctypes.c_char_p * len(query_words))(*[s.encode('utf-8') for s in query_words])

    return bool(core.match_query_apache(b_strings, len(b_strings), match_type, match_dist, c_strings, len(c_strings)))
    

# Dask - with local query_dict - very slow 
def MatchDocument_dask(doc_id: DocumentID, doc_str: str) -> ErrorCode:
    
    global queries
    global query_dict

    document_words = [word for word in doc_str.split()]

    def filter_dict(query, doc_words):
        if does_query_match(query_dict[query][0], query_dict[query][1], query_dict[query][2], doc_words):
            return query
        return None

    filtered_data = [filter_dict(query, document_words) for query in query_dict]
    filtered_results = compute(*filtered_data)
    filtered_results = [res for res in filtered_results if res]

    array = list(map(int, filtered_results))
        
    doc = Document(doc_id, len(array), array)
    docs.append(doc)

    return ErrorCode.EC_SUCCESS

#Not working - can not pickle c_type variables into rdd.filter() ???
def MatchDocument(sc: SparkContext, doc_id: DocumentID, doc_str: str) -> ErrorCode:
    
    global queries
    
    #transfrom the document from char* to (void*)&std::set<std::string>
    doc_words: c_void_p = core.doc_str_to_doc_words(doc_str.encode())

    #returns a new set with c++ type (void*)&std::set<QueryID>
    ids: c_void_p = core.init_ids()

    #returns the number of unique query's in queries for the only reason to be used in the next line  
    size: int = int(core.queries_size(queries))

    indices_rdd = sc.parallelize(range(size))

    correct_ids_rdd = indices_rdd.filter(lambda i: core.match_query_og(queries, i, doc_words))

    correct_ids = correct_ids_rdd.collect()
    for i in correct_ids:
        core.add_ids(queries, i, ids)

    #converts C++ type std::set<QueryID> to C type unsigned int**
    ids_as_array = ctypes.POINTER(ctypes.c_uint)()
    n = core.ids_to_array(ids, ctypes.byref(ids_as_array))

    #converts C type unsigned int** to python list
    array = list(map(int, ids_as_array[:n]))

    doc = Document(doc_id, n, array)
    docs.append(doc)
    return ErrorCode.EC_SUCCESS

# Apache Spark - with local query_dict - very slow 
def MatchDocument_apache(sc: SparkContext, doc_id: DocumentID, doc_str: str) -> ErrorCode:
    
    global queries
    global query_dict

    document_words = [word for word in doc_str.split()]
    doc_words_broadcast = sc.broadcast(document_words)

    python_queries_rdd = sc.parallelize(list(query_dict.items()), 100) # x[0] = query_id

    matched_qwords_ids = python_queries_rdd.filter(
        lambda query: does_query_match(query[1][0], query[1][1], query[1][2], doc_words_broadcast.value)
    ).map(lambda query: query[0]).collect()

    #converts C type unsigned int** to python list
    array = list(map(int, matched_qwords_ids))
        
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
    