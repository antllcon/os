#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <zlib.h>

constexpr uint64_t MAGIC_SIGNATURE = 0x0000007A69677573;
constexpr int SIGNAL_EXIT_ADDITION = 128;
constexpr int VALID_PACKER_ARGS = 3;
extern char **environ;

#pragma pack(push, 1)
struct Payload
{
	uint64_t compressedSize;
	uint64_t originalSize;
	uint64_t magicSignature;
};
#pragma pack(pop)

std::string gTempFilePath;
void CleanUp()
{
	if (!gTempFilePath.empty())
	{
		unlink(gTempFilePath.c_str());
		gTempFilePath.clear();
	}
}

void SignalHandler(const int signalNumber)
{
	std::cerr << "Recived signal " << signalNumber << ", clean up" << std::endl;
	CleanUp();
	_exit(SIGNAL_EXIT_ADDITION + signalNumber);
}

void AssertIsValidArgs(const int argc, const int expected)
{
	if (argc != expected)
	{
		throw std::invalid_argument("Invalid number of arguments");
	}
}

void AssertIsFileOpen(const std::ifstream& file, const std::string& filename)
{
	if (!file)
	{
		throw std::runtime_error("Couldn't open file: " + filename);
	}
}

std::string GetOwnPath()
{
	char bufferPath[PATH_MAX];
	if (readlink("/proc/self/exe", bufferPath, sizeof(bufferPath) - 1) == -1)
	{
		throw std::runtime_error("Couldn't get the path to the exe-file");
	}
	std::cout << bufferPath << std::endl;
	return std::string(bufferPath);
}

std::vector<char> ReadFileToBuffer(const std::string& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	AssertIsFileOpen(file, path);

	const auto size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	if (!file.read(buffer.data(), size))
	{
		throw std::runtime_error("Error reading file: " + path);
	}

	return buffer;
}

std::vector<char> CompressData(const std::vector<char>& data)
{
	if (data.empty())
	{
		return {};
	}

	auto compressedSize = compressBound(data.size());
	std::vector<char> compressedBuffer(compressedSize);

	if (compress(reinterpret_cast<Bytef*>(compressedBuffer.data()), &compressedSize, reinterpret_cast<const Bytef*>(data.data()), data.size()) != Z_OK)
	{
		throw std::runtime_error("ZLIB: data compression failed");
	}

	compressedBuffer.resize(compressedSize);
	return compressedBuffer;
}

std::vector<char> DecompressData(const std::vector<char>& compressedData, const uint64_t originalSize)
{
	if (compressedData.empty()) return {};

	std::vector<char> decompressedBuffer(originalSize);
	auto destLen = originalSize;

	if (uncompress(reinterpret_cast<Bytef*>(decompressedBuffer.data()), &destLen, reinterpret_cast<const Bytef*>(compressedData.data()), compressedData.size()) != Z_OK || destLen != originalSize)
	{
		throw std::runtime_error("Problem with upacking data (zlib)");
	}

	return decompressedBuffer;
}

void PackerMode(const std::string& selfPath, const std::string& inputPath, const std::string& outputPath)
{
	std::cout << "[==] Paker mode [==]" << std::endl;

	std::cout << "Compress file: " << inputPath << std::endl;

	std::vector<char> originalData = ReadFileToBuffer(inputPath);
	std::vector<char> compressedData = CompressData(originalData);

	Payload payload;
	payload.originalSize = originalData.size();
	payload.compressedSize = compressedData.size();
	payload.magicSignature = MAGIC_SIGNATURE;

	std::cout << "Create self file execution: " << outputPath << std::endl;
	std::ifstream src(selfPath, std::ios::binary);
	std::ofstream dst(outputPath, std::ios::binary);
	if (!src || !dst)
	{
		throw std::runtime_error("Error opening self or output file");
	}

	dst << src.rdbuf();
	if (!dst)
	{
		throw std::runtime_error("Error writing to output file");
	}

	src.close();

	dst.write(compressedData.data(), compressedData.size());
	dst.write(reinterpret_cast<const char*>(&payload), sizeof(payload));
	dst.close();

	chmod(outputPath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}

void UnpackerMode(const std::string& selfPath, const Payload& payload, char* argv[])
{
	std::ifstream selfFile(selfPath, std::ios::binary);
	selfFile.seekg(-static_cast<std::streamoff>(sizeof(Payload) + payload.compressedSize), std::ios::end);

	std::vector<char> compressedData(payload.compressedSize);
	selfFile.read(compressedData.data(), payload.compressedSize);
	selfFile.close();

	std::vector<char> originalData = DecompressData(compressedData, payload.originalSize);

	char tempTemplate[] = "/tmp/zigus-XXXXXX";

	int fd = mkstemp(tempTemplate);
	if (fd == -1)
	{
		throw std::runtime_error("Dont create temp file");
	}

	std::string tempPath = tempTemplate;

	write(fd, originalData.data(), originalData.size());
	chmod(tempPath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
	close(fd);

	pid_t pid = fork();
	if (pid < 0)
	{
		unlink(tempPath.c_str());
		throw std::runtime_error("Error fork()");
	}

	if (pid == 0)
	{
		execve(tempPath.c_str(), &argv[0], environ);
		perror("Error execve");
		_exit(SIGNAL_EXIT_ADDITION - 1);
	}

	if (pid > 0)
	{
		int status;
		waitpid(pid, &status, 0);
	}

	unlink(tempPath.c_str());
}

int main(const int argc, char* argv[])
{
	atexit(CleanUp);
	signal(SIGINT, SignalHandler);
	signal(SIGTERM, SignalHandler);

	try
	{
		const std::string selfPath = GetOwnPath();
		std::ifstream selfFile(selfPath, std::ios::binary);

		AssertIsFileOpen(selfFile, selfPath);

		Payload payload;
		selfFile.seekg(-static_cast<std::streamoff>(sizeof(Payload)), std::ios::end);
		selfFile.read(reinterpret_cast<char*>(&payload), sizeof(Payload));

		if (selfFile && payload.magicSignature == MAGIC_SIGNATURE)
		{
			UnpackerMode(selfPath, payload, argv);
		}
		else
		{
			AssertIsValidArgs(argc, VALID_PACKER_ARGS);
			PackerMode(selfPath, argv[1], argv[2]);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}