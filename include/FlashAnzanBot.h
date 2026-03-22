#pragma once
#include "MatiksBot.h"
#include <vector>

class FlashAnzanBot : public MatiksBot {
public:
    FlashAnzanBot();
    void runBot() override;
    void getUserSettings();

private:
    int targetDigits;
    int prevDigits;
    int flashTime;
    int prevFlashTime;
    bool includeSubtraction;
    int digitPlusX = -1;
    int digitPlusY = -1;
    int digitMinusX = -1;
    int digitMinusY = -1;
    int timePlusX = -1;
    int timePlusY = -1;
    int timeMinusX = -1;
    int timeMinusY = -1;

    // Prep phase
    bool detectCancelSearch();
    void applySettings(bool first = true);
    int getCurrentDigits();
    int getCurrentFlashTime();
    void clickToggle();

    // Flash phase
    void captureFlashedNumber();
    bool isEnterAnswerVisible();
    void submitAnswer(int sum);

    // State helpers
    bool isPrepPhaseActive();
    bool isStartingCountdown();
};