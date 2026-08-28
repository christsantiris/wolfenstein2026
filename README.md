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
<img width="1808" height="968" alt="image" src="https://github.com/user-attachments/assets/7051036f-b3b6-47cf-9d8c-3bc92a2c3cb4" />

<img width="3836" height="2046" alt="image" src="https://github.com/user-attachments/assets/2ff43031-d01d-4a09-a940-0516335750eb" />

<img width="3838" height="2040" alt="image" src="https://github.com/user-attachments/assets/1b866d45-eeb1-4a88-a9e1-12ee687de128" />

<img width="3832" height="2042" alt="image" src="https://github.com/user-attachments/assets/c2bc6051-26dd-4e26-a43b-47dafddddbd2" />

<img width="3838" height="2036" alt="image" src="https://github.com/user-attachments/assets/c19d3fd6-2720-4b86-a105-c2cde09f59c5" />

<img width="3842" height="2052" alt="image" src="https://github.com/user-attachments/assets/dbb94491-e350-4f6b-aa60-549e17529613" />

<img width="3838" height="2052" alt="image" src="https://github.com/user-attachments/assets/5aac2d96-650b-4d12-920f-a894fd01456f" />

<img width="3836" height="2044" alt="image" src="https://github.com/user-attachments/assets/044e41a9-833e-4162-b575-d48651ebd505" />

<img width="3836" height="2056" alt="image" src="https://github.com/user-attachments/assets/abbc5b72-5212-402c-ba95-3128afc8cc4d" />

<img width="3844" height="2056" alt="image" src="https://github.com/user-attachments/assets/d5b757f6-935d-425a-84d8-3405a5816227" />

<img width="3824" height="2048" alt="image" src="https://github.com/user-attachments/assets/0248e1c2-b32d-4272-82eb-f13f867950d7" />

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

