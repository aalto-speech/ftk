#include <iostream>
#include <cstdlib>

#include "MorphSet.hh"


MorphSet::MorphSet() : max_morph_length(0) { }

MorphSet::MorphSet(const std::map<std::string, double> &vocab) {
    max_morph_length = 0;
    for (auto it = vocab.cbegin(); it !=vocab.cend(); ++it)
        add(it->first, it->second);
}


MorphSet::~MorphSet() {
    for (unsigned int i=0; i<nodes.size(); i++)
        delete nodes[i];
    for (unsigned int i=0; i<arcs.size(); i++)
        delete arcs[i];
 }


MorphSet::Node*
MorphSet::insert(char letter, const std::string &morph, double cost, Node *node)
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
      node->first_arc = new Arc(letter, morph, new_node, node->first_arc, cost);
      arc = node->first_arc;
      nodes.push_back(new_node);
      arcs.push_back(arc);
    }

    // Update the existing arc if morph was set
    else if (morph.length() > 0) {
        if (arc->morph.length() > 0) {
            fprintf(stderr,
                    "ERROR: MorphSet::insert(): trying to redefine morph %s\n",
                    morph.c_str());
            exit(1);
        }
        arc->morph = morph;
        arc->cost = cost;
    }

    // Maintain the length of the longest morph
    if ((int)morph.length() > max_morph_length)
        max_morph_length = morph.length();

    return arc->target_node;
}

MorphSet::Arc*
MorphSet::find_arc(char letter, const Node *node) const
{
    Arc *arc = node->first_arc;
    while (arc != NULL) {
        if (arc->letter == letter) break;
        arc = arc->sibling_arc;
    }
    return arc;
}

void
MorphSet::add(const std::string &morph, double cost)
{
    // Create arcs
    Node *node = &root_node;
    int i=0;
    for (; i < (int)morph.length()-1; i++)
        node = insert(morph[i], "" , 0.0, node);
    insert(morph[i], morph, cost, node);
}

double
MorphSet::remove(const std::string &morph)
{
    MorphSet::Node *node = &root_node;
    MorphSet::Arc *arc;
    for (unsigned int i=0; i<morph.length(); i++) {
        arc = find_arc(morph[i], node);
        if (arc == NULL) throw std::string("could not remove morph");
        node = arc->target_node;
    }
    arc->morph.clear();
    double cost = arc->cost;
    arc->cost = 0.0;
    return cost;
}
