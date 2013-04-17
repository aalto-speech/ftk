#include <stdlib.h>
#include <vector>
#include "MorphSet.hh"

MorphSet::MorphSet() : max_morph_length(0) { }

MorphSet::Node*
MorphSet::insert(char letter, const std::string &morph, Node *node)
{
  // Find a possible existing arc with the letter
  Arc *arc = node->first_arc;
  while (arc != NULL) {
    if (arc->letter == letter)
      break;
    arc = arc->sibling_arc;
  }

  // No existing arc: create a new arc
  if (arc == NULL) {
    node->first_arc = new Arc(letter, morph, new Node(NULL), node->first_arc);
    arc = node->first_arc;
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
  }

  // Maintain the length of the longest morph
  if ((int)morph.length() > max_morph_length)
    max_morph_length = morph.length();

  return arc->target_node;
}

MorphSet::Arc*
MorphSet::find_arc(char letter, const Node *node)
{
  Arc *arc = node->first_arc;
  while (arc != NULL) {
    if (arc->letter == letter)
      break;
    arc = arc->sibling_arc;
  }
  return arc;
}

void
MorphSet::add(const std::string &morph)
{
    // Create arcs
    Node *node = &root_node;
    for (int i = 0; i < (int)morph.length(); i++)
      node = insert(morph[i], i < (int)morph.length() - 1 ? "" : morph, node);
}
