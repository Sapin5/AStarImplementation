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


struct Cell {
    int id{ 0 };
    std::string type;
    int weight{ 0 };
};

/* cant use this beacuase switch statement wants a constexpr int 
const std::map<std::string, std::vector<const int>> keys = {
    {"Space",  {32}},
    {"Enter",  {13, 10}},
    {"Up",     {72}},
    {"Down",   {80}},
    {"Left",   {75}},
    {"Right",  {77}},
    {"Escape", {27}}
};
*/

/*
* To Do
* Get rid of all magic numbers <- I dont want to do this. Code is art and should be left to interpertation
* Implement game state change from wall placement to map traversal
* Astar algorithm
* Change the OS check to use an Enum for easier readibility (theres way too many 0s
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

void printGrid(std::array<std::array<Cell, rows>, columns>& grid, coordinate& p, coordinate& g) {

    for (int i = 0; i < grid.size() * grid[0].size(); i++) {
        
        int row = i / grid.size() ;
        int column = i % grid[0].size();
        
        
        if (row == p.x && column == p.y) {
            grid[row][column].id = player;
        }
        else if (row == g.x && column == g.y) {
            grid[row][column].id = goal;
        }

        std::cout << std::setw(spacing) << " " << grid[row][column].id; 

        // Visual studio kept yelling at me to cast this because overflow or smthn
        if (static_cast<int16_t>( 1 + i ) % grid.size() == 0) std::cout << "\n";
        
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "\r" << std::flush << std::endl;
}

void moveAround(coordinate& p, std::array<std::array<Cell, rows>, columns>& grid) {

    bool CanMoveUp    = grid[std::clamp(p.x - stepSize, 0, trueRowLength)][p.y].id != 3;
    bool CanMoveDown  = grid[std::clamp(p.x + stepSize, 0, trueRowLength)][p.y].id != 3;
    bool CanMoveLeft  = grid[p.x][std::clamp(p.y - stepSize, 0, trueColLength)].id != 3;
    bool CanMoveRight = grid[p.x][std::clamp(p.y + stepSize, 0, trueColLength)].id != 3;

    if (placeWalls) {
        int c = _getch();

        if (c == KeyBindings::ArrowPrefix[0] || c == KeyBindings::ArrowPrefix[1]) {
            c = _getch(); 
            switch (c) {
            case  KeyBindings::Up:
                if (CanMoveUp) {
                    p.moveUpNormal();
                }
                break;
            case  KeyBindings::Down:
                if (CanMoveDown) {
                    p.moveDownNormal();
                }
                break;
            case  KeyBindings::Left:
                if (CanMoveLeft) {
                    p.moveLeftNormal();
                }
                break;
            case  KeyBindings::Right:
                if (CanMoveRight) {
                    p.moveRightNormal();
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
                grid[arrayTraverer.x][arrayTraverer.y].id = wall;
            }
        }
        else if (c == KeyBindings::Enter[0] || c == KeyBindings::Enter[1]) {
            placeWalls = true;
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
* This Will cause the heuristic to prefer stright lines over the curves
* dx1 = current.x - goal.x
* dy1 = current.y - goal.y
* dx2 = start.x - goal.x
* dy2 = start.y - goal.y
* cross = abs(dx1*dy2 - dx2*dy1)
* heuristic += cross*0.001
*/
int heuristic(coordinate& p, coordinate& g) {
    int D = 1; // Cost of moving, Cost is determined  
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

void aStarSearch(coordinate& p, coordinate& g, std::array<std::array<int, rows>, columns>& grid) {

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

        while (play) {
            if (!checkPosition(playerPos, goal)) break;
            system("cls");
            printGrid(grid, playerPos, goal);
            gotoxy(cursorLoc.x, cursorLoc.y);
            moveCursor(grid, cursorLoc, arrayTraverser, goal, playerPos);
            //aStarSearch(playerPos, goal, grid);
            moveAround(playerPos, grid);
        }
        
    }
    else {
        std::cout << "This program currently only runs on windows, sorry";
    }
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu