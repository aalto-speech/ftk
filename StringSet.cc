#include <iostream>

#include "StringSet.hh"

using namespace std;


StringSet::StringSet(const std::map<std::string, flt_type> &vocab) {
    max_factor_length = 0;
    for (auto it = vocab.cbegin(); it !=vocab.cend(); ++it)
        add(it->first, it->second);
    this->sort_arcs(&root_node);
}


StringSet::~StringSet() {
    for (unsigned int i=0; i<nodes.size(); i++)
        delete nodes[i];
    for (unsigned int i=0; i<arcs.size(); i++)
        delete arcs[i];
}


StringSet::Arc*
StringSet::find_arc(char letter,
                    const StringSet::Node *node) const
{
    Arc *arc = node->first_arc;
    while (arc != NULL) {
        if (arc->letter == letter) break;
        arc = arc->sibling_arc;
    }
    return arc;
}


bool
StringSet::includes(const string &factor) const
{
    const StringSet::Node *node = &root_node;
    StringSet::Arc *arc = NULL;
    for (unsigned int i=0; i<factor.length(); i++) {
        arc = find_arc(factor[i], node);
        if (arc == NULL) return false;
        node = arc->target_node;
    }
    if (arc->factor.length() == 0) return false;
    return true;
}


flt_type
StringSet::get_score(const string &factor) const
{
    const StringSet::Node *node = &root_node;
    StringSet::Arc *arc = NULL;
    for (unsigned int i=0; i<factor.length(); i++) {
        arc = find_arc(factor[i], node);
        if (arc == NULL) throw string("could not find factor");
        node = arc->target_node;
    }
    if (arc->factor.length() == 0) throw string("could not find factor");
    return arc->cost;
}


void
StringSet::add(const string &factor, flt_type cost)
{
    // Create arcs
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
    StringSet::Arc *arc = NULL;
    for (unsigned int i=0; i<factor.length(); i++) {
        arc = find_arc(factor[i], node);
        if (arc == NULL) throw string("could not remove factor");
        node = arc->target_node;
    }
    arc->factor.clear();
    flt_type cost = arc->cost;
    arc->cost = 0.0;
    return cost;
}


StringSet::Node*
StringSet::insert(char letter,
                  const string &factor,
                  flt_type cost,
                  StringSet::Node *node)
{
    // Find a possible existing arc with the letter
    Arc *arc = node->first_arc;
    while (arc != NULL) {
        if (arc->letter == letter) break;
        arc = arc->sibling_arc;
    }

    // No existing arc: create a new arc
    if (arc == NULL) {
        Node *new_node = new Node(NULL);
        node->first_arc = new Arc(letter, factor, new_node, node->first_arc, cost);
        arc = node->first_arc;
        nodes.push_back(new_node);
        arcs.push_back(arc);
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


flt_type
StringSet::sort_arcs(Node *node, bool log_domain)
{
    if (!log_domain) { throw string("Non-log-domain sort not implemented."); }

    flt_type total = SMALL_LP;
    StringSet::Arc *arc = node->first_arc;
    multimap<flt_type, Arc*> letters;
    if (arc == NULL) return SMALL_LP;

    while (arc != NULL) {
        flt_type cumsum = sort_arcs(arc->target_node);
        if (arc->factor.length() > 0)
            cumsum = add_log_domain_probs(cumsum, arc->cost);
        total = add_log_domain_probs(cumsum, total);
        letters.insert( pair<flt_type, Arc*>(cumsum, arc) );
        arc = arc->sibling_arc;
    }

    Arc* prev_arc = NULL;
    Arc* curr_arc = NULL;
    for (auto it = letters.begin(); it != letters.end(); ++it) {
        curr_arc = it->second;
        curr_arc->sibling_arc = prev_arc;
        prev_arc = curr_arc;
    }
    node->first_arc = curr_arc;

    return total;
}


flt_type
StringSet::sort_arcs(Node *node, const string &curr_prefix, const map<string, flt_type> &freqs)
{
    flt_type total = 0.0;
    StringSet::Arc *arc = node->first_arc;
    multimap<flt_type, Arc*> letters;
    if (arc == NULL) return 0.0;

    while (arc != NULL) {
        string curr_str(curr_prefix + arc->letter);
        flt_type cumsum = sort_arcs(arc->target_node, curr_str, freqs);

        if (arc->factor.length() > 0)
            cumsum += freqs.at(curr_str);
        total += cumsum;
        letters.insert( pair<flt_type, Arc*>(cumsum, arc) );
        arc = arc->sibling_arc;
    }

    Arc* prev_arc = NULL;
    Arc* curr_arc = NULL;
    for (auto it = letters.begin(); it != letters.end(); ++it) {
        curr_arc = it->second;
        curr_arc->sibling_arc = prev_arc;
        prev_arc = curr_arc;
    }
    node->first_arc = curr_arc;

    return total;
}


void
StringSet::assign_scores(const std::map<std::string, flt_type> &vocab)
{
    for (auto it = vocab.begin(); it != vocab.end(); ++it) {
        string factor(it->first);
        Node *node = &root_node;
        int i=0;
        for (; i < (int)factor.length()-1; i++)
            node = insert(factor[i], "" , 0.0, node);
        insert(factor[i], factor, it->second, node);
    }
}
