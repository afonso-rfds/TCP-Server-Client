#include "LogFile.h"



LogFile::LogFile()
{
	CreateLogFile("Log.txt");
	m_file.close();
	m_fileName = "Log.txt";
}


LogFile::~LogFile()
{
}


void LogFile::WriteString(std::string stringToWrite)
{
	m_file.open(m_fileName, std::ios_base::app);
	m_file << stringToWrite << std::endl;
	m_file.close();
}

void LogFile::CreateLogFile(std::string fileName)
{
	m_file = std::ofstream(fileName);

	return;
}
