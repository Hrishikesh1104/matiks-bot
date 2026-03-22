#include "../include/FlashAnzanBot.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <climits>
#include <cmath>
#include <unistd.h>

FlashAnzanBot::FlashAnzanBot()
    : targetDigits(1), flashTime(1500), includeSubtraction(false) {}

// detect cancel search
bool FlashAnzanBot::detectCancelSearch() {
    generateTSV();

    std::ifstream file("fulloutput.tsv");
    if (!file.is_open()) {
        cout << "ERROR: Could not open fulloutput.tsv!" << endl;
        return false;
    }

    string line;
    getline(file, line);

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

        if (text == "Cancel") {
            cancelX = left;
            cancelY = top;
            cancelW = width;
            cout << ">>> Cancel found! cancelX=" << cancelX << " cancelY=" << cancelY << endl;
        }
    }

    if (cancelX == -1) {
        cout << " cancelX=" << cancelX << " — waiting..." << endl;
        return false;
    }

    return true;
}

// gets user settings
void FlashAnzanBot::getUserSettings() {
    cout << "\n=== Flash Anzan Settings ===\n";

    cout << "Number of digits (1-5) [default 1]: ";
    cin >> targetDigits;
    if(targetDigits > 5) targetDigits = 5;
    else if(targetDigits < 1) targetDigits = 1;

    cout << "Time interval ms [default 1500]: ";
    cin >> flashTime;
    flashTime = (flashTime/100)*100;

    char sub;
    cout << "Include subtraction? (y/n) [default n]: ";
    cin >> sub;
    includeSubtraction = (sub == 'y' || sub == 'Y');

    cout << "\n→ Settings confirmed: "
              << targetDigits << " digit(s)\n"
              << flashTime << "ms\n"
              <<"subtraction = "<< (includeSubtraction ? "ON" : "OFF") << "\n\n";
}

// applies user settings
void FlashAnzanBot::applySettings(bool first) {
    cout << "digitPlus=(" << digitPlusX << "," << digitPlusY << ")\n";
    cout << "digitMinus=(" << digitMinusX << "," << digitMinusY << ")\n";
    cout << "timePlus=(" << timePlusX << "," << timePlusY << ")\n";
    cout << "timeMinus=(" << timeMinusX << "," << timeMinusY << ")\n";

    int curDigit;
    if(first) curDigit = getCurrentDigits();
    else curDigit = prevDigits;
    cout << "Digits: " << curDigit << " → " << targetDigits << endl;
    int dDiff = targetDigits - curDigit;
    if (dDiff != 0) {
        int x = dDiff > 0 ? digitPlusX : digitMinusX;
        int y = dDiff > 0 ? digitPlusY : digitMinusY;
        if (x == -1) { cerr << "time button not found!\n"; }
        else {
            string cmd = "cliclick";
            for (int i = 0; i < abs(dDiff); i++){
                cmd += " c:" + to_string(x) + "," + to_string(y);
            }
            system(cmd.c_str());
        }
    }
    prevDigits = targetDigits;

    int curTime;
    if(first) curTime = getCurrentFlashTime();
    else curTime = prevFlashTime;
    cout << "Time: " << curTime << "ms → " << flashTime << "ms\n";
    int tDiff = flashTime - curTime;
    if(tDiff<0) tDiff = max(tDiff, -2000);
    else tDiff = min(tDiff, 2000);
    if (tDiff != 0) {
        int x = tDiff > 0 ? timePlusX : timeMinusX;
        int y = tDiff > 0 ? timePlusY : timeMinusY;
        if (x == -1) { cerr << "time button not found!\n"; }
        else {
            string cmd = "cliclick";
            for (int i = 0; i < abs(tDiff) / 100; i++){
                cmd += " c:" + to_string(x) + "," + to_string(y);
            }
            system(cmd.c_str());
        }
    }
    prevFlashTime = curTime + tDiff;

    if (includeSubtraction) {
        cout << "Enabling subtraction...\n";
        clickToggle();
    }

    cout << "Settings applied!\n";
}

// gets default number of digits
int FlashAnzanBot::getCurrentDigits() {
    generateTSV();

    std::ifstream file("fulloutput.tsv");
    string line;
    getline(file, line);

    string prevWord;
    while (getline(file, line)) {
        std::istringstream ss(line);
        string temp;
        vector<string> cols;
        while (getline(ss, temp, '\t')) cols.push_back(temp);
        if (cols.size() < 12) continue;
        if (cols[10] == "-1") continue;

        string word = cols[11];
        if (word == "DIGIT") {
            try{
                return stoi(prevWord);
            }
            catch(...){
                return 3;
            }
        }
        prevWord = word;
    }
    return 3;
}

// gets default flash time
int FlashAnzanBot::getCurrentFlashTime() {
    generateTSV();

    std::ifstream file("fulloutput.tsv");
    string line;
    getline(file, line);

    while (getline(file, line)) {
        std::istringstream ss(line);
        string temp;
        vector<string> cols;
        while (getline(ss, temp, '\t')) cols.push_back(temp);
        if (cols.size() < 12) continue;
        if (cols[10] == "-1") continue;

        string word = cols[11];
        if (word.size() >= 3 && word.substr(word.size() - 2) == "MS") {
            try {
                return stoi(word.substr(0, word.size() - 2));
            } catch(...) {
                cerr << "getCurrentFlashTime: could not parse '" << word << "'\n";
                return 1500; 
            }
        }
    }
    return 1500;
}

// toggles include subtraction on prep page
void FlashAnzanBot::clickToggle() {
    generateTSV();

    std::ifstream file("fulloutput.tsv");
    string line;
    getline(file, line); 

    int includeY = -1;
    while (getline(file, line)) {
        istringstream ss(line);
        string temp;
        vector<string> cols;
        while (getline(ss, temp, '\t')) cols.push_back(temp);
        if (cols.size() < 12) continue;
        if (cols[10] == "-1") continue;
        if (cols[11] == "Include") {
            includeY = stoi(cols[7]) / 2; // Retina ÷2
            break;
        }
    }
    file.close();

    if (includeY == -1) {
        cerr << "Could not find Include label\n";
        return;
    }

    // Find rightmost element on same row as Include
    std::ifstream file2("fulloutput.tsv");
    getline(file2, line); // skip header

    int bestX = -1, bestY = -1;

    while (getline(file2, line)) {
        istringstream ss(line);
        string temp;
        vector<string> cols;
        while (getline(ss, temp, '\t')) cols.push_back(temp);
        if (cols.size() < 12) continue;
        if (cols[10] == "-1") continue;

        int y    = stoi(cols[7]) / 2; // Retina ÷2
        int x    = stoi(cols[6]) / 2;
        int dist = abs(y - includeY);

        if (dist < 40 && x > bestX) {
            bestX = x + stoi(cols[8]) / 4;
            bestY = y + stoi(cols[9]) / 4;
        }
    }
    file2.close();

    if (bestX == -1) {
        cerr << "Could not find toggle\n";
        return;
    }

    string cmd = "cliclick c:" + to_string(bestX) + "," + to_string(bestY);
    system(cmd.c_str());
    usleep(7000000);
}

// captures flashed number and submits final sum
void FlashAnzanBot::captureFlashedNumber() {
    int ans = 0;
    int lastNumber = INT_MIN;
    int iterCount = 0;
    bool questionVisible = false;

    while (true) {
        // check isEnterAnswerVisible every 5 iterations
        if (isEnterAnswerVisible()) break;
        iterCount++;

        // crop box region and run TSV
        string cropCmd = "screencapture -R " +
                         to_string(box.x) + "," +
                         to_string(box.y) + "," +
                         to_string(box.width) + "," +
                         to_string(box.height) +
                         " flashnum.png";
        system(cropCmd.c_str());
        system("tesseract flashnum.png flashnum tsv 2>/dev/null");

        ifstream file("flashnum.tsv");
        string line;
        getline(file, line); // skip header

        while (getline(file, line)) {
            istringstream ss(line);
            string temp;
            vector<string> cols;
            while (getline(ss, temp, '\t')) cols.push_back(temp);
            if (cols.size() < 12) continue;
            if (cols[10] == "-1") continue;

            string word = cols[11];
            if (word == "Question" || word == "question"){
                questionVisible = true;
                submitAnswer(ans);
                return;
            }

            string cleaned = "";
            for (char c : word)
                if (c != ' ' && c != '\r' && c != '\n') cleaned += c;
            if (cleaned.empty()) continue;

            try {
                int num = stoi(cleaned);
                if (num != lastNumber) {
                    if (cleaned[0] != '-') ans += num;
                    else ans -= abs(num);
                    cout << "Read: " << cleaned << " (sum=" << ans << ")\n";
                    lastNumber = num;
                }
            } catch(...) {
                // not a number — ignore
            }
        }
        file.close();

        usleep(30000); // poll every 30ms
    }

    submitAnswer(ans);
}

