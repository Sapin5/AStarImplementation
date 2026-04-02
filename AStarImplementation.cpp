// AStarImplementation.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <array>
#include <iomanip>
#include <chrono>
#include <thread>
#include <conio.h>
#include <algorithm>

const int rows{ 10 };
const int columns{ 10 };
const int spacing{ 1 };

struct coordinate {
    int x{};
    int y{};
};


void printGrid(std::array<std::array<int, rows>, columns>& grid, coordinate& p, coordinate& g) {
    //std::string grid = "";
    for (int i = 0; i < grid.size() * grid[0].size(); i++) {
        
        int row = i / grid.size() ;
        int column = i % grid[0].size();
        
        
        if (row == p.x && column == p.y) {
            grid[row][column] = 1;
        }
        else if (row == g.x && column == g.y) {
            grid[row][column] = 2;
        }

        std::cout << std::setw(spacing) << " " << grid[row][column];

        // Visual studio kept yelling at me to cast this because overflow or smthn
        if (static_cast<int16_t>( 1 + i ) % grid.size() == 0) std::cout << "\n";
        
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "\r" << std::flush << std::endl;
}

void moveAround(coordinate& p) {
    int c = _getch();

    // Check for arrow key prefix (0 or 224)
    if (c == 0 || c == 224) {
        c = _getch(); // Get the actual key code
        switch (c) {
        case 72: // Up
            p.x = std::clamp(p.x - 1, 0, rows - 1);
            break;
        case 80: // Down
            p.x = std::clamp(p.x + 1, 0, rows - 1);
            break;
        case 75: // Left
            p.y = std::clamp(p.y - 1, 0, columns - 1);
            break;
        case 77: // Right
            p.y = std::clamp(p.y + 1, 0, columns - 1);
            break;
        }
    }
    else if (c == 27) { // Escape key
        std::exit(0);
    }
}

bool checkPosition(coordinate& p, coordinate& g) {
    if (p.x == g.x && p.y == g.y) {
        std::cout << "YOU WIN" << std::endl;
        return false;
    }
    return true;
}

void aStarSearch(coordinate& p, coordinate& g, std::array<std::array<int, rows>, columns>& grid) {

}

int main()
{
    bool play{ true };
    srand(time(0));
    std::array<std::array<int, rows>, columns> grid = {};
    coordinate playerPos{ 0, 0 };
    coordinate goal{ rand() % rows, rand() % columns };

    while (play) {
        if (!checkPosition(playerPos, goal)) break;
        printGrid(grid, playerPos, goal);
        
        aStarSearch(playerPos, goal, grid);
        moveAround(playerPos);
    }
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