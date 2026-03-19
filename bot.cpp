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

// Open url
void openURL(string url) {
    string cmd = "osascript -e 'tell application \"Google Chrome\" to open location \"" + url + "\"'";
    system(cmd.c_str());
}

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
    
    system("convert question_large.png -brightness-contrast 0x80 question_large.png 2>/dev/null");
    
    system("tesseract question_large.png output -l eng --psm 11 --oem 3 -c tessedit_char_whitelist=0123456789+-x/");
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

// questions region detection required coordinates
int timerY  = -1;
int cancelX = -1;
int cancelY = -1;
int cancelW = -1;

// Detect the Questions Region
BoundingBox detectQuestionRegion(bool updateCancel = true) {

    system("screencapture fullscreen.png");

    system("tesseract fullscreen.png fulloutput tsv 2>/dev/null");

    ifstream file("fulloutput.tsv");
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

        if (text == "01:04" || 
            text == "01:03" || 
            text == "01:02" || 
            text == "01:01" || 
            text == "01:00") {
            timerY = top;
            cout << ">>> Timer found! timerY=" << timerY << endl;
        }

        if (updateCancel && text == "Cancel") {
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





vector<pair<int, int>> decideNextGame() {

    system("screencapture fullscreen.png");

    system("tesseract fullscreen.png fulloutput tsv 2>/dev/null");

    ifstream file("fulloutput.tsv");
    if (!file.is_open()) {
        cout << "ERROR: Could not open fulloutput.tsv!" << endl;
        return {{-1, -1}, {-1, -1}};
    }

    string line;
    getline(file, line); 

    static int rematchX = -1;
    static int rematchY = -1;
    static int newGameX = -1;
    static int newGameY = -1;

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
        int left  = stoi(cols[6]) / 2;
        int top   = stoi(cols[7]) / 2;
        int width = stoi(cols[8]) / 2;

        if (text.find("rematch") != string::npos) {
            rematchX = left;
            rematchY = top;
            cout << ">>> Rematch found! rematchX= " << rematchX << " rematchY= " << rematchY<<endl;
        }

        if (text.find("new") != string::npos) {
            newGameX = left;
            newGameY = top;
            cout << ">>> Rematch found! newGameX= " << newGameX << " newGameY= " << newGameY<<endl;
        }
    }

    if (rematchX == -1 || rematchY == -1 || newGameX == -1 || newGameY == -1) {
        cout << "rematchX= " << rematchX << " rematchY= " << rematchY << " newGameX= " << newGameX << " newGameY= " << newGameY << endl;
        return {{rematchX, rematchY}, {newGameX, newGameY}};
    }

    return {{rematchX+10, rematchY+10}, {newGameX+10, newGameY+10}};
}






//----------------------------
// Math bot
//----------------------------
void runMathBot(){
    cout<<"Waiting for the coordinates of bounding box\n";
    // Reset bounding box values for new match
    timerY  = -1;
    auto start = chrono::steady_clock::now();

    BoundingBox box = detectQuestionRegion();
    while(!box.found){
        auto now = chrono::steady_clock::now();
        int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
        if (elapsed >= 15) return ;
        box = detectQuestionRegion();
        if(!box.found){
            this_thread::sleep_for(chrono::milliseconds(500));
        }
    }
    cout<<"Coordinates found\n";
    
    this_thread::sleep_for(chrono::milliseconds(3000));
    
    start = chrono::steady_clock::now();
    vector<string> lastLines;
    int sameQuestionCount = 0;
    
    while (true) {
        auto now = chrono::steady_clock::now();
        int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
        if (elapsed >= 60) break;
    
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
            this_thread::sleep_for(chrono::milliseconds(2600));
            continue;
        }
    
        // Step 3
        string answer = solve(lines, sameQuestionCount);
    
        // Step 4
        clearInput();
        typeAnswer(answer);
    
        cout << ": Answer = " << answer << endl;
    
        this_thread::sleep_for(chrono::milliseconds(2600));
    }
}

