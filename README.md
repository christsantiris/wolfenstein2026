# Wolfenstein 2026

A first-person shooter inspired by *Wolfenstein 3D* by id Software. Navigate mazes, hunt enemies, and survive the castle.

## Table of Contents
- [Overview](#overview)
- [Screenshots](#screenshots)
- [Building and Running](#building-and-running)
  - [Compile the Game](#compile-the-game)
  - [Run the Game](#run-the-game)
  - [Clean the Build](#clean-the-build)
- [Dependencies](#dependencies)
- [Contributing](#contributing)
- [License](#license)
- [Roadmap](#roadmap)

## Overview
*Wolfenstein 2026* is a raycasting first-person shooter built in C with SDL2. Inspired by the 1992 id Software classic, it features a fast grid-based raycasting engine, textured walls, and enemy AI — faithful to the original in feel while running natively on modern hardware. This game is for everyone who gaming left behind in 1999. You don't have to be a gamer to play this game. It is fun and easy to play. 

## Screenshots
<img width="797" height="627" alt="Screenshot 2026-08-26 at 9 59 56 PM" src="https://github.com/user-attachments/assets/583dba77-bec5-4cb4-be1a-7bbfd9728696" />

<img width="797" height="627" alt="image" src="https://github.com/user-attachments/assets/3bc83ec6-2cdf-4857-a19a-4566fecf26b5" />

<img width="797" height="627" alt="image" src="https://github.com/user-attachments/assets/22d17ca8-eac0-43b6-87d3-b995804aa1af" />

<img width="797" height="627" alt="image" src="https://github.com/user-attachments/assets/d1e7f87b-e671-48a4-be15-97a350c54c7c" />

<img width="797" height="627" alt="image" src="https://github.com/user-attachments/assets/3a01b0ca-f311-4dc4-957f-ca0f467c5522" />

<img width="797" height="627" alt="image" src="https://github.com/user-attachments/assets/08d364d7-11ec-40a1-8673-bce074ba79e0" />

<img width="1602" height="1198" alt="image" src="https://github.com/user-attachments/assets/7deb1d5f-2593-4e39-aca8-acb1a447ddee" />

## Building and Running

## Compile the Game
To compile and launch the game run `cmake --build build` in the root directory.

## Run the Game
```
make run
```

## Clean the Build
Run `make clean` to remove the compiled build and start fresh.

## Dependencies
- `cmake`
- `sdl2`
- `sdl2_mixer`

Install on macOS via Homebrew:
```
brew install cmake sdl2 sdl2_mixer
```

Install on Linux (Debian/Ubuntu):
```
sudo apt install cmake libsdl2-dev libsdl2-mixer-dev pkg-config
```

## Contributing
Pull requests are welcome. Please keep changes small and focused — one feature per PR.

## License
MIT

## Sound Effects courtesy of
https://pixabay.com

## Roadmap
- Additional levels
- Mini bosses
- Additional weapons (grenades?)
- Better weapon/enemy power scaling with selected difficulty

