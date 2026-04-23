# Wolfenstein 2026

A first-person shooter inspired by *Wolfenstein 3D* by id Software. Navigate mazes, hunt enemies, and survive the castle.

## Table of Contents
- [Overview](#overview)
- [Screenshot](#screenshot)
- [Building and Running](#building-and-running)
  - [Compile the Game](#compile-the-game)
  - [Run the Game](#run-the-game)
  - [Clean the Build](#clean-the-build)
- [Dependencies](#dependencies)
- [Contributing](#contributing)
- [License](#license)
- [Roadmap](#roadmap)

## Overview
*Wolfenstein 2026* is a raycasting first-person shooter built in C with SDL2. Inspired by the 1992 id Software classic, it features a fast grid-based raycasting engine, textured walls, and enemy AI — faithful to the original in feel while running natively on modern hardware.

## Screenshot
*Screenshots coming soon.*

## Compile the Game
To compile and launch the game run `make run` in the root directory.

## Run the Game
```
make run
```

## Clean the Build
Run `make clean` to remove the compiled build and start fresh.

## Dependencies
- `cmake`
- `sdl2`

Install on macOS via Homebrew:
```
brew install cmake sdl2
```

Install on Linux (Debian/Ubuntu):
```
sudo apt install cmake libsdl2-dev pkg-config
```

## Contributing
Pull requests are welcome. Please keep changes small and focused — one feature per PR.

## License
MIT

## Roadmap
- Sound — SDL2_mixer, gunshot, reload click, enemy alert bark
- Level exit + multiple levels — exit tile triggers next map load
- Wall texture variants — map cells 1-9 already route through, just need different textures per value
