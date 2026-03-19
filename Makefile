all:
	g++ -o bot main.cpp src/MatiksBot.cpp src/MathBot.cpp src/MindSnapBot.cpp -I include/

clean:
	rm -f bot