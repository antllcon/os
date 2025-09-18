#include <fstream>
#include <sstream>
#include <vector>

struct ProcessInfo
{
	pid_t pid;
	std::string name;
	std::string user;
	long privateMemory = 0;
	long sharedMemory = 0;
	long totalMemory = 0;
};

inline long GetValueFromProcFile(const std::string& path, const std::string& key)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		return 0;
	}

	std::string line;
	while (std::getline(file, line))
	{
		if (line.rfind(key, 0) == 0)
		{
			std::stringstream stream(line);
			std::string temp;
			long value = 0;
			stream >> temp >> value;
			return value;
		}
	}

	return 0;
}

int main()
{
	std::vector<ProcessInfo> processes;



	return EXIT_SUCCESS;
}