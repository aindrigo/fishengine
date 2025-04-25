#pragma once

#include "fish/services.hpp"
#include <atomic>
#include <functional>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
namespace fish
{
    typedef void ConsoleCommand(const std::set<std::string>& args);

    struct ConsoleCommandResult
    {
        bool success;
        std::string message;   
    };

    class Console
    {
    public:
        Console(Services& services);
        Console(const Console& other);
        void init();
        void update();
        void shutdown();

        bool exists(const std::string& name);
        void registerCommand(const std::string& name, const std::function<ConsoleCommand>& command);
        ConsoleCommandResult runCommand(const std::string& name, const std::set<std::string>& args);
    private:
        struct QueuedCommand {
            std::string commandName;
            std::set<std::string> arguments;
        };

        void readThreadFunction();

        std::atomic_bool running;
        std::mutex queuedCommandMutex;
        std::optional<QueuedCommand> queuedCommand;
        Services& services;
        std::thread readThread;
        std::unordered_map<std::string, std::function<ConsoleCommand>> commands;
    };
}