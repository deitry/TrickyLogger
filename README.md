# TrickyLogger
Little helper for log writing

Inspired by EasyLogging

Single header class Log with variety of behaviour Logger classes.
All of them made in singletone way to secure that there is only one Log at a time.
Buffered versions of loggers, as defined in the IBufferedLogger, don't write anything in the output
until they are forcedly flushed or being destroyed (and therefore flushed automatically).

It supports priorities: Debug, Info, Warning, Error, Fatal. They are united under MessagePriority enum.
Messages with lesser priority are not written in the output.

- - - - - -

To start, just add before the main:
- 'TLOG_MAIN_COUT()' - if you are ok with writing log right in your stdout
- or 'TLOG_MAIN(BufferedFileLogger::Init("debug.log"))' - if you want to write log in "debug.log"

To put some message in the log, insert:
- TLOG("Hello, world", Info);

Note that default priority of Log is Info. That means, all messages with "Debug" priority are ignored and not put in the output.

It has minimal interface and customizing options, but maybe gets wider in the future.
