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
#include <map>

const int rows{ 10 };
const int columns{ 10 };
const int spacing{ 1 };
const int wall{ 3 };
const int player{ 1 };
const int goal{ 2 };
bool placeWalls{ false };
const int trueRowLength = rows - 1;
const int trueColLength = columns - 1;
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
* Change the OS check to use an Enum for easier readibility (theres way too many 0s0
*/

struct coordinate {
    int x{};
    int y{};

    // ask ivan why this was required to be made const. I may be stupid 
    bool compareValue(coordinate& other) const {
        return (this->x == other.x && this->y == other.y) ? true : false;
    }

    void moveUpNormal(bool adjustForVisual = true) {
        if (adjustForVisual) {
            this->x = std::clamp(this->x - stepSize, 0, trueRowLength);
        }
        else {
            this->y = std::clamp(this->y - stepSize, 0, trueRowLength);
        }
    }
    void moveDownNormal(bool adjustForVisual = true) {
        if (adjustForVisual) {
            this->x = std::clamp(this->x + stepSize, 0, trueRowLength);
        }
        else {
            this->y = std::clamp(this->y + stepSize, 0, trueRowLength);
        }
    }

    void moveLeftNormal(bool adjustForVisual = true) {
        if (adjustForVisual) {
            this->y = std::clamp(this->y - stepSize, 0, trueColLength);
        }
        else {
            this->x = std::clamp(this->x - stepSize - spacing, consoleStepSize, columns * consoleStepSize);
        }
    }

    void moveRightNormal(bool adjustForVisual = true) {
        // this would be funny if it worked
        // this-> (adustforVisual) ? y = std::clamp(this->y + stepSize, 0, trueColLength): x = std::clamp(this->x + stepSize + spacing, consoleStepSize, columns * consoleStepSize);;
        if (adjustForVisual) {
            this->y = std::clamp(this->y + stepSize, 0, trueColLength);
        }
        else {
            this->x = std::clamp(this->x + stepSize + spacing, consoleStepSize, columns * consoleStepSize);
        }
    }
};

// Potentially add support for Unix and MacOS cause why not. (JK im a lazy)S
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

void printGrid(std::array<std::array<int, rows>, columns>& grid, coordinate& p, coordinate& g) {

    for (int i = 0; i < grid.size() * grid[0].size(); i++) {
        
        int row = i / grid.size() ;
        int column = i % grid[0].size();
        
        
        if (row == p.x && column == p.y) {
            grid[row][column] = player;
        }
        else if (row == g.x && column == g.y) {
            grid[row][column] = goal;
        }

        std::cout << std::setw(spacing) << " " << grid[row][column]; 

        // Visual studio kept yelling at me to cast this because overflow or smthn
        if (static_cast<int16_t>( 1 + i ) % grid.size() == 0) std::cout << "\n";
        
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "\r" << std::flush << std::endl;
}

void moveAround(coordinate& p) {

    if (placeWalls) {
        int c = _getch();

        if (c == KeyBindings::ArrowPrefix[0] || c == KeyBindings::ArrowPrefix[1]) {
            c = _getch(); 
            switch (c) {
            case  KeyBindings::Up: 
                p.moveUpNormal();
                break;
            case  KeyBindings::Down:
                p.moveDownNormal();
                break;
            case  KeyBindings::Left: 
                p.moveLeftNormal();
                break;
            case  KeyBindings::Right:
                p.moveRightNormal();
                break;
            }
        }
        else if (c == KeyBindings::Escape) {
            exitProgram();
        }
    }
}


void moveCursor(std::array<std::array<int, rows>, columns>& grid, coordinate& cursor, coordinate& arrayTraverer, coordinate& goal, coordinate& playerPos) {

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
                grid[arrayTraverer.x][arrayTraverer.y] = wall;
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

void aStarSearch(coordinate& p, coordinate& g, std::array<std::array<int, rows>, columns>& grid) {

}



int main()
{
    if (checkOS() == 0) {
        bool play{ true };
        srand(time(0));
        std::array<std::array<int, rows>, columns> grid = {}; // Maze Grid
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
            moveAround(playerPos);

            // system("cls");
        }
        
    }
    else {
        std::cout << "This program currently only runs on windows, sorry";
    }
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file