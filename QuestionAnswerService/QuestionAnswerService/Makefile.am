AUTOMAKE_OPTIONS = foreign subdir-objects

bin_PROGRAMS=faqservice

faqservice_CPPFLAGS=-Wall -I../CommonSA -I../../CommonSA/mongoose -I.

faqservice_LDADD=-lPocoData -lpthread -lPocoFoundation -lPocoUtil -lPocoNet -lPocoDataMySQL -lPocoNetSSL

faqservice_SOURCES= \
../CommonSA/SAHead.cpp \
../CommonSA/SAAssistant.cpp \
../CommonSA/SAContainer.cpp \
../CommonSA/SADatabase.cpp \
../CommonSA/SAHTTPServer.cpp \
../CommonSA/SALogChannel.cpp \
../CommonSA/SATimer.cpp \
../CommonSA/SAMongooseServer.cpp \
../CommonSA/SAThreadPool.cpp \
../CommonSA/SAThreading.cpp \
../CommonSA/mongoose/mongoose.c \
stdafx.cpp \
FQACommon.cpp \
FQAServer.cpp