//----------------------------
// Main
//----------------------------
int main() {

    cout << "\n=============================\n";
    cout << "     MATIKS BOT LAUNCHER\n";
    cout << "=============================\n";
    cout << "1. Online Duels (Math Sprint)\n";
    cout << "2. Fastest Fingers Duels\n";
    cout << "3. Cross Math Duels\n";
    cout << "4. Ken Ken Duels\n";
    cout << "5. Math Maze Duels\n";
    cout << "6. Flash Anzan\n";
    cout << "7. Mindsnap Online\n";
    cout << "8. Ability Duels\n";
    cout << "=============================\n";
    cout << "Enter your choice (1-8): ";

    int choice;
    cin >> choice;

    string url = "";
    switch (choice) {
        case 1: url = "https://www.matiks.in/search?gameType=DMAS&gameMode=ONLINE_SEARCH&timeLimit=1"; break;
        case 2: url = "https://www.matiks.in/search?gameType=FASTEST_FINGER&gameMode=ONLINE_SEARCH&timeLimit=1"; break;
        case 3: url = "https://www.matiks.in/search?gameType=CROSS_MATH_PUZZLE&gameMode=ONLINE_SEARCH&timeLimit=2"; break;
        case 4: url = "https://www.matiks.in/search?gameType=KEN_KEN_PUZZLE&gameMode=ONLINE_SEARCH&timeLimit=2"; break;
        case 5: url = "https://www.matiks.in/search?gameType=MATH_MAZE_PUZZLE&gameMode=ONLINE_SEARCH&timeLimit=2"; break;
        case 6: url = "https://www.matiks.in/search?gameType=FLASH_ANZAN&gameMode=ONLINE_SEARCH&timeLimit=1.5"; break;
        case 7: url = "https://www.matiks.in/search?gameType=MIND_SNAP&gameMode=ONLINE_SEARCH&timeLimit=1"; break;
        case 8: url = "https://www.matiks.in/search?gameType=DMAS_ABILITY&gameMode=ONLINE_SEARCH&timeLimit=2"; break;
        default:
            cout << "Invalid choice!" << endl;
            return 1;
    }

    cout << "Opening game mode..." << endl;
    openURL(url);

    bool opponentAccepted = true;
    switch (choice) {
        int val;
        case 1:
            do{
                if(opponentAccepted){
                    runMathBot();
                    this_thread::sleep_for(chrono::milliseconds(500));
                    opponentAccepted = false;
                    system("open -a 'Visual Studio Code'");
                    cout << "\n1. Rematch \n2. New game \n3. Exit\n";
                    cin >> val;
                    system("open -a 'Google Chrome'");
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }

                if(val == 3) break;
                else if(val == 1 || val == 2){
                    this_thread::sleep_for(chrono::milliseconds(1000));
                    vector<pair<int, int>> newGameXY = decideNextGame();
                    if(val == 1){
                        if(newGameXY[0].first == -1 || newGameXY[0].second == -1) break;
                        string cmd = "cliclick c:" + to_string(newGameXY[0].first) + "," + to_string(newGameXY[0].second);
                        system(cmd.c_str());
                        system(cmd.c_str());

                        timerY  = -1;
                        auto start = chrono::steady_clock::now();

                        while(true) {
                            auto now = chrono::steady_clock::now();
                            int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
                            
                            if(elapsed >= 10) {
                                cout << "Opponent didn't accept rematch!" << endl;
                                break;
                            }
                            
                            BoundingBox box = detectQuestionRegion(false);
                            
                            if(box.found) {
                                cout << "Opponent accepted rematch! Starting bot..." << endl;
                                opponentAccepted = true;
                                break;
                            }
                            
                            this_thread::sleep_for(chrono::milliseconds(500));
                        }

                        if (!opponentAccepted) {
                            system("open -a 'Visual Studio Code'");
                            cout << "\n1. Rematch \n2. New game \n3. Exit\n";
                            cin >> val;
                            system("open -a 'Google Chrome'");
                            this_thread::sleep_for(chrono::milliseconds(1000));
                        }
                    }
                    else{
                        if(newGameXY[1].first == -1 || newGameXY[1].second == -1) break;
                        string cmd = "cliclick c:" + to_string(newGameXY[1].first) + "," + to_string(newGameXY[1].second);
                        system(cmd.c_str());
                        system(cmd.c_str());
                        opponentAccepted = true;
                    }
                }
                else cout<<"Invalid choice!"<<endl;
            } while(val == 1 || val == 2);
            break;

        case 2:
            do{
                if(opponentAccepted){
                    runMathBot();
                    this_thread::sleep_for(chrono::milliseconds(500));
                    opponentAccepted = false;
                    system("open -a 'Visual Studio Code'");
                    cout << "\n1. Rematch \n2. New game \n3. Exit\n";
                    cin >> val;
                    system("open -a 'Google Chrome'");
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }

                if(val == 3) break;
                else if(val == 1 || val == 2){
                    this_thread::sleep_for(chrono::milliseconds(1000));
                    vector<pair<int, int>> newGameXY = decideNextGame();
                    if(val == 1){
                        if(newGameXY[0].first == -1 || newGameXY[0].second == -1) break;
                        string cmd = "cliclick c:" + to_string(newGameXY[0].first) + "," + to_string(newGameXY[0].second);
                        system(cmd.c_str());
                        system(cmd.c_str());

                        timerY  = -1;
                        auto start = chrono::steady_clock::now();

                        while(true) {
                            auto now = chrono::steady_clock::now();
                            int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
                            
                            if(elapsed >= 10) {
                                cout << "Opponent didn't accept rematch!" << endl;
                                break;
                            }
                            
                            BoundingBox box = detectQuestionRegion(false);
                            
                            if(box.found) {
                                cout << "Opponent accepted rematch! Starting bot..." << endl;
                                opponentAccepted = true;
                                break;
                            }
                            
                            this_thread::sleep_for(chrono::milliseconds(500));
                        }

                        if (!opponentAccepted) {
                            system("open -a 'Visual Studio Code'");
                            cout << "\n1. Rematch \n2. New game \n3. Exit\n";
                            cin >> val;
                            system("open -a 'Google Chrome'");
                            this_thread::sleep_for(chrono::milliseconds(1000));
                        }
                    }
                    else{
                        if(newGameXY[1].first == -1 || newGameXY[1].second == -1) break;
                        string cmd = "cliclick c:" + to_string(newGameXY[1].first) + "," + to_string(newGameXY[1].second);
                        system(cmd.c_str());
                        system(cmd.c_str());
                        opponentAccepted = true;
                    }
                }
                else cout<<"Invalid choice!"<<endl;
            } while(val == 1 || val == 2);
            break;
            
        case 3: cout << "Bot for this mode is coming soon!" << endl; break;
        case 4: cout << "Bot for this mode is coming soon!" << endl; break;
        case 5: cout << "Bot for this mode is coming soon!" << endl; break;
        case 6: cout << "Bot for this mode is coming soon!" << endl; break;
        case 7: cout << "Bot for this mode is coming soon!" << endl; break;
        case 8: cout << "Bot for this mode is coming soon!" << endl; break;
    }

    cout << "Game Ended\n";

    return 0;
}