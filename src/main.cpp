#include "message_logger.h"
#include <stdio.h>

using namespace TrickyLogger;

// вывод в cout
TLOG_MAIN_COUT()

// или в указанный файл - раскомментировать одно, закомментировать другое
//TLOG_MAIN(BufferedFileLogger::Init("debug.log"))

int main(int argc, char** argv)
{
	TLOG( "Запись в лог 1/6", MessagePriority::Info);

	// при желании, можно переопределить цель вывода...
	//TLOG_INIT(BufferedFileLogger::Init("debug2.log"))

	TLOG( "Запись в лог 2/6", Debug);	// не должна выводиться - приоритет Debug ниже, чем приоритет лога по умолчанию
	TLOG( "Запись в лог 3/6", Error);
	
	// ...изменить приоритет...
	//TLOG_PRIOR(Error) // debug, info и warning игнорируются

	TLOG( "Запись в лог 4/6", Fatal);
	TLOG( "Запись в лог 5/6", Info);
	
	// ...вручную управлять сбросом буфера, если используется буферизация
	//TLOG_FLUSH()

	TLOG( "Запись в лог 6/6", Info);

	return 0;
}