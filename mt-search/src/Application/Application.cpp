#include "Application.h"
#include "../CommandParser/CommandParser.h"
#include "../ScopeTimer/ScopeTimer.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <future>

namespace
{
void PrintError(std::string_view message) noexcept
{
    std::cerr << message << std::endl;
}

bool ValidateArgsCount(const std::vector<std::string>& args, size_t expectedCount)
{
    if (args.size() != expectedCount)
    {
       PrintError("Wrong number of arguments");
       return false;
    }

    return true;
}

void AssertArgsNumbersValid(const std::vector<std::string>& args)
{
    if (args.empty())
    {
       PrintError("empty query");
    }
}

std::string JoinStrings(const std::vector<std::string>& args)
{
    if (args.empty())
    {
       return "";
    }

    std::string result;
    size_t totalLength = 0;
    for (const auto& str : args)
    {
       totalLength += str.length() + 1;
    }

    result.reserve(totalLength);

    for (const auto& arg : args)
    {
       result.append(arg);
       result.append(" ");
    }

    result.pop_back();
    return result;
}
} // namespace

Application::Application(unsigned numThreads)
    : m_threadPool(numThreads)
    , m_service(m_index, m_threadPool, m_fileSystem)
{
}

void Application::Run()
{
    std::string line;
    while (m_isRunning.load())
    {
       std::cout << "> ";
       if (!std::getline(std::cin, line))
       {
          break;
       }

       ProcessCommand(line);
    }
}

void Application::ProcessCommand(const std::string& line)
{
    const auto cmd = command_parser::Parse(line);

    switch (cmd.type)
    {
    case command_parser::CommandType::Empty:
       break;

    case command_parser::CommandType::AddFile:
       HandleAddFile(cmd.arguments);
       break;

    case command_parser::CommandType::AddDir:
       HandleAddDir(cmd.arguments, false);
       break;

    case command_parser::CommandType::AddDirRecursive:
       HandleAddDir(cmd.arguments, true);
       break;

    case command_parser::CommandType::RemoveFile:
       HandleRemoveFile(cmd.arguments);
       break;

    case command_parser::CommandType::RemoveDir:
       HandleRemoveDir(cmd.arguments, false);
       break;

    case command_parser::CommandType::RemoveDirRecursive:
       HandleRemoveDir(cmd.arguments, true);
       break;

    case command_parser::CommandType::Find:
       HandleFind(cmd.arguments);
       break;

    case command_parser::CommandType::FindBatch:
       HandleFindBatch(cmd.arguments);
       break;

    case command_parser::CommandType::Unknown:
       PrintError("Unknown command");
       break;
    }
}

void Application::HandleAddFile(const std::vector<std::string>& args)
{
    if (!ValidateArgsCount(args, 1)) return;

    const std::string& pathStr = args[0];
    std::cout << "Indexing " << pathStr << "..." << std::endl;
    try
    {
        std::future<void> future = m_service.AddFileAsync(Path(pathStr));

        future.get();

        std::cout << "Successfully indexed " << pathStr << std::endl;
    }
    catch (const std::exception& e)
    {
        PrintError(std::string("Failed to index ") + pathStr + ": " + e.what());
    }
}

void Application::HandleAddDir(const std::vector<std::string>& args, bool recursive)
{
    if (!ValidateArgsCount(args, 1)) return;

    const std::string& pathStr = args[0];
    const std::string recStr = recursive ? " recursively..." : "...";
    std::cout << "Indexing directory " << pathStr << recStr << std::endl;
    try
    {
        std::future<void> future = m_service.AddDirectoryAsync(Path(pathStr), recursive);

        future.get();

        std::cout << "Successfully indexed directory " << pathStr << std::endl;
    }
    catch (const std::exception& e)
    {
        PrintError(std::string("Failed to index directory ") + pathStr + ": " + e.what());
    }
}

void Application::HandleRemoveFile(const std::vector<std::string>& args)
{
    if (!ValidateArgsCount(args, 1)) return;

    try
    {
        m_service.RemoveFileAsync(Path(args[0])).get();
    }
    catch (const std::exception& e)
    {
         PrintError(std::string("Failed to remove file: ") + e.what());
    }
}

void Application::HandleRemoveDir(const std::vector<std::string>& args, bool recursive)
{
    if (!ValidateArgsCount(args, 1)) return;

    try
    {
        m_service.RemoveDirectoryAsync(Path(args[0]), recursive).get();
    }
    catch (const std::exception& e)
    {
         PrintError(std::string("Failed to remove directory: ") + e.what());
    }
}

void Application::HandleFind(const std::vector<std::string>& args)
{
    ScopeTimer time("Search took");
    const auto query = JoinStrings(args);
    const auto results = m_service.Find(query);
    PrintResults(results);
}

void Application::HandleFindBatch(const std::vector<std::string>& args)
{
    if (!ValidateArgsCount(args, 1)) return;

    auto onResultCallback = [this](
                         size_t queryId,
                         std::string_view query,
                         const std::vector<SearchResult>& results) {
       PrintBatchResult(queryId, query, results);
    };

    m_service.FindBatchAsync(Path(args[0]), onResultCallback);
}

void Application::PrintResults(const std::vector<SearchResult>& results) noexcept
{
    size_t count = 1;
    for (const auto& result : results)
    {
       std::cout << count++ << ". "
               << "id:" << result.documentId << ", "
               << "relevance:" << std::fixed << std::setprecision(5) << result.relevance << ", "
               << "path:" << result.path.string()
               << std::endl;
    }
    std::cout << "---" << std::endl;
}

void Application::PrintBatchResult(size_t queryId, std::string_view query, const std::vector<SearchResult>& results) noexcept
{
    static std::mutex coutMutex;
    const std::lock_guard<std::mutex> lock(coutMutex);

    std::cout << queryId << ". query: " << query << std::endl;
    PrintResults(results);
}