#include <iostream>
#include <vector>
#include <random>
#include <termios.h>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <iomanip>

int gridSize = 4;

int generateRandomTile();
std::vector<std::vector<int>> generateGrid();
void printGrid(const std::vector<std::vector<int>>& grid);
void addRandomTile(std::vector<std::vector<int>>& grid);
void addToGrid(const std::vector<std::vector<int>> &grid);
void setRawMode(bool enable);
enum direction{UP,DOWN,LEFT,RIGHT,NONE};
void moveAndMergeTiles(std::vector<std::vector<int>>& grid, direction dir);
bool isGameOver(const std::vector<std::vector<int>>& grid);
int main() {
    std::vector<std::vector<int>> grid = generateGrid();
    setRawMode(true);
    while (1){
        system("clear");
        printGrid(grid);
        char ch = std::cin.get();

        // Arrow keys are detected as escape sequences
        if (ch == 27) { // Escape character
            std::cin.get(); // [
            ch = std::cin.get(); // Direction

            direction dir = NONE;
            switch (ch) {
                case 'A': // Up arrow
                    dir = UP;
                    break;
                case 'B': // Down arrow
                    dir = DOWN;
                    break;
                case 'C': // Right arrow
                    dir = RIGHT;
                    break;
                case 'D': // Left arrow
                    dir = LEFT;
                    break;
            }
            if (dir != NONE){
                moveAndMergeTiles(grid, dir);
                if(!isGameOver(grid)){
                    addRandomTile(grid);
                }
                else{
                    std::cout << "Game over!"<<std::endl;
                    break;
                }
            }
        } else if (ch == 'q') {
            break; // Exit loop on 'q' press
        }
    }
    setRawMode(false);
    return 0;
}
int generateRandomTile() {
    std::random_device rd;                // Seed for the random number engine
    std::mt19937 gen(rd());               // Mersenne Twister random number engine
    std::uniform_int_distribution<> dist(1, 10); // Uniform distribution between 1 and 10

    // Return 4 if the random number is 1 (10% chance), otherwise return 2
    return (dist(gen) == 1) ? 4 : 2;
}

std::vector<std::vector<int>> generateGrid() {
    // Create a grid (2D vector) initialized with zeroes
    std::vector<std::vector<int>> grid(gridSize, std::vector<int>(gridSize, 0));

    // Random number engine for grid placement
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, gridSize - 1);

    // Place two random tiles (2 or 4) on the grid at random positions
    for (int i = 0; i < 2; ++i) {
        int row, col;
        do {
            row = dist(gen);
            col = dist(gen);
        } while (grid[row][col] != 0);  // Ensure the spot is empty

        grid[row][col] = generateRandomTile();
    }

    return grid;
}
void printGrid(const std::vector<std::vector<int>> &grid){
    for (const auto& row : grid) {
        for (int tile : row) {
            std::cout << std::setw(6) << std::right << tile << ' ';
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }
}

void setRawMode(bool enable) {
    static struct termios oldt, newt;

    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

void moveAndMergeTiles(std::vector<std::vector<int>>& grid, direction dir) {
    bool moved = false;
    int gridSize = grid.size();

    auto merge = [](std::vector<int>& row) {
        int n = row.size();
        for (int i = 0; i < n - 1; ++i) {
            if (row[i] == row[i + 1] && row[i] != 0) {
                row[i] *= 2;
                row[i + 1] = 0;
            }
        }
    };

    auto slideRow = [&moved](std::vector<int>& row) {
        std::vector<int> newRow;
        for (int val : row) {
            if (val != 0) {
                newRow.push_back(val);
            }
        }
        newRow.resize(row.size(), 0);
        moved = moved || (row != newRow);
        row = newRow;
    };

    if (dir == LEFT || dir == RIGHT) {
        for (int i = 0; i < gridSize; ++i) {
            if (dir == RIGHT) {
                std::reverse(grid[i].begin(), grid[i].end());
            }
            slideRow(grid[i]);
            merge(grid[i]);
            slideRow(grid[i]);
            if (dir == RIGHT) {
                std::reverse(grid[i].begin(), grid[i].end());
            }
        }
    } else if (dir == UP || dir == DOWN) {
        std::vector<std::vector<int>> tempGrid(gridSize, std::vector<int>(gridSize));
        for (int i = 0; i < gridSize; ++i) {
            for (int j = 0; j < gridSize; ++j) {
                tempGrid[i][j] = grid[j][i];
            }
        }
        moveAndMergeTiles(tempGrid, (dir == UP ? LEFT : RIGHT));
        for (int i = 0; i < gridSize; ++i) {
            for (int j = 0; j < gridSize; ++j) {
                grid[j][i] = tempGrid[i][j];
            }
        }
    }


}
bool isGameOver(const std::vector<std::vector<int>>& grid) {
    int gridSize = grid.size();

    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            if (grid[i][j] == 0 ||
                (i > 0 && grid[i][j] == grid[i - 1][j]) ||
                (j > 0 && grid[i][j] == grid[i][j - 1])) {
                return false;
            }
        }
    }
    return true;
}

void addRandomTile(std::vector<std::vector<int>>& grid) {
    std::vector<std::pair<int, int>> emptySpaces;
    int gridSize = grid.size();

    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            if (grid[i][j] == 0) {
                emptySpaces.push_back({i, j});
            }
        }
    }

    if (!emptySpaces.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, emptySpaces.size() - 1);
        auto [row, col] = emptySpaces[dist(gen)];
        grid[row][col] = generateRandomTile();
    }
}