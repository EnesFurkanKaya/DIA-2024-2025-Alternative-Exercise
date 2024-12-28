# Exercise 2 - Reimplementation for exercise 3

#######################################################
######################## TODO #########################

# 1. Add optimisation algorithms:
# 	1.1 Caching - Status: Not Added
# 	1.2 Multithreading - Status: Not Added

#######################################################
###################### Libraries ######################

from helper.helper import *
from concurrent.futures import ThreadPoolExecutor

#######################################################
################# Global Variables ###################

#Keeps all currently active queries
queries = Queries()
# Keeps all currently available results that has not been returned yet
docs = []

# Initialize a thread pool with the number of available cores
executor = ThreadPoolExecutor()

#######################################################
################### Helper Functions ##################
def InitializeIndex():
	return ErrorCode.EC_SUCCESS

def DestroyIndex():
	return ErrorCode.EC_SUCCESS

def editDistance(a: str, b: str):
	table = []

	for i in range(1, a.size()+1):
		table[i]=i

	left = 0
	for i in range(1, b.size()+1):
		left = i
		for j in range(1, a.size()+1):
			if(a[j - 1] == b[i - 1]):
				cur = table[j - 1]
			else:
				cur = min(min(table[j-1] + 1, table[j] + 1), left + 1)
			table[j-1] = left
			left = cur
		table[a.size()]=left
		
	return left

def hammingDistance(a: str, b: str):
	if(a.size()!=b.size()): return float('inf')
	sum = 0
	for i in range(a.size()):
		if(a[i]!=b[i]):
			sum+=1
			
	return sum

def matchQuery(query: Query, doc_words: list):
	match query.match_type:
		case MatchType.MT_EXACT_MATCH:
			# Return true if all words in doc_words exact match with a word in query.words
			return all(word in doc_words.words for word in query.words)
		case MatchType.MT_HAMMING_DIST:
			# Return true if all words in query.words hamming match with a word in doc_words
			return all(
				any(
					hammingDistance(query_word, doc_word) <= query.match_dist
					for doc_word in doc_words
				)
				for query_word in query.words
			)
		case MatchType.MT_EDIT_DIST:
			# Return true if all words in query.words edit match with a word in doc_words
			return all(
				any(
					editDistance(query_word, doc_word) <= query.match_dist
					for doc_word in doc_words
				)
				for query_word in query.words
			)
	return False

#######################################################
#################### Main Functions ###################

def StartQuery(query_id: QueryID, query_str: str, match_type: MatchType, match_dist: int):

	query = Query(match_type, match_dist, query_str)
	queries.add(query_id, query)

	return ErrorCode.EC_SUCCESS

def EndQuery(query_id: QueryID):
	queries.remove(query_id)
	return ErrorCode.EC_SUCCESS
	
def MatchDocument(doc_id: DocumentID, doc_str: str) -> ErrorCode:
    doc_words = [str(x) for x in doc_str.split(' ') if x.strip()]

    # Matching
    ids = set()

    def process_query(query):
        if matchQuery(query, doc_words):
            set_ids = queries.getIDs(query)
            ids.update(set_ids)

    # Use the thread pool to process queries in parallel
    futures = [executor.submit(process_query, query) for query in queries.getAllQuerys()]
    for future in futures:
        future.result()

    # Creating document object
    doc = Document(doc_id, len(ids), ids)
    docs.append(doc)

    return ErrorCode.EC_SUCCESS

# Get the first undeliverd resuilt from "docs" and return it
def GetNextAvailRes():
	p_doc_id = 0
	p_num_res = 0
	p_query_ids = None

	if not docs:
		return ErrorCode.EC_NO_AVAIL_RES
	
	doc = docs[0]

	p_doc_id = doc.doc_id
	p_num_res = doc.num_res
	p_query_ids = doc.query_ids

	docs.pop(0)

	return ErrorCode.EC_SUCCESS, p_doc_id, p_num_res, p_query_ids