// tells availibility of enter answer
bool FlashAnzanBot::isEnterAnswerVisible() {
    generateTSV();

    std::ifstream file("fulloutput.tsv");
    string line;
    getline(file, line); 

    while (getline(file, line)) {
        istringstream ss(line);
        string temp;
        vector<string> cols;
        while (getline(ss, temp, '\t')) cols.push_back(temp);
        if (cols.size() < 12) continue;
        if (cols[10] == "-1") continue;
        if (cols[11] == "Question") return true;
    }
    return false;
}

// submits answer
void FlashAnzanBot::submitAnswer(int sum) {
    humanReactionDelay();
    clearInput();
    typeAnswer(to_string(sum));
}

// checks for the prep page
bool FlashAnzanBot::isPrepPhaseActive() {
    generateTSV();

    std::ifstream file("fulloutput.tsv");
    string line;
    getline(file, line); 

    int numberY = -1, timeY = -1;
    bool prepFound = false;

    while (getline(file, line)) {
        istringstream ss(line);
        string temp;
        vector<string> cols;
        while (getline(ss, temp, '\t')) cols.push_back(temp);
        if (cols.size() < 12) continue;
        if (cols[10] == "-1") continue;

        string word = cols[11];
        int x = stoi(cols[6]) / 2;
        int y = stoi(cols[7]) / 2;
        int w = stoi(cols[8]) / 2;
        int h = stoi(cols[9]) / 2;

        if (word == "PREP")   prepFound = true;
        if (word == "Number") numberY = y;
        if (word == "Time")   timeY   = y;

        if (word == "+" || word == "-") {
            if (numberY != -1 && abs(y - numberY) < 40) {
                if (word == "+" && digitPlusX == -1) {
                    digitPlusX = x + w / 2;
                    digitPlusY = y + h / 2;
                }
                if (word == "-" && digitMinusX == -1) {
                    digitMinusX = x + w / 2;
                    digitMinusY = y + h / 2;
                }
            }
            if (timeY != -1 && abs(y - timeY) < 40) {
                if (word == "+" && timePlusX == -1) {
                    timePlusX = x + w / 2;
                    timePlusY = y + h / 2;
                }
                if (word == "-" && timeMinusX == -1) {
                    timeMinusX = x + w / 2;
                    timeMinusY = y + h / 2;
                }
            }
        }
    }
    file.close();
    return prepFound;
}

// tells if the match has started or not
bool FlashAnzanBot::isStartingCountdown() {
    generateTSV();

    std::ifstream file("fulloutput.tsv");
    string line;
    getline(file, line);

    while (getline(file, line)) {
        istringstream ss(line);
        string temp;
        vector<string> cols;
        while (getline(ss, temp, '\t')) cols.push_back(temp);
        if (cols.size() < 12) continue;
        if (cols[10] == "-1") continue;
        if (cols[11] == "Starting") {
            int top = stoi(cols[7]) / 2;
            cout<<"starting y coordinate = "<<top<<endl;
            cout<<"cancel y coordinate = "<<cancelY<<endl;
            int halfHeight = cancelY - top;
            box.height = halfHeight*2 + 100;
            timerY = cancelY - box.height;
            box.x = cancelX - 250;
            box.y = timerY + 30;
            box.width = 600;
            box.found = true;
            cout<<"box dimensions = "<<box.x<<" "<<box.y<<" "<<box.width<<" "<<box.height<<" "<<box.found<<endl;
            return true;
        }
    }
    box.found = false;
    return false;
}

// complete bot pipeline
void FlashAnzanBot::runBot() { 

    while (!detectCancelSearch()) usleep(50000);
    while (!isPrepPhaseActive()) usleep(20000);

    for (int round = 1; round <= 3; round++) {
        cout << "\n=== ROUND " << round << " ===\n";

        while (!isPrepPhaseActive()) usleep(20000);
        cout << "Prep phase detected!\n";

        // apply settings
        if (round == 1) applySettings(true);
        else applySettings(false);

        // wait for countdown
        cout << "Waiting for countdown...\n";
        auto start = chrono::steady_clock::now();
        while (!isStartingCountdown()){
            auto now = chrono::steady_clock::now();
            int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
            cout<<"duration elapsed : "<<elapsed<<endl;
        }
        cout<<"Countdown detected"<<endl;
        start = chrono::steady_clock::now();
        while (isStartingCountdown()){
            auto now = chrono::steady_clock::now();
            int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
            cout<<"duration elapsed : "<<elapsed<<endl;
            if(!isStartingCountdown()) break;
        }

        cout << "Game started!\n";
        captureFlashedNumber();

        cout << "Round " << round << " complete!\n";

        switchToVSCode();
        getUserSettings();
        switchToChrome();
    }
}