#include <algorithm>
#include <iostream>

#include "StringSet.hh"

using namespace std;


StringSet::StringSet(const std::map<std::string, flt_type> &vocab)
{
    max_factor_length = 0;
    for (auto it = vocab.cbegin(); it !=vocab.cend(); ++it)
        add(it->first, it->second);
}


StringSet::~StringSet() {
    clear(&root_node);
}


StringSet::Node*
StringSet::insert(char letter,
                  const string &factor,
                  flt_type cost,
                  StringSet::Node *node)
{
    // Find a possible existing arc with the letter
    Arc *arc = find_arc(letter, node);

    // No existing arc: create a new arc
    if (arc == nullptr) {
        Node *new_node = new Node();
        unsigned int remapped = remap_char(letter);
        if (node->arcs.size() < remapped+1) node->arcs.resize(remapped+1, nullptr);
        arc = new Arc(letter, factor, new_node, cost);
        node->arcs[remapped] = arc;
    }

    // Update the existing arc if factor was set
    else if (factor.length() > 0) {
        if (arc->factor.length() == 0) arc->factor = factor;
        arc->cost = cost;
    }

    // Maintain the length of the longest factor
    if ((int)factor.length() > max_factor_length)
        max_factor_length = factor.length();

    return arc->target_node;
}


StringSet::Arc*
StringSet::find_arc(char letter,
                    const StringSet::Node *node) const
{
    unsigned int remapped = remap_char(letter);
    if (remapped < node->arcs.size())
        return node->arcs[remapped];
    else
        return nullptr;
}


StringSet::Arc*
StringSet::find_arc_safe(char letter,
                         const StringSet::Node *node) const
{
    return node->arcs[remap_char(letter)];
}


bool
StringSet::includes(const string &factor) const
{
    const StringSet::Node *node = &root_node;
    StringSet::Arc *arc = nullptr;
    for (unsigned int i=0; i<factor.length(); i++) {
        arc = find_arc(factor[i], node);
        if (arc == nullptr) return false;
        node = arc->target_node;
    }
    if (arc->factor.length() == 0) return false;
    return true;
}


flt_type
StringSet::get_score(const string &factor) const
{
    const StringSet::Node *node = &root_node;
    StringSet::Arc *arc = nullptr;
    for (unsigned int i=0; i<factor.length(); i++) {
        arc = find_arc(factor[i], node);
        if (arc == nullptr) throw string("could not find factor");
        node = arc->target_node;
    }
    if (arc->factor.length() == 0) throw string("could not find factor");
    return arc->cost;
}


void
StringSet::add(const string &factor, flt_type cost)
{
    Node *node = &root_node;
    int i=0;
    for (; i < (int)factor.length()-1; i++)
        node = insert(factor[i], "" , 0.0, node);
    insert(factor[i], factor, cost, node);
}


flt_type
StringSet::remove(const string &factor)
{
    StringSet::Node *node = &root_node;
    StringSet::Arc *arc = nullptr;

    for (unsigned int i=0; i<factor.length(); i++) {
        arc = find_arc(factor[i], node);
        if (arc == nullptr) throw string("could not remove factor");
        node = arc->target_node;
    }
    arc->factor.clear();
    flt_type cost = arc->cost;
    arc->cost = 0.0;
    return cost;
}


void
StringSet::assign_scores(const map<string, flt_type> &vocab)
{
    for (auto it = vocab.begin(); it != vocab.end(); ++it) {
        string factor(it->first);
        Node *node = &root_node;
        int i=0;
        for (; i < (int)factor.length()-1; i++)
            node = insert(factor[i], "" , 0.0, node);
        insert(factor[i], factor, it->second, node);
    }

    vector<Arc*> arcs;
    collect_arcs(arcs);

    for (auto ait = arcs.begin(); ait != arcs.end(); ++ait) {
        if (vocab.find((*ait)->factor) == vocab.end()) {
            (*ait)->factor.clear();
            (*ait)->cost = 0.0;
        }
    }
}


void
StringSet::clear(Node *node)
{
    for (auto ait = node->arcs.begin(); ait != node->arcs.end(); ++ait) {
        if (*ait != nullptr) {
            clear((*ait)->target_node);
            delete (*ait)->target_node;
            delete *ait;
        }
    }
}


void
StringSet::collect_arcs(vector<Arc*> &arcs)
{
    vector<Node*> nodes_to_process;
    nodes_to_process.push_back(&root_node);

    while (nodes_to_process.size() > 0) {
        Node *node = nodes_to_process.back();
        nodes_to_process.pop_back();

        for (auto ait = node->arcs.begin(); ait != node->arcs.end(); ++ait) {
            if (*ait == nullptr) continue;
            arcs.push_back(*ait);
            nodes_to_process.push_back((*ait)->target_node);
        }
    }
}


unsigned int
StringSet::string_count()
{
    vector<Arc*> arcs;
    collect_arcs(arcs);

    unsigned int count = 0;
    for (auto ait = arcs.begin(); ait != arcs.end(); ++ait) {
        if (*ait == nullptr) continue;
        if ((*ait)->factor.length() > 0) count++;
    }
    return count;
}


void
StringSet::make_safe_end_nodes(const vector<string> &texts)
{
    for (auto tit = texts.begin(); tit != texts.end(); ++tit) {
        string text(*tit);
        for (int i=0; i<text.length(); i++) {
            Node *node = &root_node;
            for (int j=i; j<text.length(); j++) {
                unsigned int remapped = remap_char(text[j]);
                if (remapped+1 > node->arcs.size()) {
                    node->arcs.resize(remapped+1, nullptr);
                    break;
                }
                StringSet::Arc *arc = node->arcs[remapped];
                if (arc == nullptr) break;
                node = arc->target_node;
            }
        }
    }
}


void
StringSet::make_safe_end_nodes(const std::map<std::string, flt_type> &texts)
{
    vector<string> vtexts;
    for (auto tit = texts.begin(); tit != texts.end(); ++tit)
        vtexts.push_back(tit->first);
    make_safe_end_nodes(vtexts);
}

