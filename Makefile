.PHONY: all run clean debug test linux sprites

all:
	cmake -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build

run: all
	./build/wolf

dmg:
	cmake --build build --config Release
	bash package/macos/build_dmg.sh

linux:
	bash package/linux/build_linux.sh

debug:
	cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DDEBUG_NO_AMMO=OFF -DDEBUG_SHOTGUN=ON
	cmake --build build-debug
	./build-debug/wolf

debug-level:
	cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DDEBUG_START_LEVEL=8
	cmake --build build-debug
	./build-debug/wolf

test:
	cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build build-debug --target test_runner
	./build-debug/test_runner

sprites:
	mkdir -p assets/sprites
	cc tools/gen_guard.c -o /tmp/wolf_gen_guard && /tmp/wolf_gen_guard && rm /tmp/wolf_gen_guard
	cc tools/gen_pistol.c -o /tmp/wolf_gen_pistol && /tmp/wolf_gen_pistol && rm /tmp/wolf_gen_pistol
	cc tools/gen_shotgun.c -o /tmp/wolf_gen_shotgun && /tmp/wolf_gen_shotgun && rm /tmp/wolf_gen_shotgun
	cc tools/gen_ak47.c -o /tmp/wolf_gen_ak47 && /tmp/wolf_gen_ak47 && rm /tmp/wolf_gen_ak47

clean:
	rm -rf build build-debug