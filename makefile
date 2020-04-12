MPDController.exe: build/Input.o build/Output.o build/Button.o build/RotaryEncoder.o build/I2C.o build/LCD.o build/MPDConnection.o build/MPD.o build/MPDController.o
	g++ -o MPDController.exe build/Input.o build/Output.o build/Button.o build/RotaryEncoder.o build/I2C.o build/LCD.o build/MPDConnection.o build/MPD.o build/MPDController.o -Isrc/ --std=c++11

build/Button.o: src/Button.cpp
	g++ -g -c -o build/Button.o src/Button.cpp -Isrc/ --std=c++11

build/I2C.o: src/I2C.cpp
	g++ -g -c -o build/I2C.o src/I2C.cpp -Isrc/ --std=c++11

build/Input.o: src/Input.cpp
	g++ -g -c -o build/Input.o src/Input.cpp -Isrc/ --std=c++11

build/LCD.o: src/LCD.cpp
	g++ -g -c -o build/LCD.o src/LCD.cpp -Isrc/ --std=c++11

build/MPD.o: src/MPD.cpp
	g++ -g -c -o build/MPD.o src/MPD.cpp -Isrc/ --std=c++11

build/MPDConnection.o: src/MPDConnection.cpp
	g++ -g -c -o build/MPDConnection.o src/MPDConnection.cpp -Isrc/ --std=c++11

build/Output.o: src/Output.cpp
	g++ -g -c -o build/Output.o src/Output.cpp -Isrc/ --std=c++11

build/RotaryEncoder.o: src/RotaryEncoder.cpp
	g++ -g -c -o build/RotaryEncoder.o src/RotaryEncoder.cpp -Isrc/ --std=c++11

build/MPDController.o: src/MPDController.cpp
	g++ -g -c -o build/MPDController.o src/MPDController.cpp -Isrc/ --std=c++11

