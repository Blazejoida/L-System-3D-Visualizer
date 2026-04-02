# L-System 3D Visualizer

A real-time interactive 3D L-system (Lindenmayer system) visualizer built with **Windows API** and **OpenGL**. Implements the official turtle graphics specification from *The Algorithmic Beauty of Plants* (Prusinkiewicz & Lindenmayer, 1990).

---

## Features

- **Built-in presets** - canonical grammars from ABoP, 3D trees, and classic fractals
- **Full 3D turtle graphics** following the ABoP specification (heading H, left L, up U vectors with Rodrigues rotation)
- **Custom grammar editor** - type your own axiom and rewriting rules in a dedicated window
- **Real-time parameter control** - iterations (slider 1–20 + free-text input with Enter key), branch angle override, custom colours
- **Correct symbol mapping** - only `F` and `G` draw segments; all other letters are non-drawing grammar nodes
- **Dynamic near/far planes** - scene never clips at high iteration counts regardless of scale
- **Windows API UI** with OpenGL child window

---

## Turtle Symbol Reference

Full 3D symbol set:

| Symbol | Action |
|--------|--------|
| `F` `G` | Move forward one step and **draw** a segment |
| `f` | Move forward one step, no segment |
| `+` | Yaw left by δ (rotate around **U** - up axis) |
| `-` | Yaw right by δ |
| `&` | Pitch down by δ (rotate around **L** - left axis) |
| `^` | Pitch up by δ |
| `\` | Roll left by δ (rotate around **H** - heading axis) |
| `/` | Roll right by δ |
| `\|` | U-turn — yaw 180° |
| `[` | Push turtle state onto stack |
| `]` | Pop turtle state from stack |

All other letters are **non-drawing grammar nodes**. The turtle skips them silently - they exist only as rewriting symbols.

---

## Custom Grammar Editor

Click **+ Custom grammar...** to open the modeless editor. It supports any deterministic context-free L-system.

### Input format

| Field | Description |
|-------|-------------|
| Axiom | Starting string, e.g. `F` or `X` |
| Rules | One rule per line, format `X=successor` |
| Angle | δ in degrees (0.5–360) |
| Scale | Step length multiplier applied at each `[` push (0.1–2.0) |

### Example grammars

**Sierpiński triangle** (edge rewriting):
```
Axiom : F-G-G
F = F-G+F+G-F
G = GG
Angle : 120    Scale : 0.5    Iter : 5–7
```
---

## Controls

| Action | Input |
|--------|-------|
| Rotate scene | Left-click drag |
| Zoom | Scroll wheel |
| Reset camera | "Reset camera" button |
| Change preset | Dropdown list |
| Set iterations | Slider (1–20) **or** type a number and press **Enter** |
| Override branch angle | Angle slider (0 = use preset value) |
| Custom colours | Enable checkbox, then click swatch buttons |
| Custom grammar | Click "+ Custom grammar..." |

---

## Requirements

- Visual Studio 2022 (MSVC v143)
- Windows 10 SDK
- **freeglut** — for `gl.h` / `glu.h` headers and `glu32.lib`

---

## Reference

> Prusinkiewicz, P. & Lindenmayer, A. (1990). *The Algorithmic Beauty of Plants*. Springer-Verlag.
> Available at [algorithmicbotany.org](https://algorithmicbotany.org/papers/#abop).
