# Computer Graphics Project 2024

<img src="https://github.com/user-attachments/assets/68168cf8-7df4-49ff-a80e-105b61fff777" width=350 px align="right" />

_Purrfect Potion_ is the final project of "**Computer Graphics**" course held at Politecnico di Milano (2023/2024).

**Score**: 30/30 cum laude 

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
  - Possibility to switch on/off different lights
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

Simple textures have been hand-painted for some models. More complex PBR materials textures have been downloaded freely.

## Media

### Main Menu

https://github.com/user-attachments/assets/0b551886-66d0-4f7e-b0c8-ee357e95a682

### Game

https://github.com/user-attachments/assets/415b90f8-a898-4f02-8053-2f1404b490f6

### World

![World](https://github.com/user-attachments/assets/70fddfa9-0064-49d3-aacc-fe69ea2d665f)

### Shaders

![Ward Shader](https://github.com/user-attachments/assets/dd4e9641-1404-4148-9fa1-afaedb297949)

https://github.com/user-attachments/assets/ae598e23-5d2f-410d-9e21-a4819e8a7306

https://github.com/user-attachments/assets/7b5d7559-4fdf-49d9-b86a-0bc42e317383

### Bounding Boxes

![Bounding Boxes](https://github.com/user-attachments/assets/5faf86fe-9cce-4486-b3d7-7116ed93de9e)
