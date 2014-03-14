#include <algorithm>
#include <iostream>

#include "StringSet.hh"

using namespace std;


StringSet::StringSet(const std::map<std::string, flt_type> &vocab)
{
    max_factor_length = 0;
    learn_map(vocab);
    root_node.arcs.resize(character_count, nullptr);
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
        unsigned int remapped = remap_char(letter); // unsigned char!
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
    unsigned char remapped = remap_char(letter);
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


bool rank_desc_sort(pair<unsigned char, int> i,pair<unsigned char, int> j) { return (i.second > j.second); }
void
StringSet::learn_map(const map<string, flt_type> &vocab)
{
    for (int i=0; i<256; i++) charmap[i] = 255;

    map<unsigned char, int> charcounts;
    for (auto it = vocab.begin(); it != vocab.end(); ++it) {
        for (const char &chr : it->first) charcounts[(unsigned char)chr]++;
    }

    vector<pair<unsigned char, int> > sorted_charcounts;
    for (auto it = charcounts.begin(); it != charcounts.end(); ++it) {
        pair<unsigned char, int> ccount = make_pair(it->first, it->second);
        sorted_charcounts.push_back(ccount);
    }
    sort(sorted_charcounts.begin(), sorted_charcounts.end(), rank_desc_sort);

    unsigned char idx = 0;
    character_count = charcounts.size();
    for (auto it = sorted_charcounts.begin(); it != sorted_charcounts.end(); ++it) {
        charmap[it->first] = idx;
        idx++;
    }
}
