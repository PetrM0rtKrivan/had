# had
terminal snake for my little son.:)

build: 
require ncurses

build command: g++ had.cpp -o had -std=c++11 -lncurses

run: ./had <height> <width> <window xbeg> <window ybeg>
eg : ./had 20 30 20 20

notes:
There is no checks for bounds and similar. It was quick action. :) New version will bring better code and adventure mode. (Yeah, still in terminal. :D )
