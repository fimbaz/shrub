# shrub 
![Shrub](https://github.com/fimbaz/shrub/raw/master/shrub2.gif)
### shrub is a TUI pet plant simulator currently featuring
- big scrolly worlds
- water
- wind
- dirt
- plants
- pollen
- seeds
- genes
- reproduction 
- death
- decay

Controls:
- hjkl <- move cursor
- HJKL <- jump cursor
- s    <- plant seed @ cursor
- d    <- cut plant
- 0..6 <- resource view

Interesting things:
- Male flowers are pink
- female flowers are purple
- pollen is '.'
- new seeds are usually '&'


Known limitations:
- no saving
- friggin C
- limited UI
- may segfault with certain libc
- the world is a dense data structure, so everything is n^2 with worldsize

Helpful hints:
- Shrub was designed for xterm-256color and requires ncurses-dev to build

Installation guide:
 1. observe common params in game.c
 2. make
 3. ./shrub
 

