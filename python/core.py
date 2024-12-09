# Exercise 1 - Reimplementation for exercise 3

#######################################################
######################## TODO #########################

# 1. Test with test driver for correctness - Status: Not Tested

#######################################################
###################### Libraries ######################

from helper.helper import *

#######################################################
################# Global Variables ###################

#Keeps all currently active queries
queries = Queries()
# Keeps all currently available results that has not been returned yet
docs: list[Document] = []

#######################################################
################### Helper Functions ##################
def InitializeIndex():
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
	queries.add(
        id = query_id,
        query = Query(match_type, match_dist, tuple(query_str.split())))
	return ErrorCode.EC_SUCCESS

def EndQuery(query_id: QueryID) -> ErrorCode:
	queries.remove(query_id)
	return ErrorCode.EC_SUCCESS
	
def MatchDocument(doc_id: DocumentID, doc_str: str) -> ErrorCode:
	doc_words: list[str] = [str(x) for x in doc_str.split(' ') if x.strip()]
	# Matching
	ids: set[QueryID] = set()
	for query in queries.getAllQuerys():
		if matchQuery(query, doc_words):
			set_ids = queries.getIDs(query)
			ids.update(set_ids)

	# Creating document object
	doc = Document(doc_id, len(ids), sorted(ids))
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