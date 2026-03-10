# 🐍 Snake Real-Time Game (C / Turbo C)

Final project for a **Real-Time Systems course**.

This project implements a classic **Snake game** developed in **C** and designed to run in the **Turbo C DOS environment**.
The game uses **hardware interrupts**, **direct video memory access**, and **keyboard interrupt handling** to simulate real-time gameplay.

---

## 🎮 Game Description

The player controls a snake that must:

* Avoid colliding with walls
* Avoid leaving the screen
* Avoid colliding with itself
* Eat apples to grow and gain points

The game ends when the snake crashes or when the player **wins by reaching 12 points**.

---

## 🧠 Real-Time Concepts Used

The project demonstrates several low-level programming concepts:

* Timer interrupt handling (`INT 8`)
* Keyboard interrupt handling (`INT 9`)
* Direct video memory access (`0xB800`)
* Real-time scheduling for movement and rendering
* Dynamic speed changes based on game progress

---

## 🕹 Game Features

* 4 difficulty levels
* Each level has a **different background color**
* Game speed increases as the player progresses
* Apple collection increases score and snake length
* Visual obstacles (arrow-shaped walls)
* Sound feedback when eating apples
* Bonus time for each apple

---

## 🧩 Game Levels

| Level | Background | Difficulty |
| ----- | ---------- | ---------- |
| 1     | Blue       | Easy       |
| 2     | Brown      | Medium     |
| 3     | Green      | Hard       |
| 4     | Black      | Very Hard  |

The final level is intentionally difficult since the snake colors (red/black) blend with the dark background.

---

## 🏆 Win Condition

The player wins when reaching:

```
Score = 12
```

A **green victory screen** appears with the final score.

---

## ❌ Game Over Conditions

The game ends if:

* The snake leaves the screen
* The snake hits a wall
* The snake collides with itself
* The timer runs out

---

## ⌨ Controls

| Key | Action     |
| --- | ---------- |
| ↑   | Move Up    |
| ↓   | Move Down  |
| ←   | Move Left  |
| →   | Move Right |
| ESC | Exit Game  |

---

## 🛠 Technologies

* C Programming Language
* Turbo C
* DOS Environment
* Hardware Interrupts
* Direct Memory Access

---

## 📸 Screenshots(in the folder)

* Level 1 – Blue Background
* Level 2 – Brown Background
* Level 3 – Green Background
* Level 4 – Black Challenge Level
* Game Over Screen
* Victory Screen

---

## 🚀 How to Run

1. Open **Turbo C**
2. Compile the source file

```
snake.c
```

3. Run the program

```
Ctrl + F9
```

---

## 👨‍💻 Author

Final project developed as part of a **Real-Time Systems course**.
