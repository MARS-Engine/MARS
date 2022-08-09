#include "String.hpp"

void Split(const string &in, vector<string> &out, string token) {

    out.clear();
    string temp;

    for (int i = 0; i < int(in.size()); i++) {

        if(in.substr(i, token.size()) == token) {

            if (!temp.empty()) {

                out.push_back(temp);
                temp.clear();
                i += (int)token.size() - 1;
            }
            else
                out.push_back("");
        }
        else if (i + token.size() >= in.size()) {
            temp += in.substr(i, token.size());
            out.push_back(temp);
            break;
        }
        else
            temp += in[i];
    }
}

string Tail(const string &in) {

	size_t tailStart = in.find_first_not_of(" \t", in.find_first_of(" \t", in.find_first_not_of(" \t")));
	size_t tailEnd = in.find_last_not_of(" \t");

	if (tailStart != string::npos && tailEnd != string::npos)
		return in.substr(tailStart, tailEnd - tailStart + 1);
	else if (tailStart != string::npos)
		return in.substr(tailStart);

	return "";
}

string FirstToken(const string &in) {

	if (!in.empty()) {
		size_t token_start = in.find_first_not_of(" \t");
		size_t token_end = in.find_first_of(" \t", token_start);
		if (token_start != string::npos && token_end != string::npos)
			return in.substr(token_start, token_end - token_start);
		else if (token_start != string::npos)
			return in.substr(token_start);
	}

	return "";
}