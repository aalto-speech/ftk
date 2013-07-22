#ifndef STRINGSET_HH
#define STRINGSET_HH

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>


/** A structure containing a set of strings in a letter-tree format.
 * Input letters and output strings are stored in arcs. Nodes are just
 * placeholders for arcs. */

template <typename T>
class StringSet {
public:

    class Node;

    /** Arc of a string tree. */
    class Arc {
    public:
        Arc(char letter, std::string factor, Node *target_node, Arc *sibling_arc, T cost=0.0)
            : letter(letter), factor(factor), target_node(target_node), cost(cost),
              sibling_arc(sibling_arc) { }

        char letter; //!< Letter of the factor
        std::string factor; //!< Non-zero if factor in vocabulary
        Node *target_node; //!< Target node
        T cost;
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

    StringSet(const std::map<std::string, T> &vocab) {
        max_factor_length = 0;
        for (auto it = vocab.cbegin(); it !=vocab.cend(); ++it)
            add(it->first, it->second);
    }

    ~StringSet() {
        for (unsigned int i=0; i<nodes.size(); i++)
            delete nodes[i];
        for (unsigned int i=0; i<arcs.size(); i++)
            delete arcs[i];
    }

    /** Insert a letter to a node (or follow an existing arc).
     * \param letter = a letter to insert to the node
     * \param factor = a possible factor corresponding to this node (can be empty)
     * \param node = a node to which the letter is inserted
     * \return pointer to the created or existing node
     */
    typename StringSet<T>::Node*
    insert(char letter, const std::string &factor, T cost, StringSet<T>::Node *node)
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

    /** Find an arc with the given letter from the given node.
     * \param letter = the letter to search
     * \param node = the source node
     * \return the arc containing the letter or NULL if no such arc exists
     */
    typename StringSet<T>::Arc*
    find_arc(char letter, const StringSet<T>::Node *node) const
    {
        Arc *arc = node->first_arc;
        while (arc != NULL) {
            if (arc->letter == letter) break;
            arc = arc->sibling_arc;
        }
        return arc;
    }

    /** Checks if the string is in stringset */
    bool
    includes(const std::string &factor) const
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

    /** Get a score of a string in the stringset
        Throws if string not in stringset */
    T
    get_score(const std::string &factor) const
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

    /** Add a new factor to the set */
    void
    add(const std::string &factor, T cost)
    {
        // Create arcs
        Node *node = &root_node;
        int i=0;
        for (; i < (int)factor.length()-1; i++)
            node = insert(factor[i], "" , 0.0, node);
        insert(factor[i], factor, cost, node);
    }

    /** Remove a factor from the set
     * leaves the arc in tree, just nulls factor and cost
     * \return cost
     */
    T
    remove(const std::string &factor)
    {
        StringSet::Node *node = &root_node;
        StringSet::Arc *arc = NULL;
        for (unsigned int i=0; i<factor.length(); i++) {
            arc = find_arc(factor[i], node);
            if (arc == NULL) throw std::string("could not remove factor");
            node = arc->target_node;
        }
        arc->factor.clear();
        T cost = arc->cost;
        arc->cost = 0.0;
        return cost;
    }

    Node root_node; //!< The root of the string tree
    int max_factor_length; //!< The length of the longest factor in the set

private:
    // Just for destructor
    std::vector<Arc*> arcs;
    std::vector<Node*> nodes;
};


#endif /* STRINGSET_HH */
