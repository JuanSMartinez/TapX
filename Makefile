SRC_DIR=src/
BUILD_DIR=build/
LIBS=lib/libportaudio.a -lm -lrt -lasound -pthread 

Test: HapticSymbol MotuPlayer
	$(CXX) $(BUILD_DIR)HapticSymbol.o $(BUILD_DIR)MotuPlayer.o $(SRC_DIR)$(@).cpp $(LIBS) -o $(@)

MotuPlayer:
	$(CXX) -c $(SRC_DIR)$(@).cpp -o $(BUILD_DIR)$(@).o

HapticSymbol: 
	$(CXX) -c $(SRC_DIR)$(@).cpp -o $(BUILD_DIR)$(@).o

clean : 
	$(RM) $(BUILD_DIR)* Test 