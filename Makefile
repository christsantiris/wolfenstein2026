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
	cmake -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build
	./build/wolf

test:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build --target test_runner
	./build/test_runner

sprites:
	mkdir -p assets/sprites
	cc tools/gen_guard.c -o /tmp/wolf_gen_guard && /tmp/wolf_gen_guard && rm /tmp/wolf_gen_guard
	cc tools/gen_pistol.c -o /tmp/wolf_gen_pistol && /tmp/wolf_gen_pistol && rm /tmp/wolf_gen_pistol

clean:
	rm -rf build