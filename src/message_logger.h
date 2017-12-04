/**
 * Класс, позволяющий логгировать сообщения с приоритетом и позволяющий
 * выводить их куда захочется 
 */

#ifndef H_MESSAGE_LOGGER
#define H_MESSAGE_LOGGER

#include <string>
#include <ctime>
#include <list>
#include <iostream>
#include <fstream>

// интерфейс, определяющий основные требования к логгеру -
// на данный момент будем подразумевать, что у интерфейса может быть несколько реализаций
namespace TrickyLogger
{

#define TLOG(logger, message, prior) \
(logger)->Write(TrickyLogger::DetailedMessage((message),(prior), __FILE__,__LINE__));


	// TODO: глянь https://habrahabr.ru/post/225363/
	// stack tracer https://habrahabr.ru/post/302170/
	// easy logging https://github.com/muflihun/easyloggingpp/tree/master/src

	// возможные приоритеты сообщений
	enum MessagePriority
	{
		Debug,
		Info,
		Warning,
		Error,
		Fatal
	};

	std::string PriorityToString(MessagePriority prior)
	{
		std::string result;

		switch (prior)
		{
		case Debug:
			return "Debug";
		case Info:
			return "Info";
		case Warning:
			return "Warning";
		case Error:
			return "Error";
		case Fatal:
			return "FATAL ERROR";
		default: 
			return "Unknown message type";
		}
	}

	// std::string timeStr(time_t time)
	// {
	// 	std::string result;

	// 	return resultstd::string timeStr(time_t time)
	// {
	// 	std::string result;

	// 	return result;
	;
	// }

	// интерфейс, определяющий базовые требования к сообщениям
	class IMessage
	{
		MessagePriority _priority;
		std::string _body;

	public:
		// публичный конструктор
		IMessage(std::string message, MessagePriority priority = MessagePriority::Info)
		{
			_body = message;
			_priority = priority;
		}

		virtual ~IMessage() {}

		//virtual std::string AsString() = 0; // если мы хотим поддерживать разные типы сообщений.

		virtual std::string getBody() { return _body; }
		MessagePriority getPriority() { return _priority; }	// не подразумеваем изменения типа сообщения после создания
	};

	// сообщение с дополнительной информацией
	class DetailedMessage : public IMessage
	{
		time_t _t;	// https://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-cs
		std::string _file;	// я так понимаю, в задании имеется в виду место, откуда пишется сообщение. Оставим просто строку
		int _lineNumber;

	public:
		DetailedMessage(std::string message,
						MessagePriority priority = MessagePriority::Info, 
						std::string fileName = "",
						int lineNumber = -1
						) :
						IMessage(message, priority),
						_t(time(nullptr)),	// инициализируем текущим временем
						_file(fileName),
						_lineNumber(lineNumber) {}
		
		time_t getTime() { return _t; }
		std::string getFile() { return _file; }
		int getLine() { return _lineNumber; }
	};

	// собственно, логгер.
	// На данный момент в виде интерфейса, потому что подразумевается множество различных реализаций 
	class ILogger
	{
	protected:
		MessagePriority _priority;	// разрешённый приоритет сообщений - с более низким приоритетом игнорируются
		//std::vector<IMessage> list; // исключили из общего интерфейса, чтобы допустить реализации,
			// которые пишут напрямую в цель

	public:
		ILogger() : _priority(MessagePriority::Info) {}
		virtual ~ILogger() {}
		
		virtual void Write(DetailedMessage message) = 0; // добавить сообщение
		virtual int Flush() = 0;	// запись результатов логгирования в искомый поток... искомую цель.
			// - запускать в отдельном потоке?

		MessagePriority GetPriority() {return _priority; } // возвращает текущий уровень приоритета.
		virtual int SetPriority(MessagePriority newPriority) // изменяет уровень приоритета.
		{ 
			_priority = newPriority;
			return 0;	// согласно текущей идее, возвращать мы должны количество сообщений,
				// не подходящих по установленному приоритету, но уже находящихся в списке
		}
			// виртуальный, потому что мы можем захотеть изменить поведение, 
			// например, удалять из буфера сообщения с более низким приоритетом

