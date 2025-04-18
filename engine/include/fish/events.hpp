#pragma once

#include "fish/property.hpp"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
namespace fish
{
    struct EventData : public IPropertyHolder
    {};

    typedef EventData EventObserver(EventData& data);

    class EventDispatcher 
    {
    public:
        void observe(const std::string& name, const std::function<EventObserver>& observer);
        EventData dispatch(const std::string& name, const EventData& data = {});
    private:
        std::unordered_map<std::string, std::vector<std::function<EventObserver>>> observers;
    };
}