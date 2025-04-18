#pragma once

namespace fish
{
    struct Texture2DData {
        unsigned char* pixels;
        unsigned int width;
        unsigned int height;
    };

    class Texture2D
    {
    public:
        Texture2D(unsigned int id);
    };
}