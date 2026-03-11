# HW2

Jason Yeung

115780329

jason.yeung.1@stonybrook.edu

## Overview

This program implements an interactive OpenGL simulation using GLFW.  
Users can create bouncing balls or animated faces inside the window using mouse clicks.

The program supports:

- Ball mode with configurable radius and velocity
- Face mode with animated facial features
- Collision detection between objects
- Recursive face generation after collisions
- Bonus visual effects such as rotating faces and angry face reactions

Configuration values such as radius and velocity are dynamically read from `etc/config.txt`.

## Notes

- All README files for future homework should also comply with the same format as this one. 
- This program template is just for your reference. Please feel free to code your own program (i.e., not using this template). However, the user interface (mouse and keyboard functionalities) should be the same as specified in the homework manual. 
- Please **comply with the submission requirements as detailed on the [TA Help Page](https://www3.cs.stonybrook.edu/~xihan1/courses/cse328/ta_help_page.html)**. Plesase rename the directory as instructed by the TA Help Page, and submit via Brightspace. 
- Please also make sure you have checked all implemented features with "x"s in the Markdown table below. As speficied on the TA Help Page, only checked features will be considered for grading!

## Hints on The Template

- Suggested order to read and understand this program: 
  - GLFW callbacks;
  - Triangle (ignore the code related to the self-spin effect);
  - Triangle (with self-spin; involves transformation matrices);
  - Circles (involves tessellation shaders, which are not necessary in the first half of this course). 
- In this program, the circle parameters are passed into tessellation shaders via generic vertex attribute arrays. 
  Note how this differs from the "pass-by-shader-uniforms" method for the sphere example; 
- Please do remember to play with the program as guided by the comments in the tessellation evaluation shader;
- If this program does not work on your VMWare virtual environment, 
  please try to [disable the 3D acceleration feature](https://kb.vmware.com/s/article/59146). 

## Dependencies

- OpenGL:
```bash
sudo add-apt-repository ppa:kisak/kisak-mesa
sudo apt update
sudo apt-get dist-upgrade
sudo reboot
```
- [GLAD](https://glad.dav1d.de/)
  - Configuration w.r.t. results of `sudo glxinfo | grep "OpenGL`
  - Command `glxinfo` needs `mesa-utils`
- Remaining dependencies could be installed via apt:
```bash
apt install libopencv-dev libglm-dev libglew-dev libglfw3-dev mesa-utils libx11-dev libxi-dev libxrandr-dev
```

## Compile & Run

- Run inside this directory, i.e., `hw1/`: 
```bash
mkdir build
cd build
cmake -DMAKE_BUILD_TYPE=Release ..
make 
cd ..
./build/hw2
```

## Features Implemented

Check all features implemented with "x" in "[ ]"s. 
Features or parts left unchecked here won't be graded! 

- [x] 1. Bouncing Ball
  - [x] Creation
  - [x] Dynamically reading config file
  - [x] Movement
  - [x] Collison detection
- [x] 2. 4+ Bouncing Balls
- [x] 3. Bouncing Face
  - [x] Creation
  - [x] Dynamically reading config file
  - [x] Movement
  - [x] Collison detection
  - [x] Generation Evolution
- [x] 4. More Bouncing Faces
  - [x] 8+ bouncing faces
  - [x] 16+ bonucing faces
- [x] 5. BONUS
  - [x] Face rotation
  - [x] Eye orbit animation
  - [x] Recursive inner-face rendering for higher generations
  - [x] Angry face reaction when overlapping face creation is attempted
  - [x] Color transition (face turns red and fades back over time)

## Usage

Keyboard controls:

- **1** → Switch to BALL mode
- **3** → Switch to FACE mode
- **A** → Toggle animation on/off

Mouse controls:

- **Left click** → Create a ball or face depending on the current mode

Behavior:

- Balls bounce off the window boundaries and collide with each other.
- Faces bounce off boundaries and collide with other faces.
- Face collisions increase the generation number, causing recursive faces to appear inside the eyes.
- If a face creation overlaps an existing face, the creation is rejected and the existing face becomes "angry" (turns red temporarily).

## Appendix

### Config File Format

The file `etc/config.txt` contains three values: radius velocity_x velocity_y
Example: 50 120 80

Meaning:
- radius = 50 pixels
- velocity = (120, 80)

These values are dynamically loaded each time an object is created.