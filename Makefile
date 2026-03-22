all:
	g++ -o bot main.cpp src/MatiksBot.cpp src/MathBot.cpp src/FlashAnzanBot.cpp -I include/

clean:
	rm -f bot