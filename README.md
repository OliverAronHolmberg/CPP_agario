# Agario

This project is a C++ implementation of the game Agar.io. It follows the standard mechanics where a player controls a cell in a 2D arena, consuming smaller objects and other players to increase in size. The goal was to recreate the core gameplay, moving, splitting, and consuming mass, within a native desktop application. It focuses on the basic logic of the game, including collision detection and player growth, while aiming for a responsive feel through C++.

---


<img width="1072" height="713" alt="Screenshot 2026-03-28 182857" src="https://github.com/user-attachments/assets/2e52c4ed-6478-40d0-a1ea-139c5c23da0c" />

## Features
- Groth Mechanics: Consuming pellets and smaller players increase mass.
- Movement: Cells move depending on their current maxx.
- Splitting: Ability to split the cell into multiple pieces.
- Collision detection: Efficient handling of interactions between players and objects
- Enemy AI: Smart Enemy AI to create fun gameplay.

---

## Dependencies
To build and run this project, you will need:
- Raylib
- C++ compiler

---

## Controls
- Mouse: Move the cursor to change your cell's direction
 <img width="1072" height="713" alt="arrow" src="https://github.com/user-attachments/assets/2aab893d-c373-4e64-a6fc-72a720d1cae4" />

- Space: Split your cell into halves to jump forward or eat smaller players
<img width="1072" height="713" alt="Screenshot 2026-03-28 182857" src="https://github.com/user-attachments/assets/cd38aa63-c498-499f-b89b-d0b339b17c43" />


 
- W: Eject small parts of your mass to bait enemies or shrink
<img width="1072" height="713" alt="Screenshot 2026-03-28 182408" src="https://github.com/user-attachments/assets/529456a2-0e23-4a29-8389-523277e30462" />


---

## Building and running
1. Clone the repository
```bash
git clone https://github.com/OliverAronHolmberg/CPP_agario/edit/main/README.md
cd AGARIO
```
2. Build the project
```bash
make
```
3. Run the application
```bash
./main
```
---
## Licence

This project is free to use. You can modify, distribute, or do whatever you like with the code.
