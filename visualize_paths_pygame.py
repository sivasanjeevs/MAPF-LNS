import sys
import re
import pygame
import time
import numpy as np

# Usage: python3 visualize_paths_pygame.py [path_file]

def parse_paths(filename):
    paths = []
    starts = []
    goals = []
    max_row = 0
    max_col = 0
    with open(filename, 'r') as f:
        for line in f:
            m = re.match(r'Agent (\d+):(.*)', line.strip())
            if not m:
                continue
            path_str = m.group(2)
            coords = re.findall(r'\((\d+),(\d+)\)', path_str)
            path = [(int(r), int(c)) for r, c in coords]
            if path:
                starts.append(path[0])
                goals.append(path[-1])
                max_row = max(max_row, max(r for r, _ in path))
                max_col = max(max_col, max(c for _, c in path))
            paths.append(path)
    return paths, starts, goals, max_row + 1, max_col + 1

def draw_grid(screen, nrows, ncols, cell_size, margin, grid_color):
    for x in range(ncols + 1):
        pygame.draw.line(screen, grid_color, (margin + x * cell_size, margin), (margin + x * cell_size, margin + nrows * cell_size), 1)
    for y in range(nrows + 1):
        pygame.draw.line(screen, grid_color, (margin, margin + y * cell_size), (margin + ncols * cell_size, margin + y * cell_size), 1)

def get_agent_colors(n_agents):
    # Use a set of visually distinct colors
    base_colors = [
        (31, 119, 180), (255, 127, 14), (44, 160, 44), (214, 39, 40),
        (148, 103, 189), (140, 86, 75), (227, 119, 194), (127, 127, 127),
        (188, 189, 34), (23, 190, 207), (255, 152, 150), (197, 176, 213)
    ]
    colors = []
    for i in range(n_agents):
        colors.append(base_colors[i % len(base_colors)])
    return colors

def animate_paths_pygame(paths, starts, goals, nrows, ncols):
    pygame.init()
    info = pygame.display.Info()
    screen_w, screen_h = info.current_w, info.current_h
    # Use 90% of the screen for the grid+legend
    max_grid_w = int(screen_w * 0.9)
    max_grid_h = int(screen_h * 0.9)
    legend_width = 200
    # Compute cell size and margin so grid fits
    cell_size_w = (max_grid_w - legend_width) // ncols
    cell_size_h = max_grid_h // nrows
    cell_size = min(cell_size_w, cell_size_h, 60)  # Cap cell size for aesthetics
    margin = max(30, min(60, cell_size // 2))
    grid_w = ncols * cell_size
    grid_h = nrows * cell_size
    width = grid_w + margin * 2 + legend_width
    height = grid_h + margin * 2
    screen = pygame.display.set_mode((width, height))
    pygame.display.set_caption('Multi-Agent Pathfinding Visualization (Pygame)')
    font = pygame.font.SysFont('Arial', max(12, cell_size // 3))
    small_font = pygame.font.SysFont('Arial', max(10, cell_size // 4))
    clock = pygame.time.Clock()
    grid_color = (200, 200, 200)
    bg_color = (255, 255, 255)
    makespan = max(len(p) for p in paths)
    n_agents = len(paths)
    colors = get_agent_colors(n_agents)
    running = True
    paused = False
    frame = 0
    speed = 1  # frames per second (1 timestep = 1 second)
    
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE:
                    paused = not paused
                elif event.key == pygame.K_RIGHT:
                    frame = min(frame + 1, makespan - 1)
                elif event.key == pygame.K_LEFT:
                    frame = max(frame - 1, 0)
                elif event.key == pygame.K_ESCAPE:
                    running = False
        if not paused:
            frame = (frame + 1) % makespan
        screen.fill(bg_color)
        draw_grid(screen, nrows, ncols, cell_size, margin, grid_color)
        # Draw start and goal markers
        for i, (s, g) in enumerate(zip(starts, goals)):
            # Start: faded circle
            pygame.draw.circle(screen, colors[i], (margin + s[1] * cell_size + cell_size // 2, margin + s[0] * cell_size + cell_size // 2), cell_size // 4, 0)
            pygame.draw.circle(screen, (255,255,255), (margin + s[1] * cell_size + cell_size // 2, margin + s[0] * cell_size + cell_size // 2), cell_size // 4, 2)
            # Goal: star
            center = (margin + g[1] * cell_size + cell_size // 2, margin + g[0] * cell_size + cell_size // 2)
            for angle in range(0, 360, 72):
                x1 = int(center[0] + cell_size // 4 * np.cos(np.radians(angle)))
                y1 = int(center[1] + cell_size // 4 * np.sin(np.radians(angle)))
                x2 = int(center[0] + cell_size // 8 * np.cos(np.radians(angle + 36)))
                y2 = int(center[1] + cell_size // 8 * np.sin(np.radians(angle + 36)))
                pygame.draw.line(screen, colors[i], center, (x1, y1), 2)
                pygame.draw.line(screen, colors[i], center, (x2, y2), 2)
        # Draw agent trails and current positions
        for i, path in enumerate(paths):
            trail = path[:min(frame+1, len(path))]
            if len(trail) > 1:
                points = [(margin + c * cell_size + cell_size // 2, margin + r * cell_size + cell_size // 2) for r, c in trail]
                pygame.draw.lines(screen, colors[i], False, points, max(2, cell_size // 15))
            # Current position
            pos = path[min(frame, len(path)-1)]
            pos_pix = (margin + pos[1] * cell_size + cell_size // 2, margin + pos[0] * cell_size + cell_size // 2)
            pygame.draw.circle(screen, colors[i], pos_pix, max(8, cell_size // 2 - 2))
            # Agent number
            text = font.render(str(i), True, (255,255,255))
            text_rect = text.get_rect(center=pos_pix)
            screen.blit(text, text_rect)
        # Draw legend
        legend_x = grid_w + margin * 2
        legend_y = margin
        legend_title = font.render('Agents', True, (0,0,0))
        screen.blit(legend_title, (legend_x, legend_y))
        for i in range(n_agents):
            pygame.draw.circle(screen, colors[i], (legend_x + 20, legend_y + 35 + i * 30), 12)
            agent_label = small_font.render(f'Agent {i}', True, (0,0,0))
            screen.blit(agent_label, (legend_x + 40, legend_y + 25 + i * 30))
        # Draw timestep
        timestep_text = font.render(f'Timestep: {frame}', True, (0,0,0))
        screen.blit(timestep_text, (margin, 10))
        # Instructions
        instr = small_font.render('SPACE: Pause/Play   ←/→: Step   ESC: Quit', True, (80,80,80))
        screen.blit(instr, (margin, height - 30))
        pygame.display.flip()
        if not paused:
            clock.tick(speed)
        else:
            clock.tick(15)
    pygame.quit()

def main():
    path_file = sys.argv[1] if len(sys.argv) > 1 else 'paths.txt'
    paths, starts, goals, nrows, ncols = parse_paths(path_file)
    animate_paths_pygame(paths, starts, goals, nrows, ncols)

if __name__ == '__main__':
    main() 