		//void Format(std::string format);	// установить "вид" сообщения для вывода

		//int Check();	// проверяет все сообщения на соответствие заданному приоритету и удаляет лишние - нужна ли? 
	};

	// берём интерфейс и дополняем его буфером сообщений и связью с "реальностью"
	// добавить промежуточный класс BufferedLogger ?
	class BufferedStreamLogger : public ILogger
	{
		std::list<DetailedMessage> _list;	// IMessage
		std::ostream* _target;
	public:
		BufferedStreamLogger(std::ostream* target) : _target(target) {}

		~BufferedStreamLogger()
		{
			Flush();	// перед завершением работы скидываем в лог всё, что накопилось
		}

		void Write(DetailedMessage message) override
		{
			// разрешаем запись сообщений только с равным или более высоким приоритетом
			if (message.getPriority() >= this->_priority)
			{
				_list.push_back(message);
			}
		}


		// "сбрасываем" буфер, записывая его содержимое в целевой поток
		int Flush() override
		{	
			for (auto message : _list)
			{
				auto curTime = message.getTime();
				auto ltm = localtime(&curTime);

				// вывод времени в "человеческом" виде
				*_target << ltm->tm_year + 1900 << "." 
						 << ltm->tm_mon + 1 << "."
						 << ltm->tm_mday << " "
						 << ltm->tm_hour + 1 << ":"
						 << ltm->tm_min + 1 << ":"
						 << ltm->tm_sec + 1 << "\t";

				// если мы определяем буффер как список IMessage, то мы не получим доступа к полям DetailedMessage,
				// и таким образом должны определять формат вывода в рамках подклассов IMessage
				*_target << PriorityToString(message.getPriority()) << '\t'
						 << message.getBody() << '\t'
						 << message.getFile() << '\t'						 
						 << message.getLine() << std::endl;

				// кастомизация формата вывода за счёт настроек класса?
			}

			_list.clear();	// очищаем список

			return 0;	// возвращать количество записанных сообщений?
		}
	};

	// отличается от BufferedStreamLogger тем, что в качестве параметра будет принимать
	// не поток, а имя файла. Открывать и закрывать файл логгер будет на Flush, чтобы всё
	// остальное время файл был свободен
	class BufferedFileLogger : public ILogger
	{
		std::list<DetailedMessage> _list;	// IMessage
		std::string _fileName;
	public:
		BufferedFileLogger(std::string fileName) : _fileName(fileName)
		{
			// очищаем файл вывода
			std::ofstream f;
			f.open(fileName);
			f.close();
		}

		~BufferedFileLogger()
		{
			Flush();
			// закрываем файл
		}

		void Write(DetailedMessage message) override
		{
			// разрешаем запись сообщений только с равным или более высоким приоритетом
			if (message.getPriority() >= this->_priority)
			{
				_list.push_back(message);
			}
		}


		// "сбрасываем" буфер, записывая его содержимое в целевой поток
		int Flush() override
		{
			// открываем файл
			std::ofstream out;
			out.open(_fileName, std::ios_base::app);

			for (auto message : _list)
			{
				auto curTime = message.getTime();
				auto ltm = localtime(&curTime);

				// вывод времени в "человеческом" виде
				out << ltm->tm_year + 1900 << "." 
						 << ltm->tm_mon + 1 << "."
						 << ltm->tm_mday << " "
						 << ltm->tm_hour + 1 << ":"
						 << ltm->tm_min + 1 << ":"
						 << ltm->tm_sec + 1 << "\t";

				// если мы определяем буффер как список IMessage, то мы не получим доступа к полям DetailedMessage,
				// и таким образом должны определять формат вывода в рамках подклассов IMessage
				out << PriorityToString(message.getPriority()) << '\t'
						 << message.getBody() << '\t'
						 << message.getFile() << '\t'						 
						 << message.getLine() << std::endl;

				// кастомизация формата вывода за счёт настроек класса?
			}

			_list.clear();

			// закрываем файл
			out.close();

			return 0;	// возвращать количество записанных сообщений?
		}
	};
}

#endif