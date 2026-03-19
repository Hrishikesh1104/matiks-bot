#pragma once
#include "MatiksBot.h"

class MathBot : public MatiksBot {
private:
    vector<string> readOCROutput();
    string solve(vector<string> lines, int& sameQuestionCount);
public:
    void runBot() override;
};