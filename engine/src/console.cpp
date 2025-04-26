#include "fish/console.hpp"
#include "fish/common.hpp"
#include "fish/engineinfo.hpp"
#include "fish/events.hpp"
#include "fish/helpers.hpp"
#include "fish/services.hpp"
#include <cpptrace/from_current.hpp>
#include <exception>
#include <format>
#include <functional>
#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

namespace fish
{
    Console::Console(Services& services)
        : services(services)
    {}

    Console::Console(const Console& other)
        : running(false), services(other.services), commands(other.commands)
    {}

    void Console::init()
    {
        auto& engineInfo = services.getService<EngineInfo>();
        this->running = true;
        if (engineInfo.runType == EngineRunType::HEADLESS) {
            this->readThread = std::thread([this] { this->readThreadFunction(); });
            this->readThread.detach();
        }
    }

    void Console::update()
    {
        auto& engineInfo = services.getService<EngineInfo>();
        {
            std::lock_guard<std::mutex> g(this->queuedCommandMutex);
            if (engineInfo.runType == EngineRunType::HEADLESS && this->queuedCommand.has_value()) {
                auto command = this->queuedCommand.value();

                this->runCommand(command.commandName, command.arguments);
                this->queuedCommand.reset();
            }
        }
    }
    
    void Console::shutdown()
    {
        this->running = false;
    }
    
    bool Console::exists(const std::string& name)
    {
        return this->commands.contains(name);
    }

    void Console::registerCommand(const std::string& name, const std::function<ConsoleCommand>& command)
    {
        FISH_ASSERTF(!this->exists(name), "Command {} already exists", name);
        this->commands[name] = command;
    }

    ConsoleCommandResult Console::runCommand(const std::string& name, const std::set<std::string>& arguments, bool log)
    {
        ConsoleCommandResult result;

        if (!this->exists(name)) {
            result.success = false;
            result.message = std::format("Command {} does not exist", name);
            this->log(result.message);
            return result;
        }

        CPPTRACE_TRY {
            this->commands[name](arguments);
        } CPPTRACE_CATCH(std::exception& e) {
            result.success = false;
            result.message = std::format("An error was caught processing command {}: {}", name, e.what());
            this->log(result.message);
#ifndef NDEBUG
            cpptrace::from_current_exception().print();
#endif
            return result;
        }

        result.success = true;
        return result;
    }

    void Console::log(const std::string& message) 
    {
        auto& events = this->services.getService<EventDispatcher>();

        if (this->logs.size() >= 256)
            this->logs.erase(this->logs.begin());

        this->logs.push_back(message);

        EventData event;
        event.setProperty("log", message);
        events.dispatch("consoleLog", event);
    }

    const std::vector<std::string>& Console::getLogs()
    {
        return this->logs;
    }

    void Console::readThreadFunction()
    {
        while (this->running) {
            {
                std::lock_guard<std::mutex> g(this->queuedCommandMutex);
                if (this->queuedCommand.has_value())
                    continue;
            }

            std::string line;
            std::cout << "> ";
            std::getline(std::cin, line);

            std::vector<std::string> words = helpers::String::splitString(line, " ");
            if (words.size() < 1)
                continue;

            std::string command = words.at(0);
            words.erase(words.begin());

            {
                std::lock_guard<std::mutex> g(this->queuedCommandMutex);
                this->queuedCommand = QueuedCommand {
                    .commandName = command,
                    .arguments = std::set<std::string>(words.begin(), words.end())
                };
            }
        }
    }
}