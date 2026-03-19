# Matiks Bot 🤖

An intelligent, human-like automation bot for [Matiks](https://matiks.in) built in C++ using OOP principles. The bot automatically detects, solves, and submits math questions during live 1v1 duels — with adaptive delays, human-like typing, and anti-detection mechanisms.

---

## Demo

> Add your GIF demo here

---

## Features

- **Dynamic question region detection** — works on any Mac screen size, no hardcoded coordinates
- **Multi-mode launcher** — supports all 2 game modes on Matiks as of now, other modes coming soon
- **Auto browser control** — opens Chrome, navigates to game, switches apps automatically
- **Human-like behaviour** — adaptive delays based on question difficulty
- **Anti-detection** — occasional typos, backspaces, reaction time simulation, intentional wrong answers
- **Auto rematch/new game** — detects game end and handles next match automatically
- **OOP architecture** — clean, extensible class hierarchy for adding new game mode bots

---

## Project Structure

```
matiks-bot/
├── main.cpp               ← entry point
├── bot.cpp                ← launcher menu and game flow
├── Makefile               ← build system
├── include/
│   ├── MatiksBot.h        ← base class declaration
│   ├── MathBot.h          ← math bot declaration
│   └── MindSnapBot.h      ← mindsnap bot declaration (coming soon)
└── src/
    ├── MatiksBot.cpp      ← base class implementation
    ├── MathBot.cpp        ← math bot implementation
    └── MindSnapBot.cpp    ← mindsnap bot implementation (coming soon)
```

---

## Architecture

```
MatiksBot (base class)
├── detectQuestionRegion()
├── takeScreenshot()
├── runOCR()
├── decideNextGame()
├── runGameLoop()
├── humanReactionDelay()
├── typeAnswerHuman()
├── shouldMakeError()
├── generateWrongAnswer()
└── runBot()

MathBot extends MatiksBot
├── readOCROutput()
├── solve()
├── calculateDelay()
└── runBot()

MindSnapBot extends MatiksBot (coming soon)
└── runBot()
```

---

## How It Works

### Dynamic Question Region Detection

Instead of hardcoding screen coordinates, the bot uses two reference points to detect the question area on any Mac screen size:

```
"Cancel Search" button  →  bottom boundary + width reference
"01:04" countdown timer →  top boundary reference

Question area = region between these two elements
```

### Bot Pipeline

```
Step 1 → Take screenshot of question region
Step 2 → Scale up image 3x for better OCR accuracy
Step 3 → Increase contrast to remove grid lines
Step 4 → Run Tesseract OCR to read question
Step 5 → Parse question into lines
Step 6 → Solve math
Step 7 → Human reaction delay
Step 8 → Type answer with human-like speed + occasional typos
```

### Question Solving

Questions appear vertically:
```
  56
+40
-56
+45
```

The bot treats the first line as a plain number and each subsequent line as an operator + number, applying them sequentially.

### Difficulty-Based Adaptive Delay

The bot calculates a dynamic delay after each answer based on question difficulty — mimicking how a human naturally takes longer on harder questions.
```
base_delay = 800ms
+ (number of lines - 1) × 200ms   → more lines = more thinking time
+ (digit count per line) × 100ms  → larger numbers = more time
+ 1200ms for × questions           → multiplication is hardest
- flat 700ms for simple ÷ (2 line) → division is easiest
+ random noise (-500ms to +900ms) → human unpredictability
minimum delay = 600ms              → never answer too fast
```

---

## Anti-Detection Mechanisms

| Mechanism | Description |
|---|---|
| Adaptive delay | Harder questions take longer — ranges from 600ms to 3000ms+ based on difficulty |
| Human reaction time | 150-400ms delay before starting to type each answer |
| Character-by-character typing | Random 200-300ms between each keystroke simulating human typing speed |
| Occasional typos | ~7% chance per character of typing wrong digit → pause → backspace → retype |
| Intentional wrong answers | 5% chance of submitting a slightly wrong answer (±1 to ±15 off) before correcting |
| Random noise | -500ms to +900ms random variation on all delays |

---

## Game Modes Supported

| # | Mode | Bot Status |
|---|---|---|
| 1 | Online Duels (Math Sprint) | ✅ Working |
| 2 | Fastest Fingers Duels | ✅ Working |
| 3 | Cross Math Duels | 🔜 Coming Soon |
| 4 | Ken Ken Duels | 🔜 Coming Soon |
| 5 | Math Maze Duels | 🔜 Coming Soon |
| 6 | Flash Anzan | 🔜 Coming Soon |
| 7 | Mindsnap Online | 🔜 Coming Soon |
| 8 | Ability Duels | 🔜 Coming Soon |

---

## Requirements

- Mac (tested on MacBook with Chrome)
- C++ compiler (g++)
- [Tesseract OCR](https://tesseract-ocr.github.io/tessdoc/Installation.html)
- [ImageMagick](https://imagemagick.org)
- [cliclick](https://github.com/BlueM/cliclick)

### Install dependencies

```bash
brew install tesseract
brew install imagemagick
brew install cliclick
```

---

## Setup & Usage

### Step 1 — Clone the repository
```bash
git clone https://github.com/Hrishikesh1104/matiks-bot.git
cd matiks-bot
```

### Step 2 — Compile
```bash
make
```

### Step 3 — Run from Terminal
```bash
./bot
```

> ⚠️ Must run from native Terminal (not VS Code terminal) for accessibility permissions to work.

### Step 4 — Select game mode
```
=============================
     MATIKS BOT LAUNCHER
=============================
1. Online Duels (Math Sprint)
2. Fastest Fingers Duels
3. Cross Math Duels
...
=============================
Enter your choice (1-8):
```

### Step 5 — Let it play!
The bot will:
1. Open Chrome and navigate to the selected game mode
2. Wait for "Cancel Search" button → detect bottom boundary
3. Wait for "01:04" countdown timer → detect top boundary
4. Calculate question region automatically
5. Solve questions for the full 60 seconds
6. After match ends → switch to Terminal → ask Rematch/New Game/Exit

---

## Mac Permissions Required

Go to **System Settings → Privacy & Security** and enable:

| Permission | App |
|---|---|
| Accessibility | Terminal |
| Accessibility | Google Chrome |

---

## Built With

| Tool | Purpose |
|---|---|
| C++ | Core bot logic |
| OOP | Clean extensible architecture |
| Tesseract OCR | Reading questions from screenshots |
| ImageMagick | Image contrast enhancement |
| sips | Image scaling (built into Mac) |
| screencapture | Taking screenshots (built into Mac) |
| osascript | Keyboard simulation + app switching |
| cliclick | Mouse click simulation |

---

## Note

This bot is built for educational purposes to explore OCR, screen automation, computer vision, and C++ systems programming with OOP design patterns.