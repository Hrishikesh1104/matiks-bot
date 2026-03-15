#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
using namespace std;

// STEP 1 - Take screenshot
void takeScreenshot() {
    // NOTE: These coordinates work only on 13.6 inch MacBook
    // with Chrome in fullscreen mode.
    system("screencapture -R 529,325,383,370 question.png");
}

// STEP 2 - Run OCR
void runOCR() {
    // Scale image up by 3x for better OCR accuracy
    system("sips -z 1110 1149 question.png --out question_large.png");
    
    // Convert to grayscale
    system("sips -s format png question_large.png --out question_large.png");
    
    // Run tesseract on the enlarged image
    system("tesseract question_large.png output -l eng --psm 6 --oem 3 -c tessedit_char_whitelist=0123456789+-x/");
}

// STEP 2 - Read OCR output into vector
vector<string> readOCROutput() {
    vector<string> lines;
    ifstream file("output.txt");
    string line;
    bool isFirstLine = true;

    while (getline(file, line)) {
        if (line == "") continue;

        // Remove all spaces
        string cleaned = "";
        for (char c : line) {
            if (c != ' ') cleaned += c;
        }

        if (isFirstLine) {
            // First line is always a plain number — extract digits only
            string digits = "";
            for (char c : cleaned) {
                if (isdigit(c)) digits += c;
            }
            if (digits != "") {
                lines.push_back(digits);
                isFirstLine = false;
            }
        } else {
            // For subsequent lines, extract operator and digits
            char op = cleaned[0];

            // Extract only digits from the rest
            string digits = "";
            for (int i = 1; i < cleaned.size(); i++) {
                if (isdigit(cleaned[i])) digits += cleaned[i];
            }

            // If op is not a known operator, skip this line
            if (op != '+' && op != '-' && op != 'x' && op != '/') {
                continue;
            }

            if (digits != "") {
                lines.push_back(string(1, op) + digits);
            }
        }
    }

    return lines;
}

// STEP 3 - Solve the math
string solve(vector<string> lines, int &sameQuestionCount) {
    int answer = stoi(lines[0]);

    for (int i = 1; i < lines.size(); i++) {
        string line = lines[i];
        char op = line[0];
        int num = stoi(line.substr(1));

        if (lines.size() != 2 && sameQuestionCount != 1 && op == '+') answer += num;
        else if(lines.size() == 2 && sameQuestionCount == 1 && op == '+') answer += num;
        else if (lines.size()==2 && op == '+') answer /= num;
        else if (op == '-') answer -= num;
        else if (op == 'x') answer *= num;
    }

    return to_string(answer);
}

// STEP 4 - Type the answer
void typeAnswer(string answer) {
    string command = "osascript -e 'tell application \"System Events\" to keystroke \"" + answer + "\"'";
    system(command.c_str());
}

// Clear the input field
void clearInput() {
    system("osascript -e 'tell application \"System Events\" to keystroke \"a\" using command down'");
    system("osascript -e 'tell application \"System Events\" to key code 51'");
}

int main() {
    cout << "Starting bot in 5 seconds... Switch to Matiks!" << endl;
    system("sleep 3");

    auto start = chrono::steady_clock::now();
    vector<string> lastLines;
    int sameQuestionCount = 0;

    while (true) {
        // Check if 1 minute has passed
        auto now = chrono::steady_clock::now();
        int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
        if (elapsed >= 58) break;

        // Step 1
        takeScreenshot();

        // Step 2
        runOCR();
        vector<string> lines = readOCROutput();

        // Skip if OCR didn't detect anything valid
        if (lines.empty()) continue;

        // Check if same question is repeating
        if (lines == lastLines) {
            sameQuestionCount++;
        } 
        else {
            sameQuestionCount = 0;
            lastLines = lines;
        }

        // If same question seen 2 times → stuck, clear and retry for addition
        if (sameQuestionCount >= 1) {
            string answer = solve(lines, sameQuestionCount);
            clearInput();
            typeAnswer(answer);
            sameQuestionCount = 0;
            lastLines.clear();
            this_thread::sleep_for(chrono::milliseconds(500));
            continue;
        }

        // Step 3
        string answer = solve(lines, sameQuestionCount);

        // Step 4
        clearInput();
        typeAnswer(answer);

        cout << ": Answer = " << answer << endl;

        // Small delay for screen to update
        this_thread::sleep_for(chrono::milliseconds(500));
    }


    return 0;
}