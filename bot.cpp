#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>
using namespace std;

// Questions Region Dimensions
struct BoundingBox {
    int x, y, width, height;
    bool found;
};

// STEP 1 - Take screenshot
void takeScreenshot(BoundingBox box) {

    string cmd = "screencapture -R " +
                to_string(box.x) + "," +
                to_string(box.y) + "," +
                to_string(box.width) + "," +
                to_string(box.height) +
                " question.png";
    system(cmd.c_str());
}

// STEP 2 - Run OCR
void runOCR(BoundingBox box) {
    int scaledW = box.width  * 3;
    int scaledH = box.height * 3;
    string sipsCmd = "sips -z " + to_string(scaledH) + " " + to_string(scaledW) + " question.png --out question_large.png";
    system(sipsCmd.c_str());
    
    system("sips -s format png question_large.png --out question_large.png");
    
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

        string cleaned = "";
        for (char c : line) {
            if (c != ' ') cleaned += c;
        }

        if (isFirstLine) {
            string digits = "";
            for (char c : cleaned) {
                if (isdigit(c)) digits += c;
            }
            if (digits != "") {
                lines.push_back(digits);
                isFirstLine = false;
            }
        } 
        else {
            char op = cleaned[0];

            string digits = "";
            for (int i = 1; i < cleaned.size(); i++) {
                if (isdigit(cleaned[i])) digits += cleaned[i];
            }

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

// Detect the Questions Region
BoundingBox detectQuestionRegion() {

    system("screencapture fullscreen.png");

    system("tesseract fullscreen.png fulloutput tsv 2>/dev/null");

    ifstream file("fulloutput.tsv");
    if (!file.is_open()) {
        cout << "ERROR: Could not open fulloutput.tsv!" << endl;
        return {0, 0, 0, 0, false};
    }

    string line;
    getline(file, line); 

    static int timerY  = -1;
    static int cancelX = -1;
    static int cancelY = -1;
    static int cancelW = -1;

    while (getline(file, line)) {
        stringstream ss(line);
        vector<string> cols;
        string col;
        while (getline(ss, col, '\t')) {
            cols.push_back(col);
        }

        if (cols.size() < 12) continue;
        if (cols[0] != "5") continue;

        string text = cols[11];
        int left  = stoi(cols[6]) / 2;
        int top   = stoi(cols[7]) / 2;
        int width = stoi(cols[8]) / 2;

        if (text.find("01:0") != string::npos ||
            text.find("00:")  != string::npos) {
            timerY = top;
            cout << ">>> Timer found! timerY=" << timerY << endl;
        }

        if (text == "Cancel") {
            cancelX = left;
            cancelY = top;
            cancelW = width;
            cout << ">>> Cancel found! cancelX=" << cancelX << " cancelY=" << cancelY << " cancelW=" << cancelW << endl;
        }
    }

    if (timerY == -1 || cancelX == -1) {
        cout << "timerY=" << timerY << " cancelX=" << cancelX << " — waiting..." << endl;
        return {0, 0, 0, 0, false};
    }

    return {
        cancelX - 190,
        timerY + 30,
        400,
        cancelY - timerY - 15,
        true
    };
}

//----------------------------
// Main
//----------------------------
int main() {
    cout << "Starting bot in 5 seconds... Switch to Matiks!" << endl;

    cout<<"Waiting for the coords of bounding box\n";
    BoundingBox box = {0, 0, 0, 0, false};
    while(!box.found){
        box = detectQuestionRegion();
        if(!box.found){
            this_thread::sleep_for(chrono::milliseconds(500));
        }
    }
    cout<<"Coords found\n";
    
    this_thread::sleep_for(chrono::milliseconds(2000));
    
    auto start = chrono::steady_clock::now();
    vector<string> lastLines;
    int sameQuestionCount = 0;
    
    while (true) {
        auto now = chrono::steady_clock::now();
        int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
        if (elapsed >= 61) break;
    
        // Step 1
        takeScreenshot(box);
    
        // Step 2
        runOCR(box);
        vector<string> lines = readOCROutput();
    
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
            this_thread::sleep_for(chrono::milliseconds(2000));
            continue;
        }
    
        // Step 3
        string answer = solve(lines, sameQuestionCount);
    
        // Step 4
        clearInput();
        typeAnswer(answer);
    
        cout << ": Answer = " << answer << endl;
    
        this_thread::sleep_for(chrono::milliseconds(2000));
    }

    return 0;
}