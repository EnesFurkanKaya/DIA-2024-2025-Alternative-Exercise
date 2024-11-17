/// Error codes:			
typedef enum{
    /**
    * Must be returned by each core function unless specified otherwise.
    */
    EC_SUCCESS,
    /**
    * Must be returned only if there is no available result to be returned
    * by GetNextAvailRes(). That is, all results have already been returned
    * via previous calls to GetNextAvailRes().
    */
    EC_NO_AVAIL_RES,
    /**
    * Used only for debugging purposes, and must not be returned in the
    * final submission.
    */
    EC_FAIL,
    /**
    * Used only for debugging purposes of memory control, and must not be returned in the
    * final submission.
    */
    EC_MEMORY_FAIL

}
ErrorCode;

/// Matching types:
typedef enum{
    /**
    * Two words match if they are exactly the same.
    */
    MT_EXACT_MATCH,
    /**
    * Two words match if they have the same number of characters, and the
    * number of mismatching characters in the same position is not more than
    * a specific threshold.
    */
    MT_HAMMING_DIST,
    /**
    * Two words match if one of them can can be transformed into the other word
    * by inserting, deleting, and/or replacing a number of characters. The number
    * of such operations must not exceed a specific threshold.
    */
    MT_EDIT_DIST
}
MatchType;

/// Maximum document length in characters.
#define MAX_DOC_LENGTH (1<<22)

/// Maximum word length in characters.
#define MAX_WORD_LENGTH 31

/// Minimum word length in characters.
#define MIN_WORD_LENGTH 4

/// Maximum number of words in a query.
#define MAX_QUERY_WORDS 5

/// Maximum query length in characters.
#define MAX_QUERY_LENGTH ((MAX_WORD_LENGTH+1)*MAX_QUERY_WORDS)

typedef unsigned int QueryID;
typedef unsigned int DocID;

