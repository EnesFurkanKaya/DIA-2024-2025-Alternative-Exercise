#include <distance.h>

unsigned int table[MAX_WORD_LENGTH+1];
unsigned int editDistance(const string& first_word, const string& second_word){

	unsigned int n, m;
	unsigned int i, j;
	unsigned int l, c, t;

	n = first_word.size();
	m = second_word.size();
	for(i = 0; i <= n; i++){
		table[i]=i;
	}

	l = 0;
	for(i = 1; i <= m; i++){
		l = i;
		for(j = 1; j <= n; j++){
			t = table[j - 1];
			if(first_word[j - 1] == second_word[i - 1]){
				c = t;
			} else {
				c = min(min(t, table[j]), l) + 1;
			}
			table[j-1] = l;
			l = c;
		}
		table[n]=l;
	}
	return l;
}

///////////////////////////////////////////////////////////////////////////////////////////////
unsigned int hammingDistance(const string& first_word, const string& second_word){
	unsigned int sum, i;
	if(first_word.size()!=second_word.size())return INT32_MAX;
	sum = 0; 
	for(i = 0; i < first_word.size(); i++){
		if(first_word[i]!=second_word[i]){
			sum++;
		}
	}
	return sum;
}