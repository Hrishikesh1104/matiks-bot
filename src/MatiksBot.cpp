#include "../include/MatiksBot.h"

void MatiksBot::openURL(string url) {
    string cmd = "osascript -e 'tell application \"Google Chrome\" to open location \"" + url + "\"'";
    system(cmd.c_str());
}

void MatiksBot::takeScreenshot() {
    string cmd = "screencapture -R " +
                to_string(box.x) + "," +
                to_string(box.y) + "," +
                to_string(box.width) + "," +
                to_string(box.height) +
                " question.png";
    system(cmd.c_str());
}

void MatiksBot::runOCR() {
    int scaledW = box.width  * 3;
    int scaledH = box.height * 3;
    string sipsCmd = "sips -z " + to_string(scaledH) + " " + to_string(scaledW) + " question.png --out question_large.png";
    system(sipsCmd.c_str());
    system("convert question_large.png -brightness-contrast 0x80 question_large.png 2>/dev/null");
    system("tesseract question_large.png output -l eng --psm 11 --oem 3 -c tessedit_char_whitelist=0123456789+-x/");
}

void MatiksBot::clearInput() {
    system("osascript -e 'tell application \"System Events\" to keystroke \"a\" using command down'");
    system("osascript -e 'tell application \"System Events\" to key code 51'");
}

bool MatiksBot::shouldMakeError() {
    return (rand() % 20) == 0;
}

string MatiksBot::generateWrongAnswer(string &correctAnswer) {
    int correct = stoi(correctAnswer);
    
    int error = (rand() % 15) + 1;
    
    if (rand() % 2 == 0) correct += error;
    else correct -= error;
    
    return to_string(correct);
}

void MatiksBot::humanReactionDelay() {
    int delay = 150 + (rand() % 250);
    this_thread::sleep_for(chrono::milliseconds(delay));
}

void MatiksBot::typeAnswer(string answer) {
    for (int i = 0; i < answer.size(); i++) {
        // ~7% chance of making a typo
        if (rand() % 15 == 0) {
            // type a random wrong character
            char wrongChar = '0' + (rand() % 20);
            string wrongCmd = "osascript -e 'tell application \"System Events\" to keystroke \"" + string(1, wrongChar) + "\"'";
            system(wrongCmd.c_str());

            this_thread::sleep_for(chrono::milliseconds(100 + rand() % 150));

            // backspace
            system("osascript -e 'tell application \"System Events\" to key code 51'");
        }

        // type correct character
        string cmd = "osascript -e 'tell application \"System Events\" to keystroke \"" + string(1, answer[i]) + "\"'";
        system(cmd.c_str());

        // random delay between characters
        int charDelay = 200 + (rand() % 100);
        this_thread::sleep_for(chrono::milliseconds(charDelay));
    }
}

void MatiksBot::switchToVSCode() {
    system("open -a 'Visual Studio Code'");
}

void MatiksBot::switchToChrome() {
    system("open -a 'Google Chrome'");
}

void MatiksBot::generateTSV(){
    system("screencapture fullscreen.png");
    system("tesseract fullscreen.png fulloutput tsv 2>/dev/null");
}

BoundingBox MatiksBot::detectQuestionRegion(bool updateCancel) {
    generateTSV();

    std::ifstream file("fulloutput.tsv");
    if (!file.is_open()) {
        cout << "ERROR: Could not open fulloutput.tsv!" << endl;
        return {0, 0, 0, 0, false};
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

        if (text == "01:04" || text == "01:03" ||
            text == "01:02" || text == "01:01" ||
            text == "01:00") {
            timerY = top;
            cout << ">>> Timer found! timerY=" << timerY << endl;
        }

        if (updateCancel && text == "Cancel") {
            if (top > 600 && left > 500 && left < 900) {
                cancelX = left;
                cancelY = top;
                cancelW = width;
                cout << ">>> Cancel found! cancelX=" << cancelX << " cancelY=" << cancelY << endl;
            }
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

vector<pair<int,int>> MatiksBot::decideNextGame() {
    system("screencapture fullscreen.png");
    system("tesseract fullscreen.png fulloutput tsv 2>/dev/null");

    ifstream file("fulloutput.tsv");
    if (!file.is_open()) {
        cout << "ERROR: Could not open fulloutput.tsv!" << endl;
        return {{-1, -1}, {-1, -1}};
    }

    string line;
    getline(file, line);

    int rematchX = -1, rematchY = -1;
    int newGameX = -1, newGameY = -1;

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
        transform(text.begin(), text.end(), text.begin(), ::tolower);
        int left = stoi(cols[6]) / 2;
        int top  = stoi(cols[7]) / 2;

        if (text.find("rematch") != string::npos) {
            rematchX = left;
            rematchY = top;
            cout << ">>> Rematch found! rematchX=" << rematchX << " rematchY=" << rematchY << endl;
        }

        if (text.find("new") != string::npos) {
            newGameX = left;
            newGameY = top;
            cout << ">>> NewGame found! newGameX=" << newGameX << " newGameY=" << newGameY << endl;
        }
    }

    if (rematchX == -1 || rematchY == -1 || newGameX == -1 || newGameY == -1) {
        return {{rematchX, rematchY}, {newGameX, newGameY}};
    }

    return {{rematchX + 10, rematchY + 10}, {newGameX + 10, newGameY + 10}};
}

void MatiksBot::runGameLoop() {
    bool opponentAccepted = true;
    int val;

    do {
        if (opponentAccepted) {
            runBot();
            this_thread::sleep_for(chrono::milliseconds(500));
            opponentAccepted = false;
            switchToVSCode();
            cout << "\n1. Rematch \n2. New game \n3. Exit\n";
            cin >> val;
            switchToChrome();
            this_thread::sleep_for(chrono::milliseconds(1000));
        }

        if (val == 3) break;

        else if (val == 1 || val == 2) {
            this_thread::sleep_for(chrono::milliseconds(1000));
            vector<pair<int,int>> newGameXY = decideNextGame();

            if (val == 1) {
                if (newGameXY[0].first == -1 || newGameXY[0].second == -1) break;
                string cmd = "cliclick c:" + to_string(newGameXY[0].first) + "," + to_string(newGameXY[0].second);
                system(cmd.c_str());
                system(cmd.c_str());

                timerY = -1;
                auto start = chrono::steady_clock::now();

                while (true) {
                    auto now = chrono::steady_clock::now();
                    int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();

                    if (elapsed >= 10) {
                        cout << "Opponent didn't accept rematch!" << endl;
                        break;
                    }

                    BoundingBox b = detectQuestionRegion(false);
                    if (b.found) {
                        cout << "Opponent accepted rematch! Starting bot..." << endl;
                        opponentAccepted = true;
                        box = b;
                        break;
                    }

                    this_thread::sleep_for(chrono::milliseconds(500));
                }

                if (!opponentAccepted) {
                    switchToVSCode();
                    cout << "\n1. Rematch \n2. New game \n3. Exit\n";
                    cin >> val;
                    switchToChrome();
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }
            }

            else {
                if (newGameXY[1].first == -1 || newGameXY[1].second == -1) break;
                string cmd = "cliclick c:" + to_string(newGameXY[1].first) + "," + to_string(newGameXY[1].second);
                system(cmd.c_str());
                system(cmd.c_str());
                timerY  = -1;
                cancelX = -1;
                cancelY = -1;
                cancelW = -1;
                opponentAccepted = true;
            }
        }

        else cout << "Invalid choice!" << endl;

    } while (val == 1 || val == 2);
}