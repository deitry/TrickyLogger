#include "message_logger.h"
#include <stdio.h>

using namespace TrickyLogger;

int main(int argc, char** argv)
{
	//auto logger = new BufferedStreamLogger(&std::cout);
	auto logger = new BufferedFileLogger("debug.log");

	TLOG(logger, "Запись в лог 1/5", TrickyLogger::MessagePriority::Info)

	logger->Flush();


	TLOG(logger, "Запись в лог 2/5", TrickyLogger::MessagePriority::Debug)
	TLOG(logger, "Запись в лог 3/5", TrickyLogger::MessagePriority::Error)
	TLOG(logger, "Запись в лог 4/5", TrickyLogger::MessagePriority::Fatal)
	TLOG(logger, "Запись в лог 5/5", TrickyLogger::MessagePriority::Info)

	logger->Flush();


	delete logger;

	return 0;
}