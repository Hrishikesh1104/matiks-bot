# Matiks Math Sprint Bot 🤖

An automated bot for the [Matiks](https://matiks.in) Math Sprint mode that automatically detects, solves, and types answers to math questions in real time during a 1v1 duel.

---

## How It Works

The bot follows a 4-step pipeline every iteration:

```
Step 1 → Take screenshot of question region
Step 2 → Run OCR to read the question
Step 3 → Solve the math
Step 4 → Type the answer
```

Before the game starts, the bot **automatically detects the question region** on screen using two reference points:
- The **"Cancel Search"** button on the searching screen → gives the bottom boundary
- The **"01:04" blue timer** on the countdown screen → gives the top boundary

This makes the bot work on **any Mac screen size** without any hardcoded coordinates.

---

## Features

- ✅ Dynamically detects question region — works on any Mac screen size
- ✅ Supports addition, subtraction, multiplication and division
- ✅ Handles multi-line questions (e.g. 5 numbers stacked vertically)
- ✅ Handles OCR misreading ÷ as + intelligently
- ✅ Auto-retries if stuck on the same question
- ✅ Runs for the full 60 second match duration

---

## Requirements

- Mac (tested on MacBook with Chrome)
- C++ compiler (g++)
- [Tesseract OCR](https://tesseract-ocr.github.io/tessdoc/Installation.html)

### Install Tesseract on Mac
```bash
brew install tesseract
```

---

## Setup & Usage

### Step 1 — Clone the repository
```bash
git clone https://github.com/yourusername/matiks-bot.git
cd matiks-bot
```

### Step 2 — Compile the bot
```bash
g++ -o bot bot.cpp
```

### Step 3 — Open Matiks in Chrome
Go to [matiks.in](https://matiks.in) and navigate to **Math Sprint** mode and click to search for an opponent.

### Step 4 — Run the bot
```bash
./bot
```

### Step 5 — Let it run!
The bot will:
1. Wait for the **"Cancel Search"** button to appear → detect bottom boundary
2. Wait for the **"01:04" countdown timer** to appear → detect top boundary
3. Calculate the question region automatically
4. Start solving questions once the game begins

---

## How Question Detection Works

The bot uses Tesseract's TSV output to find two reference points on screen:

```
"01:04" timer (top boundary)
        │
        │  ← question appears here
        │
"Enter Answer" box (bottom boundary)
```

The width of the question region is derived from the **cursor position** which is naturally centered on the input box.

All coordinates are divided by 2 to account for Mac's **Retina display** (2x pixel density).

---

## How the Math Solver Works

Questions appear vertically like this:
```
  56
+40
-56
+45
```

The bot:
1. Treats the first line as a plain number
2. For each subsequent line, extracts the operator (first character) and the number (rest of the string)
3. Applies each operation sequentially to get the final answer

### Division Edge Case
Tesseract often misreads `÷` as `+`. The bot handles this by:
- If the question has only 2 lines and the answer is wrong → retry treating `+` as `÷`

---

## Project Structure

```
matiks-bot/
├── bot.cpp          # Main bot source code
├── README.md        # This file
```

---

## Limitations

- Works only on **Mac** (uses `screencapture`, `sips`, `osascript`)
- Works only on **Chrome** in the browser
- OCR accuracy depends on screen resolution and font rendering
- Fixed 2 second delay between questions (can be reduced)

---

## Built With

| Tool | Purpose |
|------|---------|
| C++ | Core bot logic |
| Tesseract OCR | Reading questions from screenshots |
| sips | Image resizing for better OCR accuracy |
| screencapture | Taking screenshots on Mac |
| osascript | Simulating keyboard input on Mac |

---

## Note

This bot is built for educational purposes to explore OCR, screen automation, and C++ systems programming.