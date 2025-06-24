#include <iostream>
#include "App.h"

int main(int argc, char* args[])
{
    try {
        App app;

        app.InitSDL();

        app.OnCreate();

        app.Render();
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << "\n";
    }

    return 0;
}

