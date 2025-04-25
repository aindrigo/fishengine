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
    typedef void ImmutableEventObserver(EventData const& data);
    typedef EventData MultiEventObserver(const std::string& name, EventData& data);

    class EventDispatcher 
    {
    public:
        void observe(const std::string& name, const std::function<EventObserver>& observer);
        void observeImmutable(const std::string& name, const std::function<ImmutableEventObserver>& observer);
        void observeAll(const std::function<MultiEventObserver>& observer);
        EventData dispatch(const std::string& name, const EventData& data = {});
    private:
        std::unordered_map<std::string, std::vector<std::function<EventObserver>>> observers;
        std::unordered_map<std::string, std::vector<std::function<ImmutableEventObserver>>> immutableObservers;
        std::vector<std::function<MultiEventObserver>> multiObservers;
    };
}