#include <SFML/Graphics.hpp>
#include <iostream>

using namespace sf;
using namespace std;

const int NORMAL_FPS = 30;

const int MIN = 0;

const int CELLS_WIDTH = 75;
const int CELLS_HEIGHT = 75;

const int RESOLUTION_WIDTH = (750 - (750 % CELLS_WIDTH));
const int RESOLUTION_HEIGHT = (750 - (750 % CELLS_HEIGHT));

const int RATIO_WIDTH = RESOLUTION_WIDTH / CELLS_WIDTH;
const int RATIO_HEIGHT = RESOLUTION_HEIGHT / CELLS_HEIGHT;

bool running = false;
bool cells[CELLS_WIDTH][CELLS_HEIGHT];
bool oldCells[CELLS_WIDTH][CELLS_HEIGHT];

int ticks = 5;

float overlaySeconds = -1;
string overlayText;

bool isInRange(int x, int y) {
    return x > MIN && y > MIN && x < CELLS_WIDTH && y < CELLS_HEIGHT;
}

void updateTicks(RenderWindow *window) {
    window->setFramerateLimit(running ? ticks : NORMAL_FPS);
}

int getAliveNeighbors(int x, int y) {
    int aliveNeighbours = 0;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int w = x + i;
            int h = y + j;

            if ((w == x && h == y) || !isInRange(w, h)) {
                continue;
            }

            if (oldCells[w][h]) {
                aliveNeighbours++;
            }
        }
    }

    return aliveNeighbours;
}

void toggleCellAt(int x, int y, bool pixels = true) {
    x = pixels ? x / RATIO_WIDTH : x;
    y = pixels ? y / RATIO_HEIGHT : y;

    cells[x][y] = !cells[x][y];
}

void copyCells() {
    for (int x = 0; x < CELLS_WIDTH; x++) {
        for (int y = 0; y < CELLS_HEIGHT; y++) {
            oldCells[x][y] = cells[x][y];
        }
    }
}

void simulateCell(int x, int y, int neighbors) {
    if (oldCells[x][y]) {
        if (neighbors < 2 || neighbors > 3) {
            cells[x][y] = false;
        }
    } else if (neighbors == 3) {
        cells[x][y] = true;
    }
}

void simulate() {
    copyCells();

    for (int x = 0; x < CELLS_WIDTH; x++) {
        for (int y = 0; y < CELLS_HEIGHT; y++) {
            int aliveNeighbors = getAliveNeighbors(x, y);
            simulateCell(x, y, aliveNeighbors);
        }
    }
}

void draw(Image *image) {
    for (int x = 0; x < CELLS_WIDTH; x++) {
        for (int y = 0; y < CELLS_HEIGHT; y++) {
            image->setPixel(x, y, cells[x][y] ? Color::Black : Color::White);
        }
    }
}

void clearCells() {
    for (int x = 0; x < CELLS_WIDTH; x++) {
        for (int y = 0; y < CELLS_HEIGHT; y++) {
            cells[x][y] = false;
        }
    }
}

void randomCells() {
    srand(time(nullptr));

    // TODO
}

void displayText(RenderWindow *window, const string &text) {
    Font font;

    if (!font.loadFromFile("/usr/share/fonts/TTF/OpenSans-Regular.ttf")) {
        exit(1337);
    }

    Text str(text, font, 100);
    str.setFillColor(Color::Magenta);

    sf::FloatRect textRect = str.getLocalBounds();
    str.setOrigin(textRect.left + textRect.width / 2.0f,
                  textRect.top + textRect.height / 2.0f);
    str.setPosition(sf::Vector2f(window->getSize().x / 2.0f, window->getSize().y / 2.0f));

    window->draw(str);
}

void setOverlay(Clock *clock, const string &text, float seconds) {
    overlaySeconds = seconds;
    overlayText = text;

    clock->restart();
}

void handleOverlay(RenderWindow *window, Clock *clock) {
    if (overlaySeconds >= 0) {
        if (overlaySeconds > clock->getElapsedTime().asSeconds()) {
            displayText(window, overlayText);
        } else {
            overlaySeconds = -1;
        }
    }
}

int main() {
    RenderWindow window(sf::VideoMode(RESOLUTION_WIDTH, RESOLUTION_HEIGHT), "Conway's Game Of Life",
                        Style::Titlebar | Style::Close);
    Texture texture;
    texture.create(CELLS_WIDTH, CELLS_HEIGHT);
    Image image;
    image.create(CELLS_WIDTH, CELLS_HEIGHT, Color::Black);

    Sprite sprite;
    sprite.setScale(RATIO_WIDTH, RATIO_HEIGHT);

    Clock clock;

    while (window.isOpen()) {
        Event event;

        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            } else if (event.type == Event::MouseButtonPressed && !running) {
                if (event.mouseButton.button == Mouse::Button::Left) {
                    toggleCellAt(event.mouseButton.x, event.mouseButton.y);
                }
            } else if (event.type == Event::KeyReleased) {
                switch (event.key.code) {
                    case Keyboard::Key::Space:
                        running = !running;
                        updateTicks(&window);
                        setOverlay(&clock, running ? "RUN" : "PAUSE", 1);

                        break;
                    case Keyboard::Key::Left:
                        ticks--;
                        updateTicks(&window);
                        setOverlay(&clock, "SPEED +", 1);

                        break;
                    case Keyboard::Key::Right:
                        ticks++;
                        updateTicks(&window);
                        setOverlay(&clock, "SPEED -", 1);

                        break;
                    case Keyboard::Key::C:
                        clearCells();
                        setOverlay(&clock, "CLEAR", 1);

                        break;
                    case Keyboard::Key::R:
                        randomCells();

                        break;
                }
            }
        }

        window.clear();

        if (running) {
            simulate();
        }

        draw(&image);

        texture.update(image);
        sprite.setTexture(texture);

        window.draw(sprite);
        handleOverlay(&window, &clock);

        window.display();
    }

    return 0;
}
