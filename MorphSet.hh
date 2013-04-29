#ifndef MORPHSET_HH
#define MORPHSET_HH

#include <map>
#include <string>
#include <vector>

/** A structure containing a set of morphs in a letter-tree format.
 * Input letters and output morphs are stored in arcs. Nodes are just
 * placeholders for arcs. */

class MorphSet {
public:

  class Node;

  /** Arc of a morph tree. */
  class Arc {
  public:
    Arc(char letter, std::string morph, Node *target_node, Arc *sibling_arc, double cost=0.0)
      : letter(letter), morph(morph), target_node(target_node), cost(cost),
    sibling_arc(sibling_arc) { }

    char letter; //!< Letter of the morph
    std::string morph; //!< Non-zero if complete morph
    Node *target_node; //!< Target node
    double cost;
    Arc *sibling_arc; //!< Pointer to another arc from the source node.
  };

  /** Node of a morph tree. */
  class Node {
  public:
    Node() : first_arc(NULL) { }
    Node(Arc *first_arc) : first_arc(first_arc) { }
    Arc *first_arc; //!< The first arc of a list of arcs
  };

  /** Default constructor. */
  MorphSet();
  MorphSet(const std::map<std::string, double> &vocab);
  ~MorphSet();

  /** Insert a letter to a node (or follow an existing arc).
   * \param letter = a letter to insert to the node
   * \param morph = a possible morph corresponding to this node (can be empty)
   * \param node = a node to which the letter is inserted
   * \return pointer to the created or existing node
   */
  Node *insert(char letter, const std::string &morph, double cost, Node *node);

  /** Find an arc with the given letter from the given node.
   * \param letter = the letter to search
   * \param node = the source node
   * \return the arc containing the letter or NULL if no such arc exists
   */
  Arc *find_arc(char letter, const Node *node);

  /** Add a new morph to the set */
  void add(const std::string &morph, double cost);

  /** Remove a morph from the set
   * leaves the arc in tree, just nulls morph and cost
   * \return cost
   */
  double remove(const std::string &morph);

  Node root_node; //!< The root of the morph tree
  int max_morph_length; //!< The length of the longest morph in the set

private:
  // Just for destructor
  std::vector<Arc*> arcs;
  std::vector<Node*> nodes;
};

#endif /* MORPHSET_HH */
