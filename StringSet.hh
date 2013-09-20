#ifndef STRINGSET_HH
#define STRINGSET_HH

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
        Arc(char letter, std::string factor, Node *target_node, flt_type cost=0.0)
            : letter(letter), factor(factor), target_node(target_node), cost(cost) { }
        char letter; //!< Letter of the factor
        std::string factor; //!< Non-zero if factor in vocabulary
        Node *target_node; //!< Target node
        flt_type cost;
    };

    /** Node of a string tree. */
    class Node {
    public:
        Node(int character_count = 0) { arcs.resize(character_count, nullptr); }
        std::vector<Arc*> arcs;
    };

    /** Default constructor. */
    StringSet() : max_factor_length(0) { };
    StringSet(const std::map<std::string, flt_type> &vocab);
    ~StringSet();

    /** Find an arc with the given letter from the given node.
     * \param letter = the letter to search
     * \param node = the source node
     * \return the arc containing the letter or nullptr if no such arc exists
     */
    Arc* find_arc(char letter, const Node *node) const;

    /** Find an arc with the given letter from the given node.
     * \param letter = the letter to search
     * \param node = the source node
     * \return the arc containing the letter or NULL if no such arc exists
     */
    Arc* find_arc_safe(char letter, const Node *node) const;

    /** Find an arc with the given letter from the given node.
     * \param letter = the letter to search
     * \param node = the source node
     * \return the arc containing the letter or nullptr if no such arc exists
     */
    void insert_arc(char letter, const Node *node) const;

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

    /** Assigns new scores to the StringSet
     * \param vocab = vocabulary of values to assign
     */
    void assign_scores(const std::map<std::string, flt_type> &vocab);

    /** Returns the number of stored strings */
    unsigned int string_count();

    Node root_node; //!< The root of the string tree
    int max_factor_length; //!< The length of the longest factor in the set
    unsigned char charmap[256];
    int character_count;

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

    /** Pruning helper, deletes all unused arcs and subnodes
     * \param node = the source node
     * \return true if all arcs and subnodes of node were unused and deleted
     */
    bool prune(Node *node);

    /** Helper, collects all arcs in the StringSet
     */
    void collect_arcs(std::vector<Arc*> &arcs);

    /** Helper, constructs a compact letter mapping to range [0,|C|]
     */
    void learn_map(const std::map<std::string, flt_type> &vocab);

    inline unsigned char remap_char(char orig) const
    {
        return charmap[(unsigned char)orig];
    }

};


#endif /* STRINGSET_HH */
