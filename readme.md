# Operating Systems -- Connect 4

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

## Overview
This project implements the game "Four in a Row" and offers two different modes of play:

- 1vs1: Two users play against each other.
- 1vsPC (Automatic Game): One user can play against the "PC."


## Development Team
- Aldegheri Alessandro
- Venturi Davide
- Zerman Nicolò

This project was developed as the final project for our Operating Systems class at University of Verona

## Technology Stack
- C

Everything is written in standard C. All mechanisms of Inter-Process-Communication are implemented with System V System Calls (Semaphores, Shared Memories, Signals). 

## Server

The server's role is to arbitrate the entire game. In addition to managing the alternating turns and signaling any outcome, it allows users to continue playing by arbitrating all the relevant matches (one at a time).

Before clients can play, the server must be executed as it is responsible for initializing the various Inter-Process Communications (IPCs) that will also be used by the clients. If clients are executed without a server capable of hosting the game, they will be terminated. Furthermore, clients are not allowed access to the server if it is already handling another game.

Possible actions on the server:

- CTRL+C: On the first press, a warning message will be shown. On the second press, the server will notify the clients of the termination of the game session to terminate itself as well.
- Terminal Closure: Behaves the same as the second press of CTRL+C.

In any case, when the server terminates, all the used IPCs will be eliminated so that they are not left allocated within the operating system.

To start the server, you need to specify the size of the game field (rows and columns), as well as the symbols that the two players will use (which must be different).

The command to execute is as follows:
   ```shell
   ./F4Server <Rows> <Columns> <Symbol1> <Symbol2>
   ./F4Server 5 5 X O
   ```

Once the server is initialized, before allowing full game management, you will be prompted to enter a timer. This timer will be the time limit (in seconds) that each player will have to make their move. If you do not want to have a time limit, simply enter the value '0'.

At this point, the server will be ready to handle client access for playing the game and the subsequent matches.

## Client

To initialize the client, execute the following command:
The command to execute is as follows:
   ```shell
   ./F4Client <name>
   ./F4Client user
   ```

As mentioned above, if the client cannot find a server ready to host the game,
it will end.
If the client is the only one in the game room, it will wait for another client to log in to
play a game.
After each opponent's move, the game board will be shown and the user will have to enter the
value of the column, within which he wants to make the move. The column will be required
until a correct value is entered or until the timer (set on the server) expires.
If the timer expires without the user having made a move, victory will be awarded to
table to the opponent.
If you want to play in the 'Autoplay' mode, the command to execute will be the following:
   ```shell
   ./F4Client <name> \*
   ./F4Client user \*
   ```
In this case it will be necessary to find a totally free server since it is like itself
playing for two from a single client.
At the end of each game the clients will be asked if they want to play another game. In positive case
they will have to write 'y' or 'Y' and if not 'n' or 'N'.
Possible actions on the client:
- CTRL+C: equivalent to desktop withdrawal, inform the server of its withdrawal and finish.
- Terminal Closure: same behavior as CTRL+C.

## Compile Instructions
Once the compressed folder has been downloaded and extracted, open a terminal and issue the command: 
   ```shell
   make
   ```

Or if you would like to issue one command at a time you can issue these commands:
   ```shell
   gcc F4Server.c functions.c -o F4Server -Wall
   gcc F4Client.c functions.c -o F4Client -Wall
   ```


