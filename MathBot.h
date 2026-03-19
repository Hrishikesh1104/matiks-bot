#pragma once
#include "MatiksBot.h"

class MathBot : public MatiksBot {
private:
    vector<string> readOCROutput();
    int calculateDelay(vector<string>& lines);
    string solve(vector<string> lines, int& sameQuestionCount);
public:
    void runBot() override;
};