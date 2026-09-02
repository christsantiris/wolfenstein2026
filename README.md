# Wolfenstein 2026

A first-person shooter inspired by *Wolfenstein 3D* by id Software. Navigate mazes, hunt enemies, and survive the castle.

## Table of Contents
- [Overview](#overview)
- [Screenshots](#screenshots)
- [Building and Running](#building-and-running)
  - [Compile the Game](#compile-the-game)
  - [Run the Game](#run-the-game)
  - [Clean the Build](#clean-the-build)
  - [macOS Production Release](#macos-production-release)
  - [Linux Production Release](#linux-production-release)
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

<img width="1598" height="1206" alt="image" src="https://github.com/user-attachments/assets/c8bfb212-019f-44c2-a3e8-9f55fb0f8052" />

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

## macOS Production Release

Production releases require the `Developer ID Application: Chris Tsantiris (KVW8V4B9WS)` certificate in the login keychain. Configure notarization credentials once using an app-specific password generated at [account.apple.com](https://account.apple.com/):

```bash
xcrun notarytool store-credentials "wolfenstein2026-notary" \
  --apple-id "chris.tsantiris@gmail.com" \
  --team-id "KVW8V4B9WS"
```

Create the signed Universal 2 release for Apple silicon and Intel Macs:

```bash
make production
```

This creates `wolfenstein2026.dmg` in the project root. Every newly generated DMG must be submitted to Apple and receive its own notarization ticket:

```bash
xcrun notarytool submit wolfenstein2026.dmg \
  --keychain-profile "wolfenstein2026-notary" \
  --wait
```

After the submission reports `status: Accepted`, staple and validate the ticket:

```bash
xcrun stapler staple wolfenstein2026.dmg
xcrun stapler validate wolfenstein2026.dmg
```

Mount the DMG by opening it in Finder, then verify the app with Gatekeeper:

```bash
spctl --assess --type execute --verbose=4 \
  "/Volumes/Wolfenstein 2026/Wolfenstein 2026.app"
```

The expected result is `accepted` with `source=Notarized Developer ID`. Do not rebuild or modify the DMG after notarization; doing so requires another submission and ticket.

## Linux Production Release

Install and start Docker Desktop, then create the portable x86_64 AppImage:

```bash
make production-linux
```

The release files are written to `dist/Wolfenstein2026-1.0.0-x86_64.AppImage` and `dist/SHA256SUMS`. The AppImage includes the game assets and required SDL libraries and targets glibc-based desktop distributions on x86_64 systems.

Copy both files from `dist/` to an x86_64 Linux computer. From the directory containing the downloaded files, verify and launch the release with:

```bash
sha256sum --check SHA256SUMS
chmod +x Wolfenstein2026-1.0.0-x86_64.AppImage
./Wolfenstein2026-1.0.0-x86_64.AppImage
```

The executable permission only needs to be enabled once. Afterward, most Linux file managers allow the AppImage to be launched by double-clicking it. If double-clicking does not launch it, open the file's **Properties → Permissions**, enable **Allow executing file as program**, and try again. AppImages run without installation and do not automatically add an application-menu entry.

The Linux AppImage cannot run directly on macOS or Windows. It must be tested on a Linux installation or a Linux virtual machine with a graphical desktop.

An ARM64 AppImage can be built on an ARM64 Docker host with:

```bash
make production-linux-arm64
```

Do not run the ARM64 target on an Intel Mac: Docker Desktop must emulate the entire ARM64 build and its QEMU emulator can fail while configuring system libraries.

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

## Roadmap
- Additional levels

## Sound Effects courtesy of
https://pixabay.com
