#include <distance.h>

unsigned int table[MAX_WORD_LENGTH+1];
unsigned int editDistance(const string& a, const string& b){

	unsigned int n, m;
	unsigned int i, j;
	unsigned int l, c, t;

	n = a.size();
	m = b.size();
	for(i = 0; i <= n; i++){
		table[i]=i;
	}

	l = 0;
	for(i = 1; i <= m; i++){
		l = i;
		for(j = 1; j <= n; j++){
			t = table[j - 1];
			if(a[j - 1] == b[i - 1]){
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
unsigned int hammingDistance(const string& a, const string& b){
	unsigned int sum, i;
	if(a.size()!=b.size())return INT32_MAX;
	sum = 0; 
	for(i = 0; i < a.size(); i++){
		if(a[i]!=b[i]){
			sum++;
		}
	}
	return sum;
}