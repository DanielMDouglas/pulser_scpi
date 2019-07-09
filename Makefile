all:
	g++ set_pulser.cpp -o set_pulser -llxi

clean:
	rm set_pulser
