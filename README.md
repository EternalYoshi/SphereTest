# SphereTest
An experimental piece of code to reveal hitspheres for the one true game, Ultimate Marvel vs Capcom 3.

A chunk of the code is borrowed from ermaccer's UMVC3Hook and other bits are from HKHaan's repositories.

WIP.
I was referencing the Vita version's display while making this and had to make one big change. The state best described as "Not Hittable, but throwable", is displayed as blue instead of green like other hittable states.

# Currently known issues.
- Shot files, aka Projectiles, have too many moving parts; some shots may display collisions despite not being active at the frames they do.
- Some of Strider's collisions incorrectly display when they aern't active because he's an anomoly when it comes to character states, and Vajra doesn't display.
- Some child characters don't display collisions properly.
- The displays don't show up properly in captures and streams; you'll have to screenshot or stream your whole screen for best results.

# Hotkeys
- If you want to change the hotkeys, you can open the .ini and change the values inside. They use VK values, and you can find a list here:
https://cherrytree.at/misc/vk.htm

# Installation
The .asi and .ini included in the releases goes in the same directory as the game exe. You will also need the Ultimate ASI Loader's dinput8.dll, but if you have Clone Engine, Community Edition, or Color Expansion installed, you already have the needed dinput8.dll.
By default, the hotkeys to toggle the display and change opacity are the Numpad PGDN, Numpad +, and Numpad - keys. You can change these in the ini using VK, or Virtual Key code values.

Thanks to Gneiss, anotak, HKHaan for finding many of the pointers and locations that made getting to this point possible, and Jason & n3 for help and other misc things.

~ Eternal Yoshi
