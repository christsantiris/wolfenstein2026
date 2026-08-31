# Weapon Mechanics and Balance

This document describes the weapon behavior currently implemented in `src/core/game.c`, `src/core/game.h`, `src/core/item.c`, and `src/main.c`.

The game models ammunition with a loaded magazine and a separate reserve for each weapon. Although "clip" is commonly used conversationally, the implementation uses magazine capacity.

## Current weapon statistics

| Weapon | Unlock | Magazine | Reserve | Total when full | Damage per trigger | Shot interval | Maximum fire rate | Reload | Target cone | Burst DPS |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 9mm handgun | Level 1 | 8 | 99 | 107 | 34 | 0.50 s | 2.00 shots/s | 1.5 s | 0.15 rad | 68.0 |
| Dual handguns | Level 2 | 16 | 99 | 115 | 34 | 0.25 s | 4.00 shots/s | 1.2 s | 0.15 rad | 136.0 |
| Shotgun | Level 5 | 2 | 99 | 101 | Up to 120 | 0.80 s | 1.25 shots/s | 2.0 s | 0.30 rad | Up to 150.0 |
| Battle rifle | Level 9 | 10 | 99 | 109 | 55 | 0.35 s | 2.86 shots/s | 1.8 s | 0.08 rad | 157.1 |
| AK-47 | Level 12 | 30 | 99 | 129 | 30 | 0.12 s | 8.33 shots/s | 2.0 s | 0.25 rad | 250.0 |

Burst DPS is damage per trigger divided by the shot interval. It excludes reload time and assumes every projectile hits.

## Campaign weapon pickups

| Level | Weapon pickup | Placement |
| ---: | --- | --- |
| 2 | Dual handguns | Random eligible floor tile |
| 5 | Shotgun | Random eligible floor tile |
| 7 | Rifle grenade | Random eligible floor tile |
| 9 | Battle rifle | Fixed position in the miniboss arena |
| 12 | AK-47 | Random eligible floor tile |

The 9mm handgun and knife are starting equipment rather than level pickups. Levels not listed above contain no weapon pickup.

## Shared ammunition behavior

- Every weapon has its own reserve-ammo pool.
- Every reserve can hold up to 99 rounds or shells.
- A normal ammunition pickup adds 8 rounds to the currently equipped weapon only.
- Collecting a new weapon grants a full magazine and 99 reserve rounds for that weapon.
- Reloading transfers only the ammunition needed to fill the magazine. Unused ammunition remains in reserve.
- Firing always consumes one loaded round, including the dual handguns and shotgun.

## Combat behavior

- All firearms have the same maximum range of 20 map units.
- Handguns and rifles damage the closest valid target inside the weapon's target cone.
- Damage does not decrease with distance.
- Handgun and rifle target cones are not random bullet spread. A larger value permits targets farther from the crosshair to be selected, making the weapon more forgiving to aim.
- The dual handguns do not fire two projectiles per trigger pull. Their advantage is twice the handgun's magazine capacity and twice its maximum firing rate.
- The shotgun fires six fixed pellets spread across its `0.30`-radian cone. Each pellet deals 20 damage and independently selects the nearest enemy intersecting its path.
- Enemy angular width determines how many pellets connect. A close centered target can receive all six pellets for 120 damage, while distant targets receive fewer. Different pellets can hit different enemies.
- The battle rifle is the most precise weapon, but its narrow target cone requires the player to aim closest to the target.
- The AK-47 has the highest magazine capacity, fire rate, burst damage rate, and total damage per full magazine.

## Full-magazine output

| Weapon | Damage before reload | Minimum time between first and last shot |
| --- | ---: | ---: |
| 9mm handgun | 272 | 3.50 s |
| Dual handguns | 544 | 3.75 s |
| Shotgun | Up to 240 | 0.80 s |
| Battle rifle | 550 | 3.15 s |
| AK-47 | 900 | 3.48 s |

The timing column is `(magazine size - 1) * shot interval`. It does not include player reaction time or the reload following the final shot.

## Progression assessment

Maximum close-range burst DPS now progresses from 68 for the handgun, to 136 for the dual handguns, 150 for the shotgun, 157.1 for the battle rifle, and 250 for the AK-47.

The shotgun remains intentionally situational: it can exceed the dual handguns at close range and can hit multiple clustered enemies, but its effectiveness drops with distance and its two-shell magazine forces frequent reloads. The battle rifle remains the stronger precise, sustained option at Level 9, while the AK-47 remains the strongest general-purpose weapon at Level 12.
