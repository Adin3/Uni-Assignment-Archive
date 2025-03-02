# the compiler: gcc for C program, define as g++ for C++
CC = g++
RM = rm
# compiler flags:
#  -g     - this flag adds debugging information to the executable file
#  -Wall  - this flag is used to turn on most compiler warnings
CFLAGS  = -g -Wall
# The build target 
TARGET_SERVER = server
TARGET_SUBSCRIBER = subscriber
all: $(TARGET_SERVER) $(TARGET_SUBSCRIBER)

$(TARGET_SERVER): $(TARGET_SERVER).cpp
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) $(TARGET_SERVER).cpp

$(TARGET_SUBSCRIBER): $(TARGET_SUBSCRIBER).cpp
	$(CC) $(CFLAGS) -o $(TARGET_SUBSCRIBER) $(TARGET_SUBSCRIBER).cpp

clean:
	$(RM) $(TARGET_SERVER) $(TARGET_SUBSCRIBER)