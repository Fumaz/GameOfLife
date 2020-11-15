#include <SFML/Graphics.hpp>
#include <iostream>

using namespace sf;
using namespace std;

const int NORMAL_FPS = 30;

const int MIN = 0;

const int CELLS_WIDTH = 125;
const int CELLS_HEIGHT = 125;

const int RESOLUTION_WIDTH = 750;
const int RESOLUTION_HEIGHT = 750;

const int RATIO_WIDTH = RESOLUTION_WIDTH / CELLS_WIDTH;
const int RATIO_HEIGHT = RESOLUTION_HEIGHT / CELLS_HEIGHT;

bool running = false;
bool cells[CELLS_WIDTH][CELLS_HEIGHT];
bool oldCells[CELLS_WIDTH][CELLS_HEIGHT];

int ticks = 1;

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
            image->setPixel(x, y, cells[x][y] ? Color::Blue : Color::White);
        }
    }
}

int main() {
    RenderWindow window(sf::VideoMode(RESOLUTION_WIDTH, RESOLUTION_HEIGHT), "Conway's Game Of Life", Style::Titlebar | Style::Close);
    Texture texture;
    texture.create(CELLS_WIDTH, CELLS_HEIGHT);
    Image image;
    image.create(CELLS_WIDTH, CELLS_HEIGHT, Color::Black);

    Sprite sprite;
    sprite.setScale(RATIO_WIDTH, RATIO_HEIGHT);

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
                if (event.key.code == Keyboard::Key::Space) {
                    running = !running;

                    updateTicks(&window);
                } else if (event.key.code == Keyboard::Key::Left) {
                    ticks--;
                    updateTicks(&window);
                } else if (event.key.code == Keyboard::Key::Right) {
                    ticks++;
                    updateTicks(&window);
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
        window.display();
    }

    return 0;
}
