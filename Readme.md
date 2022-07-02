# My first chess engine
Hello! This is my first ever chess engine ~~(well, almost first)~~. A chess engine is essentially a computer program that plays chess. The engine is far from being good by chess engine standards, but was fun to make and is fun to play against. It isn't very fast and doesn't understand any strategy, but I lost to it quite a few times in the most embarrassing ways.

<p align="center">
  <img height=30% width=30% src="https://lichess1.org/game/export/gif/FEBjVCsC.gif">
</p>

It adheres to the XBoard protocol. Not all the commands are implemented and not all functionality is fully supported, but it's enough to play a game against it in WinBoard or launch analysis mode. It's also possible to play against it from the command line.

# Running

## Building
The project uses CMake. To get an executable, you want to build the `Engine` target. See [here](https://cmake.org/runningcmake/) on how to build CMake projects. [Here](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) you can find what types of makefiles/projects CMake supports.

## Playing using WinBoard
WinBoard is a chess program, that has a chess GUI and can work with chess engines. To play against the engine, launch WinBoard, choose `Engine -> Load First Engine` and specify the path to the executable. Now just make a move to play as white. Choose `Mode -> Machine White` to play as black. Loading the engine once will save it in the engine list, and you will be able to choose it on the WinBoard startup. WinBoard is a Windows analogue of XBoard, but I have only tested the engine on Windows.

## Playing from the command line
To play against the engine from a command line, just launch the executable. To play as white, start making moves using the `usermove` command. To play as black use the `go` command to make the machine make the first move for white, and then just make moves for black using `usermove`.

A move is specified by the starting and destination squares, for example `e2e4`. To promote pawns specify piece code (`q`, `b`, `n` or `r`) at the end in the lower case, for example `e7e8q`. To castle move king over 2 squares, for example `e1g1`.

To start a new game, use the `new` command.

## Time control
The default time control is 10 minutes per 40 moves. The engine also understands increment. If you give the engine time for the entirety of the game, the engine will ignore it, so don't do that. In the case of a fixed amount of time without any increment, the engine will just think it has 0 seconds per move, which is not good. Time control can be changed both in WinBoard or by using the `level` command.

# How it works
The engine does an exhaustive search to a certain depth, using [alpha-beta pruning](https://www.chessprogramming.org/Alpha-Beta), [transposition table](https://www.chessprogramming.org/Transposition_Table) and some heuristics, like trying out checks and captures first or trying out moves that caused beta-cutoff in other branches. The engine also uses a simple [quiescence search](https://www.chessprogramming.org/Quiescence_Search).

Evaluation function is rather primitive and very materialistic for now. There are also some bonuses for board control and king safety, but their weight is very low, because with the current implementation that would lead to some undesired behavior. These bonuses allow the engine to develop pieces at the start of the game and find forced mates up material.

# Example games

I have uploaded some of the interesting games to lichess. 

## Checkmating

The engine can handle rook and 2 bishops checkmates rather easily. Bishop + knight checkmate is a bit too hard for my engine right now.
<details>
  <summary>rook vs king</summary>
  <img height=30% width=30% src="https://lichess1.org/game/export/gif/ho8rs4GG.gif"><br>
  <a href="https://lichess.org/ho8rs4GG/white">game link</a>
</details>

<details>
  <summary>bishops vs king</summary>
  <img height=30% width=30% src="https://lichess1.org/game/export/gif/BulgjTOO.gif"><br>
  <a href="https://lichess.org/BulgjTOO">game link</a>
</details>

## Full games

Here are 2 games between me and my engine, that show some of it's weaknesses and strengths.
<details>
  <summary>me losing to the engine</summary>
  <img height=30% width=30% src="https://lichess1.org/game/export/gif/FEBjVCsC.gif"><br>
  <a href="https://lichess.org/FEBjVCsC">game link</a>
</details>
<details>
<summary>engine losing to me</summary>
  <img height=30% width=30% src="https://lichess1.org/game/export/gif/black/BtsLNbGl.gif"><br>
  <a href="https://lichess.org/BtsLNbGl/black">game link</a>
</details>
