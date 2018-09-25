#include <iostream>
#include <stack>
#include "TextQuery.h"

using namespace std;

void runQueries(ifstream &ifs) {
    TextQuery tq(ifs);
    while (true)
    {
        cout << "enter word to look for, or q to quit: ";
        string s;
        if (!(cin >> s) || s[0] == 'q')
            break;
        print(cout, tq.query(s)) << endl;
    }
}

constexpr size_t N_OPTR = 4;
const char prio[N_OPTR][N_OPTR] = {
                  /* '~', '&', '|', '\0' */
        /* '~' */ { '>', '>', '>', '>' },
        /* '&' */ { '<', '>', '>', '>' },
        /* '|' */ { '<', '>', '>', '>' },
        /* '\0' */{ '<', '<', '<', '=' }
};

int order(const char ch) {
    int index = 0;
    switch (ch)
    {
        case '~': index = 0; break;
        case '&': index = 1; break;
        case '|': index = 2; break;
        case '\0': index = 3; break;
    }
    return index;
}

vector<string> getRPN(const char* exp) {
    vector<string> RPN;
    stack<char> optr;
    optr.push('\0');
    const char* it = exp;
    while (!optr.empty())
    {
        if (isalpha(*it))
        {
            string s(1, *it);
            while (isalpha(*(++it)))
            {
                s += *it;
            }
            RPN.push_back(s);
        }
        else if (!isspace(*it))
        {
            switch (prio[order(optr.top())][order(*it)])
            {
                case '<':
                    optr.push(*it++);
                    break;
                case '=':
                    optr.pop();
                    ++it;
                    break;
                case '>':
                    char ch = optr.top();
                    optr.pop();
                    RPN.push_back(string(1, ch));
                    break;
            }
        } else
            ++it;
    }
    return RPN;
}

Query evalRPN(vector<string> &RPN) {
    vector<Query> opnd;
    for (auto s : RPN)
    {
        if (s == "~")
        {
            Query q = opnd.back();
            opnd.pop_back();
            opnd.push_back(~q);
        } else if (s == "&")
        {
            Query rhs = opnd.back();
            opnd.pop_back();
            Query lhs = opnd.back();
            opnd.pop_back();
            opnd.push_back(lhs & rhs);
        } else if (s == "|")
        {
            Query rhs = opnd.back();
            opnd.pop_back();
            Query lhs = opnd.back();
            opnd.pop_back();
            opnd.push_back(lhs | rhs);
        } else {
            opnd.push_back(Query(s));
        }
    }
    return opnd.back();
}

TextQuery get_file(int argc, char **argv) {
    ifstream infile;
    if (argc == 2)
        infile.open(argv[1]);
    if (!infile)
        throw runtime_error("No input file!");
    return TextQuery(infile);
}


int main(int argc, char **argv) {
    TextQuery tq = get_file(argc, argv);
    string line;
    while (true)
    {
        cout << "enter query string to look for, or q to quit: ";
        if (getline(cin, line) && line != "q")
        {
            vector<string> rpn = getRPN(line.c_str());
            for (auto token : rpn)
                cout << token << " ";
            cout << endl;
            Query q = evalRPN(rpn);
            QueryResult res = q.eval(tq);
            print(cout, res);
        } else
            break;
    }

    return 0;
}