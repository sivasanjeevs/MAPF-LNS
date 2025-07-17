#include "PathTable.h"

void PathTable::insertPath(int agent_id, const Path& path)
{
    if (path.empty())
        return;
    for (int t = 0; t < (int)path.size(); t++)
    {
        int loc = path[t].location;
        int ori = path[t].orientation;
        // Ensure table is large enough for loc
        if (loc >= (int)table.size())
            table.resize(loc + 1, std::vector<std::vector<int>>(4));
        // Ensure table[loc] is large enough for ori (should always be 4, but just in case)
        if (ori >= (int)table[loc].size())
            table[loc].resize(ori + 1);
        // Ensure table[loc][ori] is large enough for t
        if ((int)table[loc][ori].size() <= t)
            table[loc][ori].resize(t + 1, NO_AGENT);
        table[loc][ori][t] = agent_id;
    }
    // Ensure goals is large enough for the goal location
    if ((int)goals.size() <= path.back().location)
        goals.resize(path.back().location + 1, MAX_TIMESTEP);
    assert(goals[path.back().location] == MAX_TIMESTEP);
    goals[path.back().location] = (int) path.size() - 1;
    makespan = max(makespan, (int) path.size() - 1);
}

void PathTable::deletePath(int agent_id, const Path& path)
{
    if (path.empty())
        return;
    for (int t = 0; t < (int)path.size(); t++)
    {
        int loc = path[t].location;
        int ori = path[t].orientation;
        assert(table[loc][ori].size() > t && table[loc][ori][t] == agent_id);
        table[loc][ori][t] = NO_AGENT;
    }
    goals[path.back().location] = MAX_TIMESTEP;
    if (makespan == (int) path.size() - 1) // re-compute makespan
    {
        makespan = 0;
        for (int time : goals)
        {
            if (time < MAX_TIMESTEP && time > makespan)
                makespan = time;
        }
    }
}

bool PathTable::constrained(int from, int from_ori, int to, int to_ori, int to_time) const
{
    // Remove debug prints and error logging
    // Bounds checking
    if (from < 0 || to < 0 || from_ori < 0 || to_ori < 0 || to_time < 0) {
        return false;
    }
    if (from >= (int)table.size() || to >= (int)table.size()) {
        return false;
    }
    if (from_ori >= (int)table[from].size() || to_ori >= (int)table[to].size()) {
        return false;
    }
    if (!table.empty())
    {
        if ((int)table[to][to_ori].size() > to_time && table[to][to_ori][to_time] != NO_AGENT)
            return true;  // vertex conflict with agent table[to][to_ori][to_time]
        else if (table[to][to_ori].size() >= to_time && table[from][from_ori].size() > to_time && !table[to][to_ori].empty() &&
                 table[to][to_ori][to_time - 1] != NO_AGENT && table[from][from_ori][to_time] == table[to][to_ori][to_time - 1])
            return true;  // edge conflict with agent table[to][to_ori][to_time - 1]
    }
    if (!goals.empty())
    {
        if (goals[to] <= to_time)
            return true; // target conflict
    }
    return false;
}

void PathTable::getConflictingAgents(int agent_id, set<int>& conflicting_agents, int from, int from_ori, int to, int to_ori, int to_time) const
{
    if (table.empty())
        return;
    if (table[to][to_ori].size() > to_time && table[to][to_ori][to_time] != NO_AGENT)
        conflicting_agents.insert(table[to][to_ori][to_time]); // vertex conflict
    if (table[to][to_ori].size() >= to_time && table[from][from_ori].size() > to_time &&
        table[to][to_ori][to_time - 1] != NO_AGENT && table[from][from_ori][to_time] == table[to][to_ori][to_time - 1])
        conflicting_agents.insert(table[from][from_ori][to_time]); // edge conflict
    // TODO: collect target conflicts as well.
}

void PathTable::get_agents(set<int>& conflicting_agents, int loc, int ori) const
{
    if (loc < 0 || ori < 0)
        return;
    for (auto agent : table[loc][ori])
    {
        if (agent >= 0)
            conflicting_agents.insert(agent);
    }
}

// get the holding time after the earliest_timestep for a location
int PathTable::getHoldingTime(int location, int orientation, int earliest_timestep) const
{
    if (table.empty() or (int) table[location][orientation].size() <= earliest_timestep)
        return earliest_timestep;
    int rst = (int) table[location][orientation].size();
    while (rst > earliest_timestep and table[location][orientation][rst - 1] == NO_AGENT)
        rst--;
    return rst;
}

