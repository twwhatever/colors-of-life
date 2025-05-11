#include <SFML/Graphics.hpp>
#include <ruby.h>
#include <iostream>

int main() {
    // Initialize Ruby VM
    ruby_init();
    ruby_init_loadpath();
    rb_eval_string("puts 'Hello from embedded Ruby!'");
    ruby_finalize();

    // Open SFML window
    sf::RenderWindow window(sf::VideoMode(400, 400), "Ruby + SFML");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);
        window.display();
    }

    return 0;
}
