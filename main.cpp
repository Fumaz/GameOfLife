#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <json/json.h>
#include <fstream>

using namespace sf;
using namespace std;

const int NORMAL_FPS = 30;

const int MIN = 0;

const int CELLS_WIDTH = 250;
const int CELLS_HEIGHT = 250;

const int RESOLUTION_WIDTH = (750 - (750 % CELLS_WIDTH));
const int RESOLUTION_HEIGHT = (750 - (750 % CELLS_HEIGHT));

const int RATIO_WIDTH = RESOLUTION_WIDTH / CELLS_WIDTH;
const int RATIO_HEIGHT = RESOLUTION_HEIGHT / CELLS_HEIGHT;

bool running = false;
bool cells[CELLS_WIDTH][CELLS_HEIGHT];
bool oldCells[CELLS_WIDTH][CELLS_HEIGHT];

Json::Value layouts;

int ticks = 5;

float overlaySeconds = -1;
string overlayText;

Font font;

void sleep(int milliseconds) {
    this_thread::sleep_for(chrono::milliseconds(milliseconds));
}

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
    if (x > RESOLUTION_WIDTH || y > RESOLUTION_HEIGHT) return;

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

void randomCellsAround(int x, int y, int radius = 5, int chance = 3, bool pixels = true) {
    if (x > RESOLUTION_WIDTH || y > RESOLUTION_HEIGHT) return;

    x = pixels ? x / RATIO_WIDTH : x;
    y = pixels ? y / RATIO_HEIGHT : y;

    for (int i = -radius; i <= radius; i++) {
        for (int k = -radius; k <= radius; k++) {
            int xi = x + i;
            int yk = y + k;

            if (!isInRange(xi, yk)) {
                continue;
            }

            cells[xi][yk] = (rand() % chance) < 1;
        }
    }
}

void placeLayoutAt(int x, int y, Json::Value layout, bool pixels = true) {
    if (x > RESOLUTION_WIDTH || y > RESOLUTION_HEIGHT) return;

    x = pixels ? x / RATIO_WIDTH : x;
    y = pixels ? y / RATIO_HEIGHT : y;

    for (int i = 0; i < layout.size(); i++) {
        for (int k = 0; k < layout[i].size(); k++) {
            int xi = x + i;
            int yk = y + k;

            if (!isInRange(xi, yk)) {
                continue;
            }

            cells[xi][yk] = layout[i][k].asBool();
        }
    }
}

Json::Value getRandomLayout() {
    return layouts[rand() % layouts.size()];
}

void loadFonts() {
    if (!font.loadFromFile("/usr/share/fonts/TTF/OpenSans-Regular.ttf")) {
        exit(1337);
    }
}

Text createText(const string &text, int size, Color color) {
    Text str = Text(text, font, size);
    str.setFillColor(color);
    return str;
}

void displayCenteredText(RenderWindow *window, const string &text) {
    Text str = createText(text, 100, Color::Magenta);

    FloatRect textRect = str.getLocalBounds();
    str.setOrigin(textRect.left + textRect.width / 2.0f,
                  textRect.top + textRect.height / 2.0f);
    str.setPosition(Vector2f(window->getSize().x / 2.0f, window->getSize().y / 2.0f));

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
            displayCenteredText(window, overlayText);
        } else {
            overlaySeconds = -1;
        }
    }
}

void drawHints(RenderWindow *window) {
    Text hintsText = createText(
            string("Space = ") + (running ? "Pause" : "Run") +
            "\tWheel = Adjust Speed\tR = Speed Reset\tC = Clear", 15,
            Color::White);

    FloatRect rect = hintsText.getLocalBounds();
    hintsText.setOrigin(rect.left + rect.width / 2.0f, 0);
    hintsText.setPosition(window->getSize().x / 2.0f, RESOLUTION_HEIGHT);
    window->draw(hintsText);
}

void loadLayouts() {
    ifstream jsonFile("layouts.json", ifstream::binary);

    jsonFile >> layouts;
}

int main() {
    RenderWindow window(sf::VideoMode(RESOLUTION_WIDTH, (RESOLUTION_HEIGHT + 20)), "Conway's Game Of Life",
                        Style::Titlebar | Style::Close);
    Texture texture;
    texture.create(CELLS_WIDTH, CELLS_HEIGHT);
    Image image;
    image.create(CELLS_WIDTH, CELLS_HEIGHT, Color::Black);

    Sprite sprite;
    sprite.setScale(RATIO_WIDTH, RATIO_HEIGHT);

    Clock clock;

    loadFonts();
    loadLayouts();
    srand(time(nullptr));

    while (window.isOpen()) {
        Event event;

        while (window.pollEvent(event)) {
            switch (event.type) {
                case Event::Closed: {
                    window.close();
                    break;
                }
                case Event::MouseButtonPressed: {
                    if (running) break;

                    switch (event.mouseButton.button) {
                        case Mouse::Button::Left:
                            toggleCellAt(event.mouseButton.x, event.mouseButton.y);
                            break;
                        case Mouse::Button::Right:
                            randomCellsAround(event.mouseButton.x, event.mouseButton.y);
                            break;
                        case Mouse::Button::Middle:
                            placeLayoutAt(event.mouseButton.x, event.mouseButton.y, getRandomLayout());
                            break;
                    }

                    break;
                }
                case Event::MouseWheelScrolled: {
                    bool up = event.mouseWheelScroll.delta > 0;
                    ticks += (up ? 1 : -1);

                    if (ticks < 1) ticks = 1;

                    updateTicks(&window);
                    setOverlay(&clock, "Speed " + to_string(ticks), 1);

                    break;
                }
                case Event::KeyReleased: {
                    switch (event.key.code) {
                        case Keyboard::Key::Space:
                            running = !running;
                            updateTicks(&window);
                            setOverlay(&clock, running ? "RUN" : "PAUSE", 1);

                            break;
                        case Keyboard::Key::C:
                            running = false;
                            clearCells();
                            setOverlay(&clock, "CLEAR", 1);

                            break;
                        case Keyboard::Key::R:
                            ticks = 5;
                            updateTicks(&window);
                            setOverlay(&clock, "SPEED RESET", 1);

                            break;
                        case Keyboard::Key::Z:
                            randomCellsAround(0, 0, CELLS_WIDTH);

                            break;
                    }
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

        drawHints(&window);
        window.display();
    }

    return 0;
}