void PathTableWC::insertPath(int agent_id, const Path& path)
{
    paths[agent_id] = &path;
    if (path.empty())
        return;
    for (int t = 0; t < (int)path.size(); t++)
    {
        int loc = path[t].location;
        int ori = path[t].orientation;
        if (table[loc][ori].size() <= t)
            table[loc][ori].resize(t + 1);
        table[loc][ori][t].push_back(agent_id);
    }
    assert(goals[path.back().location] == MAX_TIMESTEP);
    goals[path.back().location] = (int) path.size() - 1;
    makespan = max(makespan, (int) path.size() - 1);
}
void PathTableWC::insertPath(int agent_id)
{
    assert(paths[agent_id] != nullptr);
    insertPath(agent_id, *paths[agent_id]);
}
void PathTableWC::deletePath(int agent_id)
{
    const Path & path = *paths[agent_id];
    if (path.empty())
        return;
    for (int t = 0; t < (int)path.size(); t++)
    {
        int loc = path[t].location;
        int ori = path[t].orientation;
        assert(table[loc][ori].size() > t &&
               std::find (table[loc][ori][t].begin(), table[loc][ori][t].end(), agent_id)
               != table[loc][ori][t].end());
        table[loc][ori][t].remove(agent_id);
    }
    goals[path.back().location] = MAX_TIMESTEP;
    if (makespan == (int) path.size() - 1) // re-compute makespan
    {
        makespan = 0;
        for (int time : goals)
        {
            if (time < MAX_TIMESTEP && time > makespan)
                makespan = time;
        }

    }
}

int PathTableWC::getFutureNumOfCollisions(int loc, int ori, int time) const
{
    assert(goals[loc] == MAX_TIMESTEP);
    int rst = 0;
    if (!table.empty() && (int)table[loc][ori].size() > time)
    {
        for (int t = time + 1; t < (int)table[loc][ori].size(); t++)
            rst += (int)table[loc][ori][t].size();  // vertex conflict
    }
    return rst;
}

int PathTableWC::getNumOfCollisions(int from, int from_ori, int to, int to_ori, int to_time) const
{
    int rst = 0;
    if (!table.empty())
    {
        if ((int)table[to][to_ori].size() > to_time)
            rst += (int)table[to][to_ori][to_time].size();  // vertex conflict
        if (from != to && table[to][to_ori].size() >= to_time && table[from][from_ori].size() > to_time)
        {
            for (auto a1 : table[to][to_ori][to_time - 1])
            {
                for (auto a2: table[from][from_ori][to_time])
                {
                    if (a1 == a2)
                        rst++; // edge conflict
                }
            }
        }
    }
    if (!goals.empty())
    {
        if (goals[to] <= to_time)
            rst++; // target conflict
    }
    return rst;
}
bool PathTableWC::hasCollisions(int from, int from_ori, int to, int to_ori, int to_time) const
{
    if (!table.empty())
    {
        if ((int)table[to][to_ori].size() > to_time && !table[to][to_ori][to_time].empty())
            return true; // vertex conflict
        if (from != to && table[to][to_ori].size() >= to_time && table[from][from_ori].size() > to_time)
        {
            for (auto a1 : table[to][to_ori][to_time - 1])
            {
                for (auto a2: table[from][from_ori][to_time])
                {
                    if (a1 == a2)
                        return true; // edge conflict
                }
            }
        }
    }
    if (!goals.empty())
    {
        if (goals[to] <= to_time)
            return true; // target conflict
    }
    return false;
}
bool PathTableWC::hasEdgeCollisions(int from, int from_ori, int to, int to_ori, int to_time) const
{
    if (!table.empty() && from != to && table[to][to_ori].size() >= to_time && table[from][from_ori].size() > to_time)
    {
        for (auto a1 : table[to][to_ori][to_time - 1])
        {
            for (auto a2: table[from][from_ori][to_time])
            {
                if (a1 == a2)
                    return true; // edge conflict
            }
        }
    }
    return false;
}

int PathTableWC::getAgentWithTarget(int target_location, int target_orientation, int latest_timestep) const
{
    if (table.empty())
        return NO_AGENT;
    for (int t = 0; t <= latest_timestep && t < (int)table[target_location][target_orientation].size(); t++)
    {
        for (int agent : table[target_location][target_orientation][t])
        {
            if (agent != NO_AGENT)
                return agent;
        }
    }
    return NO_AGENT;
}

int PathTableWC::getLastCollisionTimestep(int location, int orientation) const
{
    if (table.empty())
        return -1;
    int last = -1;
    for (int t = 0; t < (int)table[location][orientation].size(); t++)
    {
        if (!table[location][orientation][t].empty())
            last = t;
    }
    return last;
}

void PathTableWC::clear()
{
    table.clear();
    goals.clear();
    paths.clear();
}