all:
	g++ -o bot main.cpp MatiksBot.cpp MathBot.cpp MindSnapBot.cpp

clean:
	rm -f bot