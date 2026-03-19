#include "MathBot.h"

vector<string> MathBot::readOCROutput() {
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
        } else {
            char op = cleaned[0];
            string digits = "";
            for (int i = 1; i < cleaned.size(); i++) {
                if (isdigit(cleaned[i])) digits += cleaned[i];
            }
            if (op != '+' && op != '-' && op != 'x' && op != '/') continue;
            if (digits != "") lines.push_back(string(1, op) + digits);
        }
    }

    return lines;
}

string MathBot::solve(vector<string> lines, int& sameQuestionCount) {
    int answer = stoi(lines[0]);

    for (int i = 1; i < lines.size(); i++) {
        string line = lines[i];
        char op = line[0];
        int num = stoi(line.substr(1));

        if (lines.size() != 2 && sameQuestionCount != 1 && op == '+') answer += num;
        else if (lines.size() == 2 && sameQuestionCount == 1 && op == '+') answer += num;
        else if (lines.size() == 2 && op == '+') answer /= num;
        else if (op == '-') answer -= num;
        else if (op == 'x') answer *= num;
    }

    return to_string(answer);
}

void MathBot::runBot() {
    cout << "Waiting for the coordinates of bounding box\n";
    timerY = -1;

    auto start = chrono::steady_clock::now();
    box = {0, 0, 0, 0, false};

    while (!box.found) {
        auto now = chrono::steady_clock::now();
        int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
        if (elapsed >= 15) return;
        box = detectQuestionRegion();
        if (!box.found) this_thread::sleep_for(chrono::milliseconds(500));
    }

    cout << "Coordinates found\n";
    this_thread::sleep_for(chrono::milliseconds(3000));

    start = chrono::steady_clock::now();
    vector<string> lastLines;
    int sameQuestionCount = 0;

    while (true) {
        auto now = chrono::steady_clock::now();
        int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
        if (elapsed >= 60) break;

        takeScreenshot();
        runOCR();
        vector<string> lines = readOCROutput();

        if (lines.empty()) continue;

        if (lines == lastLines) {
            sameQuestionCount++;
        } else {
            sameQuestionCount = 0;
            lastLines = lines;
        }

        if (sameQuestionCount >= 1) {
            string answer = solve(lines, sameQuestionCount);
            clearInput();
            typeAnswer(answer);
            sameQuestionCount = 0;
            lastLines.clear();
            this_thread::sleep_for(chrono::milliseconds(2600));
            continue;
        }

        string answer = solve(lines, sameQuestionCount);
        clearInput();
        typeAnswer(answer);
        cout << ": Answer = " << answer << endl;
        this_thread::sleep_for(chrono::milliseconds(2600));
    }
}