# Computer Graphics Project 2024

<img src="github/assets/logo.png" width=500 px align="right" />

_Purrfect Potion_ is the final project of "**Computer Graphics**" course held at Politecnico di Milano (2023/2024).

**Professor**: Gribaudo Marco

**Group Members**:

- [Giandomenico Roberto](https://github.com/robertogiandomenico)
- [Grimaldi Francesca](https://github.com/FrancescaGrimaldi)

## Overview

### Game plot

The idea of the game is to collect all the ingredients spread through the map to brew a potion in a limited amount of time.
The player impersonates a ghost cat that needs to get his physical body back.
The game is over when the time is up or when the player has collected all the ingredients and taken them to the cauldron.

### Technical details

The project is developed in C++ using the **Vulkan** API to write low level operations that handle the creation and transformation of meshes in a 3D environment.

In particular, the course topics are:
- Basic and advanced transformations
- 3D Projections
- Basic vertex and fragment shaders
- Lighting and shading models
- Texture mapping, normals and projection
- Rendering pipeline, Vulkan and software architectures for 3D graphics

The project goal is to put all of these concepts together to build an interactive 3D application.

## Features

Beyond what is strictly needed for a Vulkan application, here are some of the feature of the developed application.

- **Game logic**			
  - Possibility to play in third or first person
  - Collectible objects randomly placed in the map
  - Simple timer
  - Interactive menus

- **Rendering**
  - Possibility switch on/off different lights
  - Meshes for the bounding boxes that may be displayed as wireframes
  - Partially transparent objects
  - Dynamic HUD

- **Shaders**
  - Use of different maps to increase realism
  - Steam and fire animation in the vertex shader
  - Ghost cat floating animation in the vertex shader
  - Hue rotation of skybox in the fragment shader

## Assets

All the 3D models used in the project have been created by the group members using [_Blender_](https://www.blender.org).

Simple texture have been hand-painted for some models. More complex PBR materials texture have been downloaded freely.

## Media

### Main Menu

![Main Menu](https://github.com/robertogiandomenico/computer_graphics_project_2024/blob/master/github/assets/menu.mp4)
### Game

![Gameplay](https://github.com/robertogiandomenico/computer_graphics_project_2024/blob/master/github/assets/gameplay.mp4)

### World

![World](github/assets/world.jpg)

### Shaders

![Ward Shader](github/assets/ward.jpg)

![SkyBox Hue Rotation](https://github.com/robertogiandomenico/computer_graphics_project_2024/blob/master/github/assets/skybox.mp4)

![Steam and Fire animation](https://github.com/robertogiandomenico/computer_graphics_project_2024/blob/master/github/assets/steam%26fire.mp4)



### Bounding Boxes

![Bounding Boxes](github/assets/boundingbox.jpg)

