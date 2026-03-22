#pragma once
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>
#include <algorithm>
using namespace std;

struct BoundingBox {
    int x, y, width, height;
    bool found;
};

class MatiksBot {
protected:
    BoundingBox box;
    int timerY  = -1;
    int cancelX = -1;
    int cancelY = -1;
    int cancelW = -1;

public:
    void openURL(string url);
    void takeScreenshot();
    void runOCR();
    void clearInput();
    bool shouldMakeError();
    string generateWrongAnswer(string &correctAnswer);
    void humanReactionDelay();
    void typeAnswer(string answer);
    void switchToVSCode();
    void switchToChrome();
    void generateTSV();
    BoundingBox detectQuestionRegion(bool updateCancel = true);
    vector<pair<int,int>> decideNextGame();
    void runGameLoop();
    virtual void runBot() = 0;
    virtual ~MatiksBot() {}
};