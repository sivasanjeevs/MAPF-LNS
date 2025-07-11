class Agent:
    def __init__(self, agent_id, start, goal):
        self.id = agent_id
        self.start = start
        self.goal = goal
        self.path = []  # List of (row, col)

class Path:
    def __init__(self, waypoints=None):
        self.waypoints = waypoints or []

    def __len__(self):
        return len(self.waypoints)

    def __getitem__(self, idx):
        return self.waypoints[idx]

    def append(self, pos):
        self.waypoints.append(pos) 