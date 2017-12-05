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

// объявления перед main - нужны для определения статической переменной Log
// стандартный вывод в cout
#define TLOG_MAIN_COUT() \
ILogger* TrickyLogger::Log::current = StreamLogger::Init(&std::cout);

// подставить сюда объект любого класса логгера
#define TLOG_MAIN(logger) \
ILogger* TrickyLogger::Log::current = (logger);

// функции ... макросы по управлению логом
// инициализация лога 
#define TLOG_INIT(logger) \
TrickyLogger::Log::Init((logger));

// установка приоритета
#define TLOG_PRIOR(prior) \
TrickyLogger::Log::SetPriority((prior));

// запись в лог с указанным приоритетом
#define TLOG(message, prior) \
TrickyLogger::Log::Write(TrickyLogger::DetailedMessage((message),(prior), __FILE__,__LINE__));

// оборачиваем Flush(), чтобы ко всем функциям можно было обращаться одинаковыми способами
#define TLOG_FLUSH() \
TrickyLogger::Log::Flush();

	// Источники вдохновения:
	// stack tracer https://habrahabr.ru/post/302170/
	// easy logging https://habrahabr.ru/post/225363/
	// https://github.com/muflihun/easyloggingpp/tree/master/src

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
			return "Error!";
		case Fatal:
			return "FATAL";
		default: 
			return "Unknown message type";
		}
	}

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

	// сообщение с дополнительной информацией.
	// После того, как решили вынести создание строки непосредственно в логгер, теряется смысл в выделении просто IMessage:
	// - все сообщения в актуальных реалиациях ILogger объявлены Detailed
	class DetailedMessage : public IMessage
	{
		time_t _t;	// https://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-cs
		std::string _file;
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

	// собственно, логгер
	class ILogger
	{
	protected:
		MessagePriority _priority;	// разрешённый приоритет сообщений - с более низким приоритетом игнорируются
		
		// подклассы будут построены по принципу синглтона, так что прячем.
		ILogger(MessagePriority prior = MessagePriority::Info) : _priority(prior) {}
		virtual ~ILogger() {}	

	public:
		
		virtual void Write(DetailedMessage message) = 0; // добавить сообщение
		virtual int Flush() { return 0; };	// запись результатов логгирования в искомый поток... искомую цель.
			// Вообще говоря, метод нужен только объектам с буферизацией в том или ином виде. Но поскольку их
			// довольно-таки много, а мы хотим унифицировать доступ к подклассам, включаем метод в базовый интерфейс

		MessagePriority GetPriority() {return _priority; } // возвращает текущий уровень приоритета.
		virtual int SetPriority(MessagePriority newPriority) // изменяет уровень приоритета.
		{ 
			_priority = newPriority;
			return 0;	// согласно текущей идее, возвращать мы должны количество сообщений,
				// не подходящих по установленному приоритету, но уже находящихся в буфере
		}
			// метод SetPriority виртуальный, потому что мы можем захотеть изменить поведение, 
			// например, удалять из буфера сообщения с более низким приоритетом

		//void Format(std::string format);	// ? установить "вид" сообщения для вывода
	};

	// берём интерфейс и дополняем его буфером сообщений и связью с "реальностью"
	// добавить промежуточный класс BufferedLogger ?
	class StreamLogger : public ILogger
	{
	protected:
		std::ostream* _target;

		StreamLogger(std::ostream* target) : _target(target) {}
		~StreamLogger() {}

	public:

		static StreamLogger* Init(std::ostream* target)
		{
			static StreamLogger logger(target);
			// если хотим инициализировать другим потоком
			if (target != logger._target) logger._target = target;

			return &logger;
		}

		// "сбрасываем" буфер, записывая его содержимое в целевой поток
		void Write(DetailedMessage message) override
		{	
			// если так получилось, что в список затесались сообщения с более низким приоритетом,
			// чем нам хотелось бы - пропускаем
			if (message.getPriority() < this->_priority) return;

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
		}
	};

	class IBufferedLogger : public ILogger
	{
	protected:
		std::list<DetailedMessage> _list;	// IMessage

		virtual void WriteMessage(DetailedMessage message) = 0; 
			// собственно, запись сообщения куда бы то ни было

	public:
		void Write(DetailedMessage message) override
		{
			// разрешаем запись сообщений только с равным или более высоким приоритетом
			if (message.getPriority() >= this->_priority)
			{
				_list.push_back(message);
			}
		}

		int Flush() override
		{	
			for (auto message : _list)
			{				
				WriteMessage(message);
			}

			_list.clear();	// очищаем список

			return 0;	// возвращать количество записанных сообщений?
		}

	};

	// берём интерфейс и дополняем его буфером сообщений и связью с "реальностью"
	// добавить промежуточный класс BufferedLogger ?
	class BufferedStreamLogger : public IBufferedLogger
	{
	protected:
		std::ostream* _target;

		BufferedStreamLogger(std::ostream* target) : _target(target) {}
		~BufferedStreamLogger()
		{
			Flush();	// перед завершением работы скидываем в лог всё, что накопилось
		}

		// "сбрасываем" буфер, записывая его содержимое в целевой поток
		void WriteMessage(DetailedMessage message) override
		{	
			// если так получилось, что в список затесались сообщения с более низким приоритетом,
			// чем нам хотелось бы - пропускаем
			if (message.getPriority() < this->_priority) return;

			auto curTime = message.getTime();
			auto ltm = localtime(&curTime);

			// вывод времени в "человеческом" виде
			*_target << ltm->tm_year + 1900 << "."; 
			_target->width(2); _target->fill('0'); *_target << ltm->tm_mon + 1 << ".";
			_target->width(2); _target->fill('0'); *_target << ltm->tm_mday << " ";
			_target->width(2); _target->fill('0'); *_target << ltm->tm_hour + 1 << ":";
			_target->width(2); _target->fill('0'); *_target << ltm->tm_min + 1 << ":";
			_target->width(2); _target->fill('0'); *_target << ltm->tm_sec + 1 << "\t";

			// если мы определяем буффер как список IMessage, то мы не получим доступа к полям DetailedMessage,
			// и таким образом должны определять формат вывода в рамках подклассов IMessage
			*_target << PriorityToString(message.getPriority()) << '\t'
						<< message.getBody() << '\t'
						<< message.getFile() << '\t'						 
						<< message.getLine() << std::endl;
		}

	public:

		static BufferedStreamLogger* Init(std::ostream* target)
		{
			static BufferedStreamLogger logger(target);
			// если хотим инициализировать другим потоком
			if (target != logger._target)
			{
				logger.Flush();	// сбрасываем, всё что накопилось
				logger._target = target;
			}
			return &logger;
		}
	};

	// отличается от BufferedStreamLogger тем, что в качестве параметра будет принимать
	// не поток, а имя файла. Открывать и закрывать файл логгер будет на Flush, чтобы всё
	// остальное время файл был свободен/
	// Поведение, при котором файл открывается в конструкторе, а закрывается в деструкторе
	// оставим потенциальному подклассу BufferedStreamFileLogger : BufferedStreamLogger
	class BufferedFileLogger : public BufferedStreamLogger
	{
	protected:
		std::string _fileName;
		
		BufferedFileLogger(std::string fileName) :
			BufferedStreamLogger(nullptr), _fileName(fileName)
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

	public:

		// Гипотетически, можем разрешить публичный конструктор,
		// чтобы обеспечить ведение логов в разных файлах.
		// Однако, поскольку Log сейчас всего один, оставляем так
		static BufferedFileLogger* Init(std::string fileName)
		{
			static BufferedFileLogger logger(fileName);
			
			// хитрая хитрость - переключаемся на новый объект, если передано другое имя.
			// Вообще конечно, можем просто запретить изменять цель вывода после инициализации 
			if (fileName != logger._fileName)
			{
				logger.Flush(); // сбрасываем, всё что накопилось
				logger = BufferedFileLogger(fileName);
			}
			return &logger;
		}

		// "сбрасываем" буфер, записывая его содержимое в целевой поток
		int Flush() override
		{

			// открываем файл
			std::ofstream out;
			out.open(_fileName, std::ios_base::app);

			this->_target = &out;

			int result = IBufferedLogger::Flush();

			// закрываем файл
			out.close();

			this->_target = nullptr;

			return result;	// возвращать количество записанных сообщений?
		}
	};

	// обёртка вокруг логгера - позволяет обращаться к виртуальным методам как к статичным.
	// По сути, аналогичного эффекта можно было достичь просто введя глобальную переменную.
	// Но класс в перспективе позволяет достичь большей гибкости
	// - например, при желании/необходимости можем запретить изменять назначение лога после инициализации
	// Терминология. "Лог" - определяет объект, а "логгер" - поведение
	class Log
	{
	private:
		static ILogger* current;

	public:
		static void Init(ILogger* logger)
		{
			current = logger;
		}

		static void Write(DetailedMessage message) { if (current) return current->Write(message); }
		static int Flush() { if (current) return current->Flush(); return 0; }
		static MessagePriority GetPriority() {return current->GetPriority(); } // возвращает текущий уровень приоритета.
		static int SetPriority(MessagePriority newPriority) // изменяет уровень приоритета.
		{ 
			return current->SetPriority(newPriority);
		}
	};
}

#endif