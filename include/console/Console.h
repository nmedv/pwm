#ifndef CONSOLE_H
#define	CONSOLE_H

#include <console/MemoryManager.h>

struct console_data;

class Console
{
public:
	Console(void);
	Console(MemoryManager* mm);
	~Console();

	/// Get argv with UTF-8 encoding
	const char** GetUTF8Argv(void);

	int Show(const char* title);

	int Terminate(void);

	int Read(char* data, int dataSize);

	int Write(const char* data);

	int Printf(const char* format, ...);
	
	void SetStdinEcho(bool enable);

private:
	MemoryManager* mmgr;
	console_data* conData;

	void Init(void);
};

#endif // !CONSOLE_H