#include <iostream>

#include "StringSet.hh"


StringSet::Arc*
StringSet::find_arc(char letter, const StringSet::Node *node) const
{
    Arc *arc = node->first_arc;
    while (arc != NULL) {
        if (arc->letter == letter) break;
        arc = arc->sibling_arc;
    }
    return arc;
}


bool
StringSet::includes(const std::string &factor) const
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
StringSet::get_score(const std::string &factor) const
{
    const StringSet::Node *node = &root_node;
    StringSet::Arc *arc = NULL;
    for (unsigned int i=0; i<factor.length(); i++) {
        arc = find_arc(factor[i], node);
        if (arc == NULL) throw std::string("could not find factor");
        node = arc->target_node;
    }
    if (arc->factor.length() == 0) throw std::string("could not find factor");
    return arc->cost;
}


void
StringSet::add(const std::string &factor, flt_type cost)
{
    // Create arcs
    Node *node = &root_node;
    int i=0;
    for (; i < (int)factor.length()-1; i++)
        node = insert(factor[i], "" , 0.0, node);
    insert(factor[i], factor, cost, node);
}


flt_type
StringSet::remove(const std::string &factor)
{
    StringSet::Node *node = &root_node;
    StringSet::Arc *arc = NULL;
    for (unsigned int i=0; i<factor.length(); i++) {
        arc = find_arc(factor[i], node);
        if (arc == NULL) throw std::string("could not remove factor");
        node = arc->target_node;
    }
    arc->factor.clear();
    flt_type cost = arc->cost;
    arc->cost = 0.0;
    return cost;
}


StringSet::Node*
StringSet::insert(char letter,
       const std::string &factor,
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
        if (arc->factor.length() > 0) {
            std::cerr << "ERROR: StringSet::insert(): trying to redefine factor "
                      << factor << std::endl;
            exit(1);
        }
        arc->factor = factor;
        arc->cost = cost;
    }

    // Maintain the length of the longest factor
    if ((int)factor.length() > max_factor_length)
        max_factor_length = factor.length();

    return arc->target_node;
}


flt_type
StringSet::sort(Node *node, bool log_domain)
{
    StringSet::Arc *arc = node->first_arc;
    if (arc == NULL) return SMALL_LP;
    return 0;
}
