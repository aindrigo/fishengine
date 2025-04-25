#pragma once


namespace fish
{
    class ISystem
    {
    public:
        virtual ~ISystem() = default;

        virtual void init() {};
        virtual void update() {};
        virtual void preRender() {};
        virtual void render() {};
        virtual void postRender() {};
        virtual void tick() {};
        virtual void shutdown() {};
    };
}