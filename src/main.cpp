#include "PiskvorkLinker.hpp"

int main()
{
    PiskvorkLinker linker;

    while (linker.nextTurn()) {
        linker.playTurn();
    }
}