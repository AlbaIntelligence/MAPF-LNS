﻿#pragma once
#include "SingleAgentSolver.h"
#include "ReservationTable.h"

class SIPPNode: public LLNode
{
public:
	// define a typedefs for handles to the heaps (allow up to quickly update a node in the heap)
	typedef boost::heap::pairing_heap< SIPPNode*, compare<SIPPNode::compare_node> >::handle_type open_handle_t;
	typedef boost::heap::pairing_heap< SIPPNode*, compare<SIPPNode::secondary_compare_node> >::handle_type focal_handle_t;
	open_handle_t open_handle;
	focal_handle_t focal_handle;
	Interval interval;

	SIPPNode() : LLNode() {}
	SIPPNode(int loc, int g_val, int h_val, SIPPNode* parent, int timestep, const Interval& interval,
	        int num_of_conflicts) :
		LLNode(loc, g_val, h_val, parent, timestep, num_of_conflicts), interval(interval) {}
	SIPPNode(const SIPPNode& other): LLNode(other), open_handle(other.open_handle) {} // copy everything except for handles
	~SIPPNode() {}

	void copy(const SIPPNode& other)
    {
	    LLNode::copy(other);
	    interval = other.interval;
    }
	// The following is used by for generating the hash value of a nodes
	struct NodeHasher
	{
		std::size_t operator()(const SIPPNode* n) const
		{
			size_t loc_hash = std::hash<int>()(n->location);
			//size_t timestep_hash = std::hash<size_t>()(get<1>(n->interval));
			return loc_hash; //(loc_hash ^ (timestep_hash << 1));
		}
	};

	// The following is used for checking whether two nodes are equal
	// we say that two nodes, s1 and s2, are equal if
	// both are non-NULL and agree on the id and timestep
	struct eqnode
	{
		bool operator()(const SIPPNode* n1, const SIPPNode* n2) const
		{
			return (n1 == n2) ||
			            (n1 && n2 && n1->location == n2->location &&
				        n1->wait_at_goal == n2->wait_at_goal); //&&
                        //get<1>(n1->interval) == get<1>(n2->interval));
                        //max(n1->timestep, n2->timestep) <
                        //min(get<1>(n1->interval), get<1>(n2->interval))); //overlapping time intervals
		}
	};
};

// Structure to represent a node in Interval Search Tree
struct ITNode
{
    SIPPNode* n;
    int max; // the max upper bound for the subtree
    ITNode *left, *right;
};

class SIPP: public SingleAgentSolver
{
public:

    // find path by SIPP
	// Returns a shortest path that satisfies the constraints of the give node  while
	// minimizing the number of internal conflicts (that is conflicts with known_paths for other agents found so far).
	// lowerbound is an underestimation of the length of the path in order to speed up the search.
    //Path findOptimalPath(const PathTable& path_table) {return Path(); } // TODO: To implement
    //Path findOptimalPath(const ConstraintTable& constraint_table, const PathTableWC& path_table);
	Path findOptimalPath(const HLNode& node, const ConstraintTable& initial_constraints,
		const vector<Path*>& paths, int agent, int lowerbound);
	pair<Path, int> findSuboptimalPath(const HLNode& node, const ConstraintTable& initial_constraints,
		const vector<Path*>& paths, int agent, int lowerbound, double w);  // return the path and the lowerbound
    Path findPath(const ConstraintTable& constraint_table); // return A path that minimizes collisions, breaking ties by cost
    int getTravelTime(int start, int end, const ConstraintTable& constraint_table, int upper_bound);

	string getName() const { return "SIPP"; }

	SIPP(const Instance& instance, int agent):
		SingleAgentSolver(instance, agent) {}

private:
	// define typedefs and handles for heap
	typedef boost::heap::pairing_heap< SIPPNode*, boost::heap::compare<LLNode::compare_node> > heap_open_t;
	typedef boost::heap::pairing_heap< SIPPNode*, boost::heap::compare<LLNode::secondary_compare_node> > heap_focal_t;
	heap_open_t open_list;
	heap_focal_t focal_list;

	// define typedef for hash_map
	//typedef boost::unordered_map<SIPPNode*, ITNode*, SIPPNode::NodeHasher, SIPPNode::eqnode> hashtable_t;
	vector<ITNode*> allNodes_table;

    Path findNoCollisionPath(const ConstraintTable& constraint_table);
	void generateChild(const Interval& interval, SIPPNode* curr, int next_location, int next_timestep,
		const ReservationTable& reservation_table);
    void generateChildToFocal(const Interval& interval, SIPPNode* curr, int next_location,
            int next_timestep, int next_h_val);
	
	// Updates the path datamember
	static void updatePath(const LLNode* goal, std::vector<PathEntry> &path);
	inline SIPPNode* popNode();
	inline void pushNode(SIPPNode* node);
	void updateFocalList();
	void releaseNodes();
    void mergeNodes(SIPPNode* old_node, SIPPNode* new_node);
    static bool dominanceCheck(ITNode* root, SIPPNode* new_node);
    void updateNodeToFocal(SIPPNode* old_node, const SIPPNode* new_node);
	void printSearchTree() const;

	// Interval tree search - used by the allNodes_table
    static ITNode* newNode(SIPPNode* n);
    static ITNode *insert(ITNode *root, SIPPNode* n);
    static bool doOVerlap(SIPPNode* n1, SIPPNode* n2);
    static void overlapSearch(ITNode *root, SIPPNode* n, list<SIPPNode*>& overlaps);
    static void deleteNodes(ITNode *root);
};

