#ifndef _MATH_STRING_
#define _MATH_STRING_

#include <string>
#include <vector>

using namespace std;

void Split(const string &in, vector<string> &out, string token);
string Tail(const string &in);
string FirstToken(const string &in);

template <class T> const T &getElement(const vector<T> &elements, string &index) {

	int idx = stoi(index);
	if (idx < 0)
		idx = int(elements.size()) + idx;
	else
		idx--;
	return elements[idx];
}

#endif