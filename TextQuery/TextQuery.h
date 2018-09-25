//
// Created by qiuyu on 18-9-21.
//

#ifndef TEXTQUERY_TEXTQUERY_H
#define TEXTQUERY_TEXTQUERY_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

class QueryResult;
class TextQuery {
public:
    using line_no = std::vector<std::string>::size_type;
    TextQuery(std::ifstream &);
    QueryResult query(const std::string &) const;
    void display_map();     // debugging aid: print the map

private:
    std::shared_ptr<std::vector<std::string>> file;
    std::map<std::string, std::shared_ptr<std::set<line_no>>> wm;
    static std::string cleanup_str(const std::string&);
};

class QueryResult {
    friend std::ostream& print(std::ostream &, const QueryResult &);
public:
    typedef std::vector<std::string>::size_type line_no;
    typedef std::set<line_no>::const_iterator line_it;
    QueryResult(std::string s, std::shared_ptr<std::set<line_no>> p,
                std::shared_ptr<std::vector<std::string>> f):
                sought(s), lines(p), file(f) { }
    line_it begin() const { return lines->cbegin(); }
    line_it end() const { return lines->cend(); }
    std::shared_ptr<std::vector<std::string>> get_file() { return file; }
private:
    std::string sought;
    std::shared_ptr<std::vector<std::string>> file;
    std::shared_ptr<std::set<line_no>> lines;
};

std::ostream& print(std::ostream &, const QueryResult &);

class Query_base {
    friend class Query;
protected:
    using line_no = TextQuery::line_no;
    virtual ~Query_base() {}
private:
    virtual QueryResult eval(const TextQuery &) const = 0;
    virtual std::string rep() const = 0;
};

class WordQuery: public Query_base {
    friend class Query;
    WordQuery(const std::string &s): word_query(s) { std::cout << "WordQuery constructor " << s << std::endl; }
    QueryResult eval(const TextQuery &tq) const
                            { return tq.query(word_query); }
    std::string rep() const { std::cout << "WordQuery rep(): " << word_query << std::endl; return word_query; }
    std::string word_query;
};

class Query {
    friend Query operator~(const Query &);
    friend Query operator|(const Query &, const Query &);
    friend Query operator&(const Query &, const Query &);
public:
    Query(const std::string &s): q(new WordQuery(s)) { std::cout << "Query constructor with string " << s <<  std::endl; }
    QueryResult eval(const TextQuery &tq) const
                        { return q->eval(tq); }
    std::string rep() const
                        { std::cout << "Query rep()" << std::endl; return q->rep(); }
private:
    Query(std::shared_ptr<Query_base> query): q(query) { std::cout << "Query constructor with shared_ptr" << std::endl; }
    std::shared_ptr<Query_base> q;
};

std::ostream &operator<<(std::ostream &os, const Query query);

class NotQuery: public Query_base {
    friend Query operator~(const Query &);
    NotQuery(const Query &q): query(q) { std::cout << "NotQuery constructor" << std::endl; }
    QueryResult eval(const TextQuery &) const;
    std::string rep() const { std::cout << "NotQuery rep()" << std::endl; return "~(" + query.rep() + ")"; }
    Query query;
};

Query operator~(const Query &operand);

class BinaryQuery: public Query_base {
protected:
    BinaryQuery(const Query &l, const Query &r, std::string s):
                    lhs(l), rhs(r), opSym(s) { std::cout << "BinaryQuery constructor" << std::endl; }
    // 抽象类：BinaryQuery不定义eval
    std::string rep() const { std::cout << "BinaryQuery rep(): " << opSym << std::endl; return "(" + lhs.rep() + " "
                                        + opSym + " "
                                        + rhs.rep() + ")"; }
    Query lhs, rhs;
    std::string opSym;
};

class AndQuery: public BinaryQuery {
    friend Query operator&(const Query &, const Query &);
    AndQuery(const Query &left, const Query &right):
                        BinaryQuery(left, right, "&") { std::cout << "AndQuery constructor" << std::endl; }
    QueryResult eval(const TextQuery&) const;
};

Query operator&(const Query &lhs, const Query &rhs);

class OrQuery: public BinaryQuery {
    friend Query operator|(const Query &, const Query &);
    OrQuery(const Query &left, const Query &right):
                        BinaryQuery(left, right, "|") { std::cout << "OrQuery constructor" << std::endl; }
    QueryResult eval(const TextQuery&) const;
};

Query operator|(const Query &lhs, const Query &rhs);

#endif //TEXTQUERY_TEXTQUERY_H
