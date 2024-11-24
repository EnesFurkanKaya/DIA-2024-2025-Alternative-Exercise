#ifndef DISTANCE_H
#define DISTANCE_H

#include <core.h>
#include <string>

using namespace std;
/**
 * @brief computes the hamming distance between first word and the second word
 * 
 * @param first word
 * @param second word
 * @return hamming distance
 */
unsigned int hammingDistance(const string& first, const string& second);

unsigned int editDistance(const string& first, const string& second);

#endif