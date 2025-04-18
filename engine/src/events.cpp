#include "fish/events.hpp"
#include <format>
#include <functional>
#include <iostream>
#include <string>
#include "cpptrace/from_current.hpp"
namespace fish
{
    void EventDispatcher::observe(const std::string& name, const std::function<EventObserver>& observer)
    {
        if (!this->observers.contains(name))
            this->observers[name] = {};

        this->observers[name].emplace_back(observer);
    }

    EventData EventDispatcher::dispatch(const std::string& name, const EventData& data)
    {
        if (!this->observers.contains(name))
            return data;

        EventData result = data;
        for (auto const& observer : this->observers[name]) {
            CPPTRACE_TRY {
                result = observer(result);
            } CPPTRACE_CATCH (std::exception& e) {
                std::cout << std::format("Caught an exception dispatching event {}, continuing anyway", name) << std::endl;
                cpptrace::from_current_exception().print();
            }
        }

        return result;
    }
}