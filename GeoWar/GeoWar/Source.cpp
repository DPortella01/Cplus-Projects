////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//  Assignment:     Geo Wars  
//  Instructor:     David Burchill
//  Year / Term:    Fall 2023
//  File name:      Source.cpp
// 
//  Student name:   Diego Portella
//  Student email:  portella.sdmf@hotmail.com
// 
//     I certify that this work is my work only, any work copied from Stack Overflow, textbooks, 
//     or elsewhere is properly cited. 
// 
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  BUG 
//  list any and all bugs in your code 
//  1. I didnt get abc to work because of xyz, I tried ijk...
//

 
#include <iostream>

#include "Game.h"

#include "Utilities.h"
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

int main() {

    Game game("../config.txt");
    game.run();
    return 0;
}