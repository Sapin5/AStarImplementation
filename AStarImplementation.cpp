// AStarImplementation.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <array>
#include <iomanip>
#include <chrono>
#include <thread>
#include <conio.h>
#include <algorithm>
#include <Windows.h>
#include <list>

const int rows{ 10 };
const int columns{ 10 };
const int spacing{ 1 };
const int wall{ 3 };
const int player{ 1 };
const int goal{ 2 };
bool placeWalls{ false };
const int trueRowLength{ rows - 1 };
const int trueColLength{ columns - 1 };
const int stepSize{ 1 };
const int consoleStepSize{ 2 };

// I hate this
struct KeyBindings {
    static constexpr int Space = 32;
    static constexpr int Enter[2] = { 13, 10 };
    static constexpr int Up = 72;
    static constexpr int Down = 80;
    static constexpr int Left = 75;
    static constexpr int Right = 77;
    static constexpr int Escape = 27;
    static constexpr int ArrowPrefix[2] = { 0, 224 };
};

enum CellType {
    EMPTY,
    PLAYER,
    GOAL,
    WALL
};



/*
* To Do
* Get rid of all magic numbers <- I dont want to do this. Code is art and should be left to interpertation <- done
* Implement game state change from wall placement to map traversal  <- done
* Astar algorithm  <- almost done theres a bug somewhere I cant find
*/

struct coordinate {
    int x{};
    int y{};

    // ask ivan why this was required to be made const. I may be stupid 
    bool compareValue(coordinate& other) const {
        return (this->x == other.x && this->y == other.y);
    }

    void moveUpNormal(bool adjustForVisual = true) {
        (adjustForVisual) ? this->x = std::clamp(this->x - stepSize, 0, trueRowLength) : 
                            this->y = std::clamp(this->y - stepSize, 0, trueRowLength);
    }
    void moveDownNormal(bool adjustForVisual = true) {
        (adjustForVisual) ? this->x = std::clamp(this->x + stepSize, 0, trueRowLength) :
                            this->y = std::clamp(this->y + stepSize, 0, trueRowLength);
    }

    void moveLeftNormal(bool adjustForVisual = true) {
        (adjustForVisual) ? this->y = std::clamp(this->y - stepSize, 0, trueColLength) :
                            this->x = std::clamp(this->x - stepSize - spacing, consoleStepSize, columns * consoleStepSize);
    }

    void moveRightNormal(bool adjustForVisual = true) {
        (adjustForVisual) ? this->y = std::clamp(this->y + stepSize, 0, trueColLength) : 
                            this->x = std::clamp(this->x + stepSize + spacing, consoleStepSize, columns * consoleStepSize);
    }


    bool operator==(const coordinate& other) const {
        return (x == other.x && y == other.y);
    }

    bool operator!=(const coordinate& other) const {
        return !(*this == other);
    }
};


struct Cell {
    int id{ 0 };
    CellType type{ EMPTY };
    coordinate loc{};
    int weight{ 1 }; 
    int g{ 10000 };
    int f{ 0 };
    coordinate parent{ -1, -1 };
};


// Potentially add support for Unix and MacOS cause why not. (JK im a lazy)
const short checkOS() {
    short OS{};
    #if defined(_WIN32)
        OS = 0;
    #elif defined(__APPLE__) || defined(__MACH__)
        OS = 1;
    #elif defined(__linux__)
        OS = 2;
    #else
        OS = 3;
    #endif
    return OS;
}


void gotoxy(short int x, short int y) {
    HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position{ x, y };
    ::SetConsoleCursorPosition(hStdOut, position);
}


void exitProgram() {
    gotoxy(0, rows + 10);
    std::exit(0);
}

void printGrid(std::array<std::array<Cell, rows>, columns>& grid) {

    for (int i = 0; i < grid.size() * grid[0].size(); i++) {
        
        int row = i / grid.size() ;
        int column = i % grid[0].size();
       

        std::cout << std::setw(spacing) << " " << grid[row][column].type;

        // Visual studio kept yelling at me to cast this because overflow or smthn
        if (static_cast<int16_t>( 1 + i ) % grid.size() == 0) std::cout << "\n";
        
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "\r" << std::flush << std::endl;
}

void moveAround(coordinate& p, std::array<std::array<Cell, rows>, columns>& grid) {

    bool CanMoveUp    = grid[std::clamp(p.x - stepSize, 0, trueRowLength)][p.y].type != WALL;
    bool CanMoveDown  = grid[std::clamp(p.x + stepSize, 0, trueRowLength)][p.y].type != WALL;
    bool CanMoveLeft  = grid[p.x][std::clamp(p.y - stepSize, 0, trueColLength)].type != WALL;
    bool CanMoveRight = grid[p.x][std::clamp(p.y + stepSize, 0, trueColLength)].type != WALL;

    if (placeWalls) {
        int c = _getch();

        if (c == KeyBindings::ArrowPrefix[0] || c == KeyBindings::ArrowPrefix[1]) {
            c = _getch(); 
            switch (c) {
            case  KeyBindings::Up:
                if (CanMoveUp) {
                    p.moveUpNormal();
                    grid[p.x][p.y].type = PLAYER;
                }
                break;
            case  KeyBindings::Down:
                if (CanMoveDown) {
                    p.moveDownNormal();
                    grid[p.x][p.y].type = PLAYER;
                }
                break;
            case  KeyBindings::Left:
                if (CanMoveLeft) {
                    p.moveLeftNormal();
                    grid[p.x][p.y].type = PLAYER;
                }
                break;
            case  KeyBindings::Right:
                if (CanMoveRight) {
                    p.moveRightNormal();
                    grid[p.x][p.y].type = PLAYER;
                }
                break;
            }
        }
        else if (c == KeyBindings::Escape) {
            exitProgram();
        }
    }

    if (!CanMoveUp && !CanMoveDown && !CanMoveLeft && !CanMoveRight) {
        std::exit(0);
    }
}


void moveCursor(std::array<std::array<Cell, rows>, columns>& grid, coordinate& cursor, coordinate& arrayTraverer, coordinate& goal, coordinate& playerPos) {

    if (!placeWalls) {
        int c = _getch();

        if (c == KeyBindings::ArrowPrefix[0] || c == KeyBindings::ArrowPrefix[1]) {

            c = _getch();
            switch (c) {
            case KeyBindings::Up:
                cursor.moveUpNormal(false);
                arrayTraverer.moveUpNormal();
                break;
            case KeyBindings::Down:
                cursor.moveDownNormal(false);
                arrayTraverer.moveDownNormal();
                break;
            case KeyBindings::Left:
                cursor.moveLeftNormal(false);
                arrayTraverer.moveLeftNormal();
                break;
            case KeyBindings::Right:
                cursor.moveRightNormal(false);
                arrayTraverer.moveRightNormal();
                break;
            }

        }
        else if (c == KeyBindings::Space) {
            if (!arrayTraverer.compareValue(goal) && !arrayTraverer.compareValue(playerPos)) {
                grid[arrayTraverer.x][arrayTraverer.y].type = WALL;
            }
        }
        else if (c == KeyBindings::Enter[0] || c == KeyBindings::Enter[1]) {
            placeWalls = true;
            gotoxy(0, rows + 2);
        }
        else if (c == KeyBindings::Escape) {
            exitProgram();
        }
    }
}

bool checkPosition(coordinate& p, coordinate& g) {
    if (p.x == g.x && p.y == g.y) {
        gotoxy(0, rows + 2);
        std::cout << "YOU WIN" << std::endl;
        return false;
    }
    return true;
}

// Manhattan distance between player and goal
// D represents cost of moving around

// heuristic *= (1.0 + p) <- The p will help break the tie and select a path if there are multiple
// p should be less than the (minimum cost of taking one step) / (expected maximum path length)

/*
* This Will cause the heuristic to prefer straight lines over the curves
* dx1 = current.x - goal.x
* dy1 = current.y - goal.y
* dx2 = start.x - goal.x
* dy2 = start.y - goal.y
* cross = abs(dx1*dy2 - dx2*dy1)
* heuristic += cross*0.001
*/
int heuristic(coordinate& p, coordinate& g) {
    int D = 1; // Cost of moving, Cost is determined. This would alos work as the cell weight. Will not be pulling though
    int dx = abs(p.x - g.x);
    int dy = abs(p.y - g.y);
    return D * (dx + dy);
}

std::vector<coordinate> neighbors(coordinate& node, std::array<std::array<Cell, rows>, columns>& grid) {

    int nearestCells[4][2] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
    std::vector<coordinate> results;
    for (auto& test : nearestCells) {
        coordinate neighbour = { test[0] + node.x, test[1] + node.y};
        if (0 <= neighbour.x  && neighbour.x < columns && 0 <= neighbour.y && neighbour.y < rows) results.push_back(neighbour);
    }
    return results;
}

// Theres a memory leak somewhere in here. I hit 31GB of ram usage
std::vector<coordinate> aStarSearch(coordinate& p, coordinate& g, std::array<std::array<Cell, rows>, columns>& grid) {

    // OPEN = priority queue containing START
    std::vector<coordinate> open{p};
    grid[p.x][p.y].g = 0;
    grid[p.x][p.y].f = heuristic(p, g);
    grid[p.x][p.y].parent = { -1, -1 };

    // CLOSED = empty set
    std::vector<coordinate> closed{};

    // while lowest rank in OPEN is not the GOAL:
    while (!open.empty()) {

        // current = remove lowest rank item from OPEN
        // This took abt 3 hours to figure out
        auto comparator = [&grid, &p, &g](const coordinate& a, const coordinate& b) {
            return grid[a.x][a.y].f < grid[b.x][b.y].f;
        };
        std::vector<coordinate>::iterator current_ = std::min_element(open.begin(), open.end(), comparator);
        coordinate current = *current_;
        open.erase(current_);

        // add current to CLOSED
        closed.push_back(current);
        if (current == g) break;
        // for neighbors of current:
        for (coordinate& neighbor : neighbors(current, grid)) {
            
            // Ignore walls
            // Maybe move this to neighbor check
            if (grid[neighbor.x][neighbor.y].type == WALL) continue;

            // cost = g(current) + movementcost(current, neighbor)
            float cost = grid[current.x][current.y].g + grid[neighbor.x][neighbor.y].weight;
           
            // if neighbor in OPEN and cost less than g(neighbor):
            // remove neighbor from OPEN, because new path is better
            float neighborG = grid[neighbor.x][neighbor.y].g;
            auto inOpen = std::find(open.begin(), open.end(), neighbor);
            if (inOpen != open.end() && cost < neighborG) {
                open.erase(inOpen);
                inOpen = open.end(); // Clear the search query. Needed for running another find later with no issues
            }

            // if neighbor in CLOSED and cost less than g(neighbor): ⁽²⁾
            // remove neighbor from CLOSED
            auto inClosed = std::find(closed.begin(), closed.end(), neighbor);
            if (inClosed != closed.end() && cost < neighborG) {
                closed.erase(inClosed);
                inClosed = closed.end();
            }

            // if neighbor not in OPEN and neighbor not in CLOSED:
            // set g(neighbor) to cost
            // add neighbor to OPEN
            // set priority queue rank to g(neighbor) + h(neighbor)
            // set neighbor's parent to current
            if (inOpen == open.end() && inClosed == closed.end()) {
                grid[neighbor.x][neighbor.y].g = cost;
                grid[neighbor.x][neighbor.y].f = cost + heuristic(neighbor, g);
                grid[neighbor.x][neighbor.y].parent = current;
                open.push_back(neighbor);
            }
        }
        
    }

    // Reconstruct reverse path from goal to start
    std::vector<coordinate> path;
    coordinate curr = g;
    // start node's parent is {-1, -1}
    while (curr.x != -1 && curr.y != -1) {
        path.push_back(curr);
        //  if (curr == p) break; // break once we reach start
        curr = grid[curr.x][curr.y].parent;
    }
    // Reverse so it goes start -> end
    std::reverse(path.begin(), path.end());
    
    return path;
}


int main()
{
    if (checkOS() == 0) {
        bool play{ true };
        srand(time(0));
        
        std::array<std::array<Cell, rows>, columns> grid = {}; // Maze Grid
        coordinate playerPos{ 0, 0 };
        coordinate goal{ rand() % rows, rand() % columns };
        coordinate cursorLoc{ consoleStepSize, 0 };
        coordinate arrayTraverser{ 0, 0 };
        std::vector<coordinate> path{};

        int tracker = 0;
        for (int i = 0; i < grid.size(); i++) {
            for (int j = 0; j < grid[0].size(); j++) {
                grid[i][j].id = tracker;
                tracker++;
                grid[i][j].loc.x = i;
                grid[i][j].loc.y = j;

                if (playerPos.x == grid[i][j].loc.x && playerPos.y == grid[i][j].loc.y) {
                    grid[i][j].type = PLAYER;
                }
                else if(goal.x == grid[i][j].loc.x && goal.y == grid[i][j].loc.y){
                    grid[i][j].type = GOAL;
                }
                else {
                    grid[i][j].type = EMPTY;
                }
            }
        }


        while (play) {
            if (!checkPosition(playerPos, goal)) break;
            system("cls");
            printGrid(grid);
            gotoxy(cursorLoc.x, cursorLoc.y);
            moveCursor(grid, cursorLoc, arrayTraverser, goal, playerPos);
           if (placeWalls) { 
                path = aStarSearch(playerPos, goal, grid); 
                play = false;
            }
            // moveAround(playerPos, grid);
        }

        if (path.size() <= 1) {
            std::cout << "No Path Found";
        }
        else {
            std::cout << "Path Found" << std::endl;
            for (auto& test : path) {
                std::cout << "{ " << test.x << ", " << test.y << " } ";
            }
        }
    }
    else {
        std::cout << "This program currently only runs on windows, sorry";
    }
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu