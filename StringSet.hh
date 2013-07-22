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
        Arc(char letter, std::string morph, Node *target_node, Arc *sibling_arc, T cost=0.0)
            : letter(letter), morph(morph), target_node(target_node), cost(cost),
              sibling_arc(sibling_arc) { }

        char letter; //!< Letter of the morph
        std::string morph; //!< Non-zero if complete morph
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
    StringSet() : max_morph_length(0) { }

    StringSet(const std::map<std::string, T> &vocab) {
        max_morph_length = 0;
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
     * \param morph = a possible morph corresponding to this node (can be empty)
     * \param node = a node to which the letter is inserted
     * \return pointer to the created or existing node
     */
    typename StringSet<T>::Node*
    insert(char letter, const std::string &morph, T cost, StringSet<T>::Node *node)
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
                std::cerr << "ERROR: StringSet::insert(): trying to redefine morph "
                          << morph << std::endl;
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
    includes(const std::string &morph) const
    {
        const StringSet::Node *node = &root_node;
        StringSet::Arc *arc = NULL;
        for (unsigned int i=0; i<morph.length(); i++) {
            arc = find_arc(morph[i], node);
            if (arc == NULL) return false;
            node = arc->target_node;
        }
        return true;
    }

    /** Get a score of a string in the stringset
        Throws if string not in stringset */
    T
    get_score(const std::string &morph) const
    {
        const StringSet::Node *node = &root_node;
        StringSet::Arc *arc = NULL;
        for (unsigned int i=0; i<morph.length(); i++) {
            arc = find_arc(morph[i], node);
            if (arc == NULL) throw std::string("could not find morph");
            node = arc->target_node;
        }
        return arc->cost;
    }


    /** Add a new morph to the set */
    void
    add(const std::string &morph, T cost)
    {
        // Create arcs
        Node *node = &root_node;
        int i=0;
        for (; i < (int)morph.length()-1; i++)
            node = insert(morph[i], "" , 0.0, node);
        insert(morph[i], morph, cost, node);
    }

    /** Remove a morph from the set
     * leaves the arc in tree, just nulls morph and cost
     * \return cost
     */
    T
    remove(const std::string &morph)
    {
        StringSet::Node *node = &root_node;
        StringSet::Arc *arc = NULL;
        for (unsigned int i=0; i<morph.length(); i++) {
            arc = find_arc(morph[i], node);
            if (arc == NULL) throw std::string("could not remove morph");
            node = arc->target_node;
        }
        arc->morph.clear();
        T cost = arc->cost;
        arc->cost = 0.0;
        return cost;
    }

    Node root_node; //!< The root of the morph tree
    int max_morph_length; //!< The length of the longest morph in the set

private:
    // Just for destructor
    std::vector<Arc*> arcs;
    std::vector<Node*> nodes;
};


#endif /* STRINGSET_HH */
