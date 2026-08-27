.PHONY: all run clean debug debug-pistol debug-dual-handguns debug-shotgun debug-battle-rifle debug-ak47 debug-knife debug-level test linux sprites

all:
	cmake -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build

run: all
	./build/wolf

dmg:
	cmake -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build
	bash package/macos/build_dmg.sh

linux:
	bash package/linux/build_linux.sh

debug: debug-shotgun

debug-pistol:
	cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DDEBUG_NO_AMMO=OFF -DDEBUG_SHOTGUN=OFF -DDEBUG_AK47=OFF -DDEBUG_START_LEVEL=0 -DDEBUG_START_WEAPON=GUN_9MM_HANDGUN
	cmake --build build-debug
	./build-debug/wolf

debug-dual-handguns:
	cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DDEBUG_NO_AMMO=OFF -DDEBUG_SHOTGUN=OFF -DDEBUG_AK47=OFF -DDEBUG_START_LEVEL=0 -DDEBUG_START_WEAPON=GUN_DUAL_HANDGUN
	cmake --build build-debug
	./build-debug/wolf

debug-shotgun:
	cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DDEBUG_NO_AMMO=OFF -DDEBUG_SHOTGUN=OFF -DDEBUG_AK47=OFF -DDEBUG_START_LEVEL=0 -DDEBUG_START_WEAPON=GUN_SHOTGUN
	cmake --build build-debug
	./build-debug/wolf

debug-battle-rifle:
	cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DDEBUG_NO_AMMO=OFF -DDEBUG_SHOTGUN=OFF -DDEBUG_AK47=OFF -DDEBUG_START_LEVEL=0 -DDEBUG_START_WEAPON=GUN_BATTLE_RIFLE
	cmake --build build-debug
	./build-debug/wolf

debug-ak47:
	cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DDEBUG_NO_AMMO=OFF -DDEBUG_SHOTGUN=OFF -DDEBUG_AK47=OFF -DDEBUG_START_LEVEL=0 -DDEBUG_START_WEAPON=GUN_AK47
	cmake --build build-debug
	./build-debug/wolf

debug-knife:
	cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DDEBUG_NO_AMMO=OFF -DDEBUG_SHOTGUN=OFF -DDEBUG_AK47=OFF -DDEBUG_START_LEVEL=0 -DDEBUG_START_WEAPON=GUN_KNIFE
	cmake --build build-debug
	./build-debug/wolf

debug-level:
	cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DDEBUG_START_LEVEL=2 -DDEBUG_START_WEAPON=
	cmake --build build-debug
	./build-debug/wolf

sprites:
	mkdir -p assets/sprites
	cc tools/gen_pistol.c -o /tmp/wolf_gen_pistol && /tmp/wolf_gen_pistol && rm /tmp/wolf_gen_pistol
	cc tools/gen_shotgun.c -o /tmp/wolf_gen_shotgun && /tmp/wolf_gen_shotgun && rm /tmp/wolf_gen_shotgun
	cc tools/gen_ak47.c -o /tmp/wolf_gen_ak47 && /tmp/wolf_gen_ak47 && rm /tmp/wolf_gen_ak47

clean:
	rm -rf build build-debug
