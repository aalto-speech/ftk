#ifndef STRINGSET_HH
#define STRINGSET_HH

#include <map>
#include <string>
#include <vector>

#include "defs.hh"


/** A structure containing a set of strings in a letter-tree format.
 * Input letters and output strings are stored in arcs. Nodes are just
 * placeholders for arcs. */

class StringSet {
public:

    class Node;

    /** Arc of a string tree. */
    class Arc {
    public:
        Arc(char letter, std::string factor, Node *target_node, Arc *sibling_arc, flt_type cost=0.0)
            : letter(letter), factor(factor), target_node(target_node), cost(cost),
              sibling_arc(sibling_arc) { }

        char letter; //!< Letter of the factor
        std::string factor; //!< Non-zero if factor in vocabulary
        Node *target_node; //!< Target node
        flt_type cost;
        Arc *sibling_arc; //!< Pointer to another arc from the source node.
    };

    /** Node of a string tree. */
    class Node {
    public:
        Node() : first_arc(NULL) { }
        Node(Arc *first_arc) : first_arc(first_arc) { }
        Arc *first_arc; //!< The first arc of a list of arcs
    };

    /** Default constructor. */
    StringSet() : max_factor_length(0) { }
    StringSet(const std::map<std::string, flt_type> &vocab, bool log_domain=true);
    ~StringSet();

    /** Find an arc with the given letter from the given node.
     * \param letter = the letter to search
     * \param node = the source node
     * \return the arc containing the letter or NULL if no such arc exists
     */
    Arc* find_arc(char letter, const Node *node) const;

    /** Checks if the string is in stringset */
    bool includes(const std::string &factor) const;

    /** Get a score of a string in the stringset
        Throws if string not in stringset */
    flt_type get_score(const std::string &factor) const;

    /** Add a new factor to the set */
    void add(const std::string &factor, flt_type cost);

    /** Remove a factor from the set
     * leaves the arc in tree, just nulls factor and cost
     * \return cost
     */
    flt_type remove(const std::string &factor);

    /** Recursively sorts arcs in this node and each subnode,
     *  in descending order according to cumulative counts
     * \param node = initial node
     * \param log_domain = if the scores are in log domain (add scores in log domain, otherwise just +)
     * \return cumulative count of all subarcs reachable from this node
     */
    flt_type optimize_arcs(Node *node, bool log_domain = true);

    /** Assigns new scores to the StringSet
     * \param vocab = vocabulary of values to assign
     */
    void assign_scores(const std::map<std::string, flt_type> &vocab);

    /** Prunes unused arcs and nodes */
    void prune();

    Node root_node; //!< The root of the string tree
    int max_factor_length; //!< The length of the longest factor in the set

private:

    /** Insert a letter to a node (or follow an existing arc).
     * \param letter = a letter to insert to the node
     * \param factor = a possible factor corresponding to this node (can be empty)
     * \param node = a node to which the letter is inserted
     * \return pointer to the created or existing node
     */
    Node* insert(char letter, const std::string &factor, flt_type cost, Node *node);

    /** Destructor helper, deletes all arcs and subnodes
     * \param node = the source node
     */
    void clear(Node *node);

};


#endif /* STRINGSET_HH */
