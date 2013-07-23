#include <iostream>

#include "StringSet.hh"

using namespace std;


StringSet::StringSet(const std::map<std::string, flt_type> &vocab, bool log_domain) {
    max_factor_length = 0;
    for (auto it = vocab.cbegin(); it !=vocab.cend(); ++it)
        add(it->first, it->second);
    this->optimize_arcs(&root_node, log_domain);
}


StringSet::~StringSet() {
    clear(&root_node);
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
StringSet::optimize_arcs(Node *node, bool log_domain)
{
    flt_type total = log_domain ? SMALL_LP : 0.0;
    StringSet::Arc *arc = node->first_arc;
    multimap<flt_type, Arc*> letters;

    while (arc != NULL) {
        flt_type cumsum = optimize_arcs(arc->target_node, log_domain);
        if (log_domain) {
            if (arc->factor.length() > 0)
                cumsum = add_log_domain_probs(cumsum, arc->cost);
            total = add_log_domain_probs(cumsum, total);
        } else {
            if (arc->factor.length() > 0)
                cumsum += arc->cost;
            total += cumsum;
        }
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


void
StringSet::clear(Node *node)
{
    StringSet::Arc *curr_arc = node->first_arc;
    StringSet::Arc *temp_arc = NULL;

    while (curr_arc != NULL) {
        clear(curr_arc->target_node);
        temp_arc = curr_arc;
        curr_arc = curr_arc->sibling_arc;
        delete temp_arc->target_node;
        delete temp_arc;
    }
}


bool
StringSet::prune(Node *node)
{
    StringSet::Arc *curr_arc = node->first_arc;
    StringSet::Arc *temp_arc = NULL;

    std::vector<StringSet::Arc*> arcs;
    while (curr_arc != NULL) {
        bool unused = prune(curr_arc->target_node);
        temp_arc = curr_arc;
        curr_arc = curr_arc->sibling_arc;
        if (unused && temp_arc->factor.length() == 0) {
            delete temp_arc->target_node;
            delete temp_arc;
        } else {
            arcs.push_back(temp_arc);
        }
    }

    node->first_arc = NULL;
    for (int i=0; i<(int)arcs.size()-1; i++)
        arcs[i]->sibling_arc = arcs[i+1];
    if (arcs.size() > 0) {
        arcs[arcs.size()-1]->sibling_arc = NULL;
        node->first_arc = arcs[0];
    }

    if (arcs.size() > 0) return false;
    else return true;
}
