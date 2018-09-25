//
// Created by qiuyu on 18-9-21.
//

#include <iostream>
using std::cout; using std::ostream; using std::endl;

#include <fstream>
using std::ifstream;

#include <sstream>
using std::istringstream;

#include <string>
using std::string; using std::getline;

#include <vector>
using std::vector;

#include <set>
using std::set;

#include <memory>
using std::make_shared; using std::shared_ptr;

#include <algorithm>
using std::set_intersection;

#include <iterator>
using std::inserter;

#include "TextQuery.h"

// 读取输入文件并建立单词到行号的映射
TextQuery::TextQuery(std::ifstream &ifs): file(new vector<string>)
{
    string text;
    while (getline(ifs, text))
    {
        file->push_back(text);
        int n = file->size() - 1;
        istringstream line(text);
        string word;
        while (line >> word)
        {
            word = cleanup_str(word);
            auto &lines = wm[word];
            if (!lines)
                lines.reset(new set<line_no>);
            lines->insert(n);
        }
    }
}

// 去除符号字符并将所有字符转换为小写，这样query操作就是大小写不敏感的
std::string TextQuery::cleanup_str(const std::string &s) {
    string res;
    for (auto c : s)
    {
        if (!ispunct(c))
            res += tolower(c);
    }
    return res;
}

QueryResult TextQuery::query(const std::string &sought) const {
    // 如果未找到sought，我们将返回一个指向此set的指针
    static shared_ptr<set<line_no>> nodata(new set<line_no>);
    auto loc = wm.find(cleanup_str(sought));
    if (loc != wm.end())
        return QueryResult(sought, loc->second, file);
    else
        return QueryResult(sought, nodata, file);
}

void TextQuery::display_map() {
    for (auto it = wm.begin(); it != wm.end(); ++it)
    {
        cout << "word: " << it->first << " {";

        auto text_locs = it->second;
        auto loc_it = text_locs->begin();
        auto loc_it_end = text_locs->end();
        while (loc_it != loc_it_end)
        {
            cout << *loc_it;
            if (++loc_it != loc_it_end)
                cout << ", ";
        }
        cout << "}\n";
    }
    cout << endl;
}

ostream &print(ostream &os, const QueryResult &query) {
    os << query.sought << " occurs " << query.lines->size() << " "
        << (query.lines->size() > 1 ? "time": "times") << endl;
    for (auto num : *query.lines)
        os << "\t(line " << num + 1 << ") "
            << (*query.file)[num] << endl;
    return os;
}

QueryResult NotQuery::eval(const TextQuery & text) const {
    auto result = query.eval(text);
    auto ret_lines = make_shared<set<line_no>>();
    auto beg = result.begin(), end = result.end();
    // 对于输入文件中的每一行，如果该行不在result当中，则将其添加到ret_lines
    auto sz = result.get_file()->size();
    for (size_t i = 0; i < sz; ++i)
    {
        if (beg == end || *beg != i)
            ret_lines->insert(i);
        else if (beg != end)
            ++beg;
    }
    return QueryResult(rep(), ret_lines, result.get_file());
}

QueryResult AndQuery::eval(const TextQuery &text) const {
    auto lhs_result = lhs.eval(text);
    auto rhs_result = rhs.eval(text);
    auto ret_lines = make_shared<set<line_no>>();
    set_intersection(lhs_result.begin(), lhs_result.end(),
                    rhs_result.begin(), rhs_result.end(),
                    inserter(*ret_lines, ret_lines->begin()));
    return QueryResult(rep(), ret_lines, lhs_result.get_file());
}

QueryResult OrQuery::eval(const TextQuery &text) const {
    auto lhs_result = lhs.eval(text);
    auto rhs_resut = rhs.eval(text);
    auto ret_lines = make_shared<set<line_no>>(lhs_result.begin(), lhs_result.end());
    ret_lines->insert(rhs_resut.begin(), rhs_resut.end());
    return QueryResult(rep(), ret_lines, lhs_result.get_file());
}

std::ostream &operator<<(std::ostream &os, const Query query) {
    return os << query.rep();
}

Query operator~(const Query &operand) {
    return std::shared_ptr<Query_base>(new NotQuery(operand));
}

Query operator&(const Query &lhs, const Query &rhs) {
    return std::shared_ptr<Query_base>(new AndQuery(lhs, rhs));
}

Query operator|(const Query &lhs, const Query &rhs) {
    return std::shared_ptr<Query_base>(new OrQuery(lhs, rhs));
}



