AUTOMAKE_OPTIONS = foreign subdir-objects

bin_PROGRAMS=robotservice

robotservice_CPPFLAGS=-Wall -I../CommonSA -I.

robotservice_LDADD=-lPocoData -lpthread -lPocoFoundation -lPocoUtil -lPocoNet -lPocoDataMySQL

robotservice_SOURCES= \
../CommonSA/SAHead.cpp \
../CommonSA/SAAssistant.cpp \
../CommonSA/SAContainer.cpp \
../CommonSA/SADatabase.cpp \
../CommonSA/SAHTTPServer.cpp \
../CommonSA/SALogChannel.cpp \
../CommonSA/SATimer.cpp \
../CommonSA/SATCPSocket.cpp \
../CommonSA/SAThreadPool.cpp \
../CommonSA/SAThreading.cpp \
../CommonSA/SAWebSocket.cpp \
stdafx.cpp \
Robot.cpp \
RobotCommon.cpp \
RobotHttpServer.cpp \
RobotManager.cpp \
RobotServer.cpp \
SessionManager.cpp \
WebSocketSession.cpp \
TCPCommunication.cpp \
JoyStickClient.cpp
