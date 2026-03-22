#include "MathBot.h"
#include "FlashAnzanBot.h"

void runLauncher() {
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
            return;
    }

    MatiksBot* bot = nullptr;
    switch (choice) {
        case 1: bot = new MathBot(); break;
        case 2: bot = new MathBot(); break;
        case 6: {
            FlashAnzanBot* fb = new FlashAnzanBot();
            fb->getUserSettings();  
            bot = fb;
            break;
        }
        default:
            cout << "Bot for this mode is coming soon!" << endl;
            return;
    }

    cout << "Opening game mode..." << endl;
    bot->openURL(url);
    bot->runGameLoop();

    delete bot;
    cout << "Game Ended\n";
}