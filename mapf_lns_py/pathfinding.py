import heapq
from collections import defaultdict

def manhattan(a, b):
    return abs(a[0] - b[0]) + abs(a[1] - b[1])

def get_neighbors(pos, grid):
    moves = [(-1,0),(1,0),(0,-1),(0,1),(0,0)]
    for dr, dc in moves:
        nr, nc = pos[0]+dr, pos[1]+dc
        if 0 <= nr < len(grid) and 0 <= nc < len(grid[0]) and grid[nr][nc]:
            yield (nr, nc)

class ConstraintTable:
    def __init__(self):
        self.vertex_constraints = defaultdict(set)  # (row, col) -> set of timesteps
        self.edge_constraints = defaultdict(set)    # ((row1, col1), (row2, col2)) -> set of timesteps
        self.goal_constraints = dict()              # (row, col) -> earliest timestep reserved forever

    def add_path(self, path):
        for t, pos in enumerate(path):
            self.vertex_constraints[pos].add(t)
            if t > 0:
                prev = path[t-1]
                self.edge_constraints[(prev, pos)].add(t)
        # Reserve goal for all future timesteps
        if path:
            goal = path[-1]
            self.goal_constraints[goal] = len(path)-1

    def is_constrained(self, curr, next_pos, t):
        # Vertex conflict
        if t in self.vertex_constraints[next_pos]:
            return True
        # Edge conflict
        if t in self.edge_constraints[(curr, next_pos)]:
            return True
        # Goal reservation
        if next_pos in self.goal_constraints and t >= self.goal_constraints[next_pos]:
            return True
        return False

def space_time_astar(grid, start, goal, constraint_table, max_t=5000, max_expansions=100000):
    heap = []
    heapq.heappush(heap, (manhattan(start, goal), 0, start, [start]))
    visited = set()
    expansions = 0
    while heap:
        f, t, curr, path = heapq.heappop(heap)
        if (curr, t) in visited:
            continue
        visited.add((curr, t))
        expansions += 1
        if expansions > max_expansions:
            print(f"A* expansion limit reached for agent from {start} to {goal}")
            return None
        if curr == goal:
            # Check if goal is reserved for all future timesteps
            reserved = False
            for future_t in range(t, t+20):
                if constraint_table.is_constrained(curr, curr, future_t):
                    reserved = True
                    break
            if not reserved:
                return path
        if t > max_t:
            continue
        for nbr in get_neighbors(curr, grid):
            if constraint_table.is_constrained(curr, nbr, t+1):
                continue
            heapq.heappush(heap, (t+1+manhattan(nbr, goal), t+1, nbr, path+[nbr]))
    return None

def prioritized_planning(instance, agents):
    constraint_table = ConstraintTable()
    paths = []
    for agent in agents:
        print(f"Planning for agent {agent.id} from {agent.start} to {agent.goal}")
        path = space_time_astar(instance.grid, agent.start, agent.goal, constraint_table)
        if not path:
            print(f"No path found for agent {agent.id} from {agent.start} to {agent.goal}")
            return None
        constraint_table.add_path(path)
        agent.path = path
        paths.append(path)
    return paths 