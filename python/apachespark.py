# Exercise 3 - Reimplementation of our functions on top of a distributed data processing framework
# We used Apache Spark API for Python (Pyspark)


#######################################################
######################## TODO #########################

# 1. Implement the functions:
# 	1.1: def StartQuery() - Status: Not Implemented
# 	1.2: def EndQuery() - Status: Not Implemented
# 	1.3: def MatchDocument() - Status: Not Implemented
# 	1.4: def GetNextAvailRest() - Status: Not Implemented

#######################################################
###################### Libraries ######################

from helper.helper import *
from pyspark.sql import SparkSession

#######################################################
################# Global Variables ###################

spark = SparkSession.builder.appName("QueryDocumentMatching").getOrCreate()

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

#######################################################
#################### Main Functions ###################

def StartQuery(query_id: QueryID, query_str: str, match_type: MatchType, match_dist: int):
	return ErrorCode.EC_SUCCESS

def EndQuery(query_id: QueryID):
	return ErrorCode.EC_SUCCESS
	
def MatchDocument(doc_id: DocumentID, doc_str: str):
	return ErrorCode.EC_SUCCESS

# Get the first undeliverd resuilt from "docs" and return it
def GetNextAvailRes():
	return ErrorCode.EC_SUCCESS