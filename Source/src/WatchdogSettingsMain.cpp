#include <gtkmm.h>
#include <iostream>

int main(int argc, char* argv[]) {
    auto application = Gtk::Application::create(argc, argv);
    Gtk::Window window;
    return application->run(window);
}