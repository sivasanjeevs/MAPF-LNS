#include "SpaceTimeAStar.h"


void SpaceTimeAStar::updatePath(const LLNode* goal, vector<PathEntry> &path)
{
    num_collisions = goal->num_of_conflicts;
    const LLNode* curr = goal;
    if (curr->is_goal)
        curr = curr->parent;
	path.reserve(curr->g_val + 1);
    while (curr != nullptr)
    {
        path.emplace_back(curr->location, curr->orientation);
        curr = curr->parent;
    }
    std::reverse(path.begin(), path.end());
}


Path SpaceTimeAStar::findOptimalPath(const HLNode& node, const ConstraintTable& initial_constraints,
	const vector<Path*>& paths, int agent, int lowerbound)
{
	return findSuboptimalPath(node, initial_constraints, paths, agent, lowerbound, 1).first;
}

// find path by time-space A* search
// Returns a path that minimizes the collisions with the paths in the path table, breaking ties by the length
Path SpaceTimeAStar::findPath(const ConstraintTable& constraint_table)
{
    reset();
    Path path;
    if (constraint_table.constrained(start_location, 0, 0))
    {
        return path;
    }
    auto holding_time = constraint_table.getHoldingTime(goal_location, 0, constraint_table.length_min); // TODO: use correct orientation if available
    auto static_timestep = constraint_table.getMaxTimestep() + 1; // everything is static after this timestep
    auto last_target_collision_time = constraint_table.getLastCollisionTimestep(goal_location, 0); // TODO: use correct orientation if available
    // generate start and add it to the OPEN & FOCAL list
    auto h = max(max(my_heuristic[start_location], holding_time), last_target_collision_time + 1);
    auto start = new AStarNode(start_location, 0, h, 0, nullptr, 0, 0);
    num_generated++;
    start->in_openlist = true;
    start->focal_handle = focal_list.push(start); // we only use focal list; no open list is used
    allNodes_table.insert(start);
    while (!focal_list.empty())
    {
        auto* curr = focal_list.top();
        focal_list.pop();
        curr->in_openlist = false;
        num_expanded++;
        assert(curr->location >= 0);
        // check if the popped node is a goal
        if (curr->is_goal)
        {
            updatePath(curr, path);
            break;
        }
        else if (curr->location == goal_location and // arrive at the goal location
                 !curr->wait_at_goal and // not wait at the goal location
                 curr->timestep >= holding_time) // the agent can hold the goal location afterward
         {
            int future_collisions = constraint_table.getFutureNumOfCollisions(curr->location, curr->orientation, curr->timestep);
            if (future_collisions == 0)
            {
                updatePath(curr, path);
                break;
            }
            // generate a goal node
            auto goal = new AStarNode(*curr);
            goal->is_goal = true;
            goal->parent = curr;
            goal->num_of_conflicts += future_collisions;
            goal->h_val = 0;
            // try to retrieve it from the hash table
            auto it = allNodes_table.find(goal);
            if (it == allNodes_table.end())
            {
                goal->focal_handle = focal_list.push(goal);
                goal->in_openlist = true;
                num_generated++;
                allNodes_table.insert(goal);
            }
            else // update existing node's if needed (only in the open_list)
            {
                auto existing_next = *it;
                if (existing_next->num_of_conflicts > goal->num_of_conflicts ||
                   (existing_next->num_of_conflicts == goal->num_of_conflicts &&
                    existing_next->getFVal() > goal->getFVal()))
                {
                    assert(existing_next->in_openlist);
                    existing_next->copy(*goal);	// update existing node
                    focal_list.update(existing_next->focal_handle);
                    num_generated++; // reopen is considered as a new node
                }
                delete (goal);
            }
        }
        if (curr->timestep >= constraint_table.length_max)
            continue;

        auto next_states = instance.getNextStates(curr->location, curr->orientation);
        for (const auto& [next_location, next_orientation] : next_states)
        {
            int next_timestep = curr->timestep + 1;
            if (static_timestep < next_timestep)
            {
                if (next_location == curr->location && next_orientation == curr->orientation)
                {
                    continue;
                }
                next_timestep--;
            }
            if (constraint_table.constrained(next_location, next_orientation, next_timestep) ||
                constraint_table.constrained(curr->location, curr->orientation, next_location, next_orientation, next_timestep))
                continue;
            int next_g_val = curr->g_val + 1;
            int next_h_val = my_heuristic[next_location];
            if (next_g_val + next_h_val > constraint_table.length_max)
                continue;
            auto num_conflicts = curr->num_of_conflicts +
                    constraint_table.getNumOfConflictsForStep(curr->location, curr->orientation, next_location, next_orientation, next_timestep);
            if (num_conflicts == 0)
                next_h_val = max(next_h_val, curr->getFVal() - next_g_val);  // path max
            else
                next_h_val = max(next_h_val, holding_time - next_g_val); // path max
            // generate (maybe temporary) node
            auto next = new AStarNode(next_location, next_orientation, next_g_val, next_h_val,
                                      curr, next_timestep, num_conflicts);
            if (next_location == goal_location && curr->location == goal_location && next_orientation == curr->orientation)
                next->wait_at_goal = true;
            // try to retrieve it from the hash table
            auto it = allNodes_table.find(next);
            if (it == allNodes_table.end())
            {
                pushNode(next);
                allNodes_table.insert(next);
                continue;
            }
            // update existing node's if needed (only in the open_list)
            auto existing_next = *it;
            if (existing_next->getFVal() > next->getFVal() ||
                (existing_next->getFVal() == next->getFVal() &&
                    existing_next->num_of_conflicts > next->num_of_conflicts))
            {
                if (!existing_next->in_openlist)
                {
                    existing_next->copy(*next);
                    pushNode(existing_next);
                }
                else
                {
                    bool add_to_focal = false;
                    bool update_in_focal = false;
                    bool update_open = false;
                    if ((next_g_val + next_h_val) <= w * min_f_val)
                    {
                        if (existing_next->getFVal() > w * min_f_val)
                            add_to_focal = true;
                        else
                            update_in_focal = true;
                    }
                    if (existing_next->getFVal() > next_g_val + next_h_val)
                        update_open = true;
                    existing_next->copy(*next);
                    if (update_open)
                        open_list.increase(existing_next->open_handle);
                    if (add_to_focal)
                        existing_next->focal_handle = focal_list.push(existing_next);
                    if (update_in_focal)
                        focal_list.update(existing_next->focal_handle);
                }
            }
            delete(next);
        }
    }  // end while loop

    releaseNodes();
    return path;
}

// find path by time-space A* search
// Returns a bounded-suboptimal path that satisfies the constraints of the give node  while
// minimizing the number of internal conflicts (that is conflicts with known_paths for other agents found so far).
// lowerbound is an underestimation of the length of the path in order to speed up the search.
pair<Path, int> SpaceTimeAStar::findSuboptimalPath(const HLNode& node, const ConstraintTable& initial_constraints,
	const vector<Path*>& paths, int agent, int lowerbound, double w)
{
    reset();
	this->w = w;
	Path path;

	// build constraint table
	auto t = clock();
    ConstraintTable constraint_table(initial_constraints);
    constraint_table.insert2CT(node, agent);
	runtime_build_CT = (double)(clock() - t) / CLOCKS_PER_SEC;
	if (constraint_table.constrained(start_location, 0, 0))
	{
		return {path, 0};
	}

	t = clock();
    constraint_table.insert2CAT(agent, paths);
	runtime_build_CAT = (double)(clock() - t) / CLOCKS_PER_SEC;

	// the earliest timestep that the agent can hold its goal location. The length_min is considered here.
    auto holding_time = constraint_table.getHoldingTime(goal_location, 0, constraint_table.length_min);
    auto static_timestep = constraint_table.getMaxTimestep() + 1; // everything is static after this timestep
    lowerbound =  max(holding_time, lowerbound);

	// generate start and add it to the OPEN & FOCAL list
	auto start = new AStarNode(start_location, 0, max(lowerbound, my_heuristic[start_location]), 0, nullptr, 0, 0);

	num_generated++;
	start->open_handle = open_list.push(start);
	start->focal_handle = focal_list.push(start);
	start->in_openlist = true;
	allNodes_table.insert(start);
	min_f_val = (int) start->getFVal();
	// lower_bound = int(w * min_f_val));

	while (!open_list.empty())
	{
		updateFocalList(); // update FOCAL if min f-val increased
		auto* curr = popNode();
        assert(curr->location >= 0);
		// check if the popped node is a goal
		if (curr->location == goal_location && // arrive at the goal location
			!curr->wait_at_goal && // not wait at the goal location
			curr->timestep >= holding_time) // the agent can hold the goal location afterward
		{
			updatePath(curr, path);
			break;
		}

		if (curr->timestep >= constraint_table.length_max)
			continue;

		auto next_locations = instance.getNeighbors(curr->location);
		next_locations.emplace_back(curr->location);
		for (int next_location : next_locations)
		{
			int next_timestep = curr->timestep + 1;
			if (static_timestep < next_timestep)
			{ // now everything is static, so switch to space A* where we always use the same timestep
				if (next_location == curr->location)
				{
					continue;
				}
				next_timestep--;
			}

			if (constraint_table.constrained(next_location, 0, next_timestep) ||
				constraint_table.constrained(curr->location, 0, next_location, 0, next_timestep))
				continue;

			// compute cost to next_id via curr node
			int next_g_val = curr->g_val + 1;
			int next_h_val = max(lowerbound - next_g_val, my_heuristic[next_location]);
			if (next_g_val + next_h_val > constraint_table.length_max)
				continue;
			int next_internal_conflicts = curr->num_of_conflicts +
				constraint_table.getNumOfConflictsForStep(curr->location, 0, next_location, 0, next_timestep);

			// generate (maybe temporary) node
			auto next = new AStarNode(next_location, 0, next_g_val, next_h_val,
				curr, next_timestep, next_internal_conflicts);
			if (next_location == goal_location && curr->location == goal_location)
				next->wait_at_goal = true;

			// try to retrieve it from the hash table
			auto it = allNodes_table.find(next);
			if (it == allNodes_table.end())
			{
				pushNode(next);
				allNodes_table.insert(next);
				continue;
			}
			// update existing node's if needed (only in the open_list)

			auto existing_next = *it;
			if (existing_next->getFVal() > next->getFVal() || // if f-val decreased through this new path
				(existing_next->getFVal() == next->getFVal() &&
					existing_next->num_of_conflicts > next->num_of_conflicts)) // or it remains the same but there's fewer conflicts
			{
				if (!existing_next->in_openlist) // if its in the closed list (reopen)
				{
					existing_next->copy(*next);
					pushNode(existing_next);
				}
				else
				{
					bool add_to_focal = false;  // check if it was above the focal bound before and now below (thus need to be inserted)
					bool update_in_focal = false;  // check if it was inside the focal and needs to be updated (because f-val changed)
					bool update_open = false;
					if ((next_g_val + next_h_val) <= w * min_f_val)
					{  // if the new f-val qualify to be in FOCAL
						if (existing_next->getFVal() > w * min_f_val)
							add_to_focal = true;  // and the previous f-val did not qualify to be in FOCAL then add
						else
							update_in_focal = true;  // and the previous f-val did qualify to be in FOCAL then update
					}
					if (existing_next->getFVal() > next_g_val + next_h_val)
						update_open = true;

					existing_next->copy(*next);	// update existing node

					if (update_open)
						open_list.increase(existing_next->open_handle);  // increase because f-val improved
					if (add_to_focal)
						existing_next->focal_handle = focal_list.push(existing_next);
					if (update_in_focal)
						focal_list.update(existing_next->focal_handle);  // should we do update? yes, because number of conflicts may go up or down			
				}
			}

			delete(next);  // not needed anymore -- we already generated it before
		}  // end for loop that generates successors
	}  // end while loop

	releaseNodes();
	return {path, min_f_val};
}


int SpaceTimeAStar::getTravelTime(int start, int end, const ConstraintTable& constraint_table, int upper_bound)
{
    reset();
	int length = MAX_TIMESTEP;
    auto static_timestep = constraint_table.getMaxTimestep() + 1; // everything is static after this timestep
	auto root = new AStarNode(start, 0, compute_heuristic(start, end), 0, nullptr, 0, 0);
	root->open_handle = open_list.push(root);  // add root to heap
	allNodes_table.insert(root);       // add root to hash_table (nodes)
	AStarNode* curr = nullptr;
	while (!open_list.empty())
	{
		curr = open_list.top(); open_list.pop();
		if (curr->location == end)
		{
			length = curr->g_val;
			break;
		}
		list<int> next_locations = instance.getNeighbors(curr->location);
		next_locations.emplace_back(curr->location);
		for (int next_location : next_locations)
		{
			int next_timestep = curr->timestep + 1;
			int next_g_val = curr->g_val + 1;
			if (static_timestep < next_timestep)
			{
				if (curr->location == next_location)
				{
					continue;
				}
				next_timestep--;
			}
			if (!constraint_table.constrained(next_location, 0, next_timestep) &&
				!constraint_table.constrained(curr->location, 0, next_location, 0, next_timestep))
			{  // if that grid is not blocked
				int next_h_val = compute_heuristic(next_location, end);
				if (next_g_val + next_h_val >= upper_bound) // the cost of the path is larger than the upper bound
					continue;
				auto next = new AStarNode(next_location, 0, next_g_val, next_h_val, nullptr, next_timestep, 0);
				auto it = allNodes_table.find(next);
				if (it == allNodes_table.end())
				{  // add the newly generated node to heap and hash table
					next->open_handle = open_list.push(next);
					allNodes_table.insert(next);
				}
				else {  // update existing node's g_val if needed (only in the heap)
					delete(next);  // not needed anymore -- we already generated it before
					auto existing_next = *it;
					if (existing_next->g_val > next_g_val)
					{
						existing_next->g_val = next_g_val;
						existing_next->timestep = next_timestep;
						open_list.increase(existing_next->open_handle);
					}
				}
			}
		}
	}
	releaseNodes();
    num_expanded = 0;
    num_generated = 0;
    num_reopened = 0;
	return length;
}

inline AStarNode* SpaceTimeAStar::popNode()
{
	auto node = focal_list.top(); focal_list.pop();
	open_list.erase(node->open_handle);
	node->in_openlist = false;
	num_expanded++;
	return node;
}


inline void SpaceTimeAStar::pushNode(AStarNode* node)
{
	node->open_handle = open_list.push(node);
	node->in_openlist = true;
	num_generated++;
	if (node->getFVal() <= w * min_f_val)
		node->focal_handle = focal_list.push(node);		
}


void SpaceTimeAStar::updateFocalList()
{
	auto open_head = open_list.top();
	if (open_head->getFVal() > min_f_val)
	{
		int new_min_f_val = (int)open_head->getFVal();
		for (auto n : open_list)
		{
			if (n->getFVal() >  w * min_f_val && n->getFVal() <= w * new_min_f_val)
				n->focal_handle = focal_list.push(n);
		}
		min_f_val = new_min_f_val;
	}
}


void SpaceTimeAStar::releaseNodes()
{
	open_list.clear();
	focal_list.clear();
	for (auto node: allNodes_table)
		delete node;
	allNodes_table.clear();
}

