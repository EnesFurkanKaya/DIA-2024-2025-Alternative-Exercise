from dataclasses import dataclass
import datetime
from enum import Enum
import time
from typing import TextIO
from array import array
from collections import deque

from python.core import DestroyIndex, EndQuery, GetNextAvailRes, InitializeIndex, MatchDocument, StartQuery
from python.helper.helper import ErrorCode, MatchType

INPUT_FILE_PATH = "../test_data/small_test.txt"
OUTPUT_FILE_PATH = "../results.txt"

class LineType(Enum):
    START_QUERY = "s"
    END_QUERY = "e"
    MATCH_DOCUMENT = "m"
    UPDATE_EXPECTED_RESULTS = "r"

@dataclass(frozen=True)
class StartQueryLine:
    query_id: int
    match_dist: int
    match_type: MatchType
    query_words: int
    query_str: str

@dataclass(frozen=True)
class EndQueryLine:
    query_id: int

@dataclass(frozen=True)
class MatchDocumentLine:
    doc_id: int
    doc_words: int
    doc_str: str

@dataclass(frozen=True)
class UpdateResultsLine:
    first_res: int
    num_res: int
    results: list[int]
    
def process_line(line: str) -> (StartQueryLine | EndQueryLine | MatchDocumentLine | UpdateResultsLine):
    parts: list[str] = line.split()
    line_type: LineType = LineType(parts[0])
    
    match line_type:
        case LineType.START_QUERY:
            result = StartQueryLine(
                query_id = int(parts[1]), 
                match_type = MatchType(int(parts[2])),
                match_dist = int(parts[3]),
                query_words = int(parts[4]), 
                query_str = " ".join(parts[5:])
            )

        case LineType.END_QUERY:
            result  = EndQueryLine(query_id=int(parts[1]))

        case LineType.MATCH_DOCUMENT:
            result = MatchDocumentLine(
                doc_id = int(parts[1]),
                doc_words = int(parts[2]),
                doc_str = " ".join(parts[3:])
            )

        case LineType.UPDATE_EXPECTED_RESULTS:
            result = UpdateResultsLine(
                first_res = int(parts[1]),
                num_res = int(parts[2]),
                results = [int(x) for x in parts[3:]]
            )

    return result

def check_cur_results(cur_results: deque[list[int]], next_doc_id: int):
    while(len(cur_results) > 0):
        error, doc_id, num_res, query_ids =  GetNextAvailRes()
        
        #check ErrorCode
        if(error.value is not ErrorCode.EC_SUCCESS.value):
            output_file.write(str(error))
            return error

        #check doc id
        if(doc_id != next_doc_id):
            output_file.write("The call to GetNextAvailRes() returned unknown document ID .\n")
            return ErrorCode.EC_FAIL, next_doc_id
        cur_result: list[int] = cur_results.popleft()
        
        #check queryID's
        if(query_ids!=cur_result or num_res != len(cur_result)):
            output_file.write("The call to GetNextAvailRes() returned wrong number of query IDs: " + str(num_res)+ "vs" +str(len(cur_result)) +"\n")
            output_file.write("expected: "+str(cur_result)+"\n")
            output_file.write("your solution: "+str(query_ids)+"\n")
            return ErrorCode.EC_FAIL, next_doc_id
        next_doc_id += 1

    return ErrorCode.EC_SUCCESS, next_doc_id
    
def test_sigmoid(input_file: TextIO, output_file: TextIO, time_limit_seconds: int) -> None:

    output_file.write("Start Test ...\n")
    start_time: float = time.time()
    end_time: float = start_time + time_limit_seconds

    InitializeIndex()

    cur_results: deque[list[int]] = deque()
    next_doc_id: int = 0
    error: ErrorCode
    for line in input_file:
        typed_line: (StartQueryLine | EndQueryLine | MatchDocumentLine | UpdateResultsLine)
        typed_line = process_line(line)

        if type(typed_line) == StartQueryLine:
            error, next_doc_id = check_cur_results(cur_results, next_doc_id)
            error = StartQuery(typed_line.query_id, typed_line.query_str, typed_line.match_type, typed_line.match_dist)
            if(error.value != ErrorCode.EC_SUCCESS.value):
                output_file.write("start_query returned "+ str(error))
                return

        if type(typed_line) == EndQueryLine:
            error, next_doc_id = check_cur_results(cur_results, next_doc_id)
            error = EndQuery(typed_line.query_id)
            if(error.value != ErrorCode.EC_SUCCESS.value):
                output_file.write("end_query returned "+ str(error))
                return

        if type(typed_line) == MatchDocumentLine:
            error = MatchDocument(typed_line.doc_id, typed_line.doc_str)
            if(error.value != ErrorCode.EC_SUCCESS.value):
                output_file.write("match_document returned "+ str(error))
                return

        if type(typed_line) == UpdateResultsLine:
            if(len(cur_results)==0):
                next_doc_id = typed_line.first_res
            cur_results.append(typed_line.results)            

        if time.time() > end_time:
            output_file.write("time limit of "+str(time_limit_seconds)+" sec exceeded\n")
            break

    duration: float = time.time() - start_time
    output_file.write("Duration: " + str(datetime.timedelta(seconds= duration))+ "\n")
    output_file.write("Throughput: "+ str(next_doc_id/duration) + " documents per second\n")
    DestroyIndex()
    output_file.write("Test finished\n")

input_file: TextIO = open(INPUT_FILE_PATH, 'r')
output_file: TextIO = open(OUTPUT_FILE_PATH, 'w')
test_sigmoid(input_file, output_file, 1000)
input_file.close()
output_file.close()