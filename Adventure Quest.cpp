#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream>

// Game constants based on assignment
const int gridSize = 5;
const int cellSize = 100;
const int pathLen = 13;
const int coinCount = 8;
const int hurdleCount = 5;

// Point values from assignment
const int GOLD_COIN_POINTS = 10;
const int SILVER_COIN_POINTS = 5;

// Item costs
const int SWORD_COST = 40;
const int SHIELD_COST = 30;
const int WATER_COST = 50;
const int KEY_COST = 70;

// Hurdle costs
const int FIRE_COST = 50;
const int SNAKE_COST = 30;
const int GHOST_COST = 20;
const int LION_COST = 50;    // Only with gold coins
const int LOCK_COST = 60;    // Only with silver coins

// Initial money
const int INITIAL_GOLD = 20;
const int INITIAL_SILVER = 40;

enum CoinType { GOLD, SILVER };
enum HurdleType { FIRE, SNAKE, GHOST, LION, LOCK };
enum GameMode { MOVE_MODE, BUY_MODE, PLACE_HURDLE_MODE };

// Base class for all game items
class GameObject {
public:
    int x, y;
    bool active;

    GameObject() : x(0), y(0), active(true) {}
    GameObject(int xPos, int yPos) : x(xPos), y(yPos), active(true) {}

    virtual void interact() {}
    virtual ~GameObject() {}
};

// Derived class for coins
class Coin : public GameObject {
public:
    CoinType type;
    bool collected;

    Coin() : GameObject(), type(GOLD), collected(false) {}
    Coin(int xPos, int yPos, CoinType coinType) : GameObject(xPos, yPos), type(coinType), collected(false) {}

    void interact() override {
        collected = true;
        active = false;
    }
};

// Derived class for hurdles
class Hurdle : public GameObject {
public:
    HurdleType type;
    bool triggered;

    Hurdle() : GameObject(), type(FIRE), triggered(false) {}
    Hurdle(int xPos, int yPos, HurdleType hurdleType) : GameObject(xPos, yPos), type(hurdleType), triggered(false) {}

    void interact() override {
        triggered = true;
    }
};

class Player {
public:
    int path[pathLen][2];
    int pos;
    sf::Color color;
    int goldCoins, silverCoins;
    int score;
    int skipTurns;
    int sword, shield, water, key;
    bool canMove;
    std::string name;
    bool atGoal;
    char symbol;

    Player(bool isP1, sf::Color col) : pos(0), color(col), skipTurns(0), sword(1), shield(1),
        water(1), key(1), canMove(true), atGoal(false),
        goldCoins(INITIAL_GOLD), silverCoins(INITIAL_SILVER), score(0) {
        name = isP1 ? "Player 1" : "Player 2";
        symbol = isP1 ? '1' : '2';

        int idx = 0;
        if (isP1) {
            // Player 1 path - kept the same as your original code
            for (int c = 4; c >= 0; c--) path[idx][0] = c, path[idx++][1] = 0;
            path[idx][0] = 0; path[idx++][1] = 1;
            for (int c = 1; c <= 4; c++) path[idx][0] = c, path[idx++][1] = 1;
            path[idx][0] = 4; path[idx++][1] = 2;
            path[idx][0] = 3; path[idx++][1] = 2;
            path[idx][0] = 2; path[idx++][1] = 2;
        }
        else {
            // Player 2 path - kept the same as your original code
            for (int c = 0; c <= 4; c++) path[idx][0] = c, path[idx++][1] = 4;
            path[idx][0] = 4; path[idx++][1] = 3;
            for (int c = 3; c >= 0; c--) path[idx][0] = c, path[idx++][1] = 3;
            path[idx][0] = 0; path[idx++][1] = 2;
            path[idx][0] = 1; path[idx++][1] = 2;
            path[idx][0] = 2; path[idx++][1] = 2;
        }
    }

    void move() {
        if (!canMove) return;

        if (skipTurns > 0) {
            skipTurns--;
            return;
        }

        if (pos + 1 < pathLen) {
            pos++;
            canMove = false; // Player must release key before moving again

            // Check if player has reached the goal
            if (path[pos][0] == 2 && path[pos][1] == 2) {
                atGoal = true;
            }
        }
    }

    sf::Vector2i getPosition() {
        return { path[pos][0], path[pos][1] };
    }

    void collectCoin(Coin& coin) {
        if (coin.collected) return;
        sf::Vector2i p = getPosition();
        if (p.x == coin.x && p.y == coin.y) {
            coin.collected = true;
            if (coin.type == GOLD) {
                goldCoins++;
                score += GOLD_COIN_POINTS;
            }
            else {
                silverCoins++;
                score += SILVER_COIN_POINTS;
            }
            std::cout << name << " collected a " << (coin.type == GOLD ? "gold" : "silver") << " coin!" << std::endl;
        }
    }
    bool buyItem(const std::string& itemType) {
        int totalValue = goldCoins * GOLD_COIN_POINTS + silverCoins * SILVER_COIN_POINTS;

        if (itemType == "sword") {
            if (totalValue < SWORD_COST) return false;  // Not enough coins

            score -= SWORD_COST;
            // Deduct from coins (prefer silver first to preserve gold)
            int costRemaining = SWORD_COST;
            int silverValue = silverCoins * SILVER_COIN_POINTS;
            if (silverValue >= costRemaining) {
                silverCoins -= costRemaining / SILVER_COIN_POINTS;
                if (costRemaining % SILVER_COIN_POINTS > 0) silverCoins--;
            }
            else {
                costRemaining -= silverValue;
                silverCoins = 0;
                goldCoins -= costRemaining / GOLD_COIN_POINTS;
                if (costRemaining % GOLD_COIN_POINTS > 0) goldCoins--;
            }
            sword++;
            return true;
        }
        else if (itemType == "shield") {
            if (totalValue < SHIELD_COST) return false;  // Not enough coins

            score -= SHIELD_COST;
            // Similar deduction logic
            int costRemaining = SHIELD_COST;
            int silverValue = silverCoins * SILVER_COIN_POINTS;
            if (silverValue >= costRemaining) {
                silverCoins -= costRemaining / SILVER_COIN_POINTS;
                if (costRemaining % SILVER_COIN_POINTS > 0) silverCoins--;
            }
            else {
                costRemaining -= silverValue;
                silverCoins = 0;
                goldCoins -= costRemaining / GOLD_COIN_POINTS;
                if (costRemaining % GOLD_COIN_POINTS > 0) goldCoins--;
            }
            shield++;
            return true;
        }
        else if (itemType == "water") {
            if (totalValue < WATER_COST) return false;  // Not enough coins

            score -= WATER_COST;
            // Similar deduction logic
            int costRemaining = WATER_COST;
            int silverValue = silverCoins * SILVER_COIN_POINTS;
            if (silverValue >= costRemaining) {
                silverCoins -= costRemaining / SILVER_COIN_POINTS;
                if (costRemaining % SILVER_COIN_POINTS > 0) silverCoins--;
            }
            else {
                costRemaining -= silverValue;
                silverCoins = 0;
                goldCoins -= costRemaining / GOLD_COIN_POINTS;
                if (costRemaining % GOLD_COIN_POINTS > 0) goldCoins--;
            }
            water++;
            return true;
        }
        else if (itemType == "key") {
            if (totalValue < KEY_COST) return false;  // Not enough coins

            score -= KEY_COST;
            // Similar deduction logic
            int costRemaining = KEY_COST;
            int silverValue = silverCoins * SILVER_COIN_POINTS;
            if (silverValue >= costRemaining) {
                silverCoins -= costRemaining / SILVER_COIN_POINTS;
                if (costRemaining % SILVER_COIN_POINTS > 0) silverCoins--;
            }
            else {
                costRemaining -= silverValue;
                silverCoins = 0;
                goldCoins -= costRemaining / GOLD_COIN_POINTS;
                if (costRemaining % GOLD_COIN_POINTS > 0) goldCoins--;
            }
            key++;
            return true;
        }

        return false;
    }
    bool buyHurdle(const std::string& hurdleType) {
        if (hurdleType == "fire") {
            if (goldCoins * GOLD_COIN_POINTS + silverCoins * SILVER_COIN_POINTS < FIRE_COST)
                return false;  // Not enough coins

            score -= FIRE_COST;
            // Deduct from coins
            int costRemaining = FIRE_COST;
            int silverValue = silverCoins * SILVER_COIN_POINTS;
            if (silverValue >= costRemaining) {
                silverCoins -= costRemaining / SILVER_COIN_POINTS;
                if (costRemaining % SILVER_COIN_POINTS > 0) silverCoins--;
            }
            else {
                costRemaining -= silverValue;
                silverCoins = 0;
                goldCoins -= costRemaining / GOLD_COIN_POINTS;
                if (costRemaining % GOLD_COIN_POINTS > 0) goldCoins--;
            }
            return true;
        }
        else if (hurdleType == "snake") {
            if (goldCoins * GOLD_COIN_POINTS + silverCoins * SILVER_COIN_POINTS < SNAKE_COST)
                return false;  // Not enough coins

            score -= SNAKE_COST;
            // Similar deduction logic
            int costRemaining = SNAKE_COST;
            int silverValue = silverCoins * SILVER_COIN_POINTS;
            if (silverValue >= costRemaining) {
                silverCoins -= costRemaining / SILVER_COIN_POINTS;
                if (costRemaining % SILVER_COIN_POINTS > 0) silverCoins--;
            }
            else {
                costRemaining -= silverValue;
                silverCoins = 0;
                goldCoins -= costRemaining / GOLD_COIN_POINTS;
                if (costRemaining % GOLD_COIN_POINTS > 0) goldCoins--;
            }
            return true;
        }
        else if (hurdleType == "ghost") {
            if (goldCoins * GOLD_COIN_POINTS + silverCoins * SILVER_COIN_POINTS < GHOST_COST)
                return false;  // Not enough coins

            score -= GHOST_COST;
            // Similar deduction logic
            int costRemaining = GHOST_COST;
            int silverValue = silverCoins * SILVER_COIN_POINTS;
            if (silverValue >= costRemaining) {
                silverCoins -= costRemaining / SILVER_COIN_POINTS;
                if (costRemaining % SILVER_COIN_POINTS > 0) silverCoins--;
            }
            else {
                costRemaining -= silverValue;
                silverCoins = 0;
                goldCoins -= costRemaining / GOLD_COIN_POINTS;
                if (costRemaining % GOLD_COIN_POINTS > 0) goldCoins--;
            }
            return true;
        }
        else if (hurdleType == "lion") {
            if (goldCoins < LION_COST / GOLD_COIN_POINTS)
                return false;  // Not enough gold coins

            // Lion can only be bought with gold
            score -= LION_COST;
            goldCoins -= LION_COST / GOLD_COIN_POINTS;
            return true;
        }
        else if (hurdleType == "lock") {
            if (silverCoins < LOCK_COST / SILVER_COIN_POINTS)
                return false;  // Not enough silver coins

            // Lock can only be bought with silver
            score -= LOCK_COST;
            silverCoins -= LOCK_COST / SILVER_COIN_POINTS;
            return true;
        }

        return false;
    }

    void handleHurdle(Hurdle& h) {
        if (h.triggered) return;
        sf::Vector2i p = getPosition();
        if (p.x == h.x && p.y == h.y) {
            switch (h.type) {
            case FIRE:
                if (water > 0) {
                    water--;
                    std::cout << name << " used water to extinguish fire!" << std::endl;
                }
                else {
                    skipTurns = 2;
                    std::cout << name << " got burned! Skip 2 turns." << std::endl;
                }
                break;
            case SNAKE:
                if (sword > 0) {
                    sword--;
                    std::cout << name << " used sword to defeat snake!" << std::endl;
                }
                else {
                    skipTurns = 3;
                    if (pos >= 3) pos -= 3;
                    std::cout << name << " was bitten by snake! Move back 3 spaces and skip 3 turns." << std::endl;
                }
                break;
            case GHOST:
                if (shield > 0) {
                    shield--;
                    std::cout << name << " used shield against ghost!" << std::endl;
                }
                else {
                    skipTurns = 1;
                    std::cout << name << " was scared by ghost! Skip 1 turn." << std::endl;
                }
                break;
            case LION:
                if (sword > 0) {
                    sword--;
                    std::cout << name << " used sword to defeat lion!" << std::endl;
                }
                else {
                    skipTurns = 4;
                    std::cout << name << " was attacked by lion! Skip 4 turns." << std::endl;
                }
                break;
            case LOCK:
                if (key > 0) {
                    key--;
                    std::cout << name << " used key to unlock!" << std::endl;
                }
                else {
                    skipTurns = 5; 
                    std::cout << name << " is locked! have to wait for 5 turns ." << std::endl;
                }
                break;
            }
            h.triggered = true;
        }
    }

    int getScore() {
        return score;
    }

    void draw(sf::RenderWindow& window) {
        sf::CircleShape playerShape(cellSize / 3);
        playerShape.setFillColor(color);
        playerShape.setPosition(path[pos][0] * cellSize + cellSize / 3, path[pos][1] * cellSize + cellSize / 3);
        window.draw(playerShape);

        // Draw player symbol (P1 or P2)
        sf::Text playerText;
        sf::Font font;
        if (font.loadFromFile("arial.ttf")) {
            playerText.setFont(font);
            playerText.setString("P" + std::string(1, symbol));
            playerText.setCharacterSize(20);
            playerText.setFillColor(sf::Color::White);
            playerText.setPosition(path[pos][0] * cellSize + cellSize / 2 - 10,
                path[pos][1] * cellSize + cellSize / 2 - 10);
            window.draw(playerText);
        }

        // Draw skip turns indicator if needed
        if (skipTurns > 0) {
            sf::Text skipText;
            if (font.loadFromFile("arial.ttf")) {
                skipText.setFont(font);
                skipText.setString(std::to_string(skipTurns));
                skipText.setCharacterSize(16);
                skipText.setFillColor(sf::Color::White);
                skipText.setPosition(path[pos][0] * cellSize + cellSize / 2 + 10,
                    path[pos][1] * cellSize + cellSize / 2 - 10);
                window.draw(skipText);
            }
        }
    }
};

class Game {
private:
    sf::RenderWindow window;
    Player p1, p2;
    Coin coins[coinCount];
    Hurdle hurdles[hurdleCount];
    bool gameOver;
    sf::Font font;
    sf::Clock moveClock;
    const float moveDelay = 0.1f; // 100ms delay between moves
    sf::Text statusText;
    std::string statusMessage;
    sf::Clock statusClock;
    GameMode currentMode;
    int currentPlayer; // 1 for P1, 2 for P2
    HurdleType selectedHurdleType;
    bool placingHurdle;

    // Visual feedback elements
    sf::RectangleShape p1InfoBox;
    sf::RectangleShape p2InfoBox;
    sf::RectangleShape shopPanel;
    sf::RectangleShape actionPanel;

public:
    Game() : window(sf::VideoMode(gridSize* cellSize, gridSize* cellSize + 150), "Adventure Quest"),
        p1(true, sf::Color(255, 50, 50)), p2(false, sf::Color(100, 100, 255)),
        gameOver(false), currentMode(MOVE_MODE), currentPlayer(1),
        placingHurdle(false) {

        std::srand(static_cast<unsigned>(std::time(0)));
        if (!font.loadFromFile("arial.ttf")) {
            std::cerr << "Error loading font!" << std::endl;
        }

        // Initialize status text
        statusText.setFont(font);
        statusText.setCharacterSize(16);
        statusText.setFillColor(sf::Color::Red);
        statusText.setPosition(10, gridSize * cellSize + 100);

        // Initialize player info boxes
        p1InfoBox.setSize(sf::Vector2f(gridSize * cellSize / 2 - 10, 40));
        p1InfoBox.setPosition(5, gridSize * cellSize + 5);
        p1InfoBox.setFillColor(sf::Color(255, 200, 200, 150)); // Light red background

        p2InfoBox.setSize(sf::Vector2f(gridSize * cellSize / 2 - 10, 40));
        p2InfoBox.setPosition(gridSize * cellSize / 2 + 5, gridSize * cellSize + 5);
        p2InfoBox.setFillColor(sf::Color(200, 200, 250, 150)); // Light blue background

        // Shop panel
        shopPanel.setSize(sf::Vector2f(gridSize * cellSize, 50));
        shopPanel.setPosition(0, gridSize * cellSize + 50);
        shopPanel.setFillColor(sf::Color(200, 200, 200, 150));

        // Initialize coins with random positions, avoiding player start positions
        for (int i = 0; i < coinCount; i++) {
            bool validPosition = false;
            while (!validPosition) {
                coins[i].x = std::rand() % gridSize;
                coins[i].y = std::rand() % gridSize;

                // Avoid placing coins on player start positions or goal (2,2)
                if ((coins[i].x == 4 && coins[i].y == 0) || // P1 start
                    (coins[i].x == 0 && coins[i].y == 4) || // P2 start
                    (coins[i].x == 2 && coins[i].y == 2)) { // Goal
                    continue;
                }

                // Check if position already has a coin
                validPosition = true;
                for (int j = 0; j < i; j++) {
                    if (coins[j].x == coins[i].x && coins[j].y == coins[i].y) {
                        validPosition = false;
                        break;
                    }
                }
            }

            coins[i].type = i < 4 ? GOLD : SILVER;
            coins[i].collected = false;
        }

        // Initialize hurdles with random positions, avoiding coins, player start positions and goal
        for (int i = 0; i < hurdleCount; i++) {
            bool validPosition = false;
            while (!validPosition) {
                hurdles[i].x = std::rand() % gridSize;
                hurdles[i].y = std::rand() % gridSize;

                // Avoid placing hurdles on player start positions or goal
                if ((hurdles[i].x == 4 && hurdles[i].y == 0) || // P1 start
                    (hurdles[i].x == 0 && hurdles[i].y == 4) || // P2 start
                    (hurdles[i].x == 2 && hurdles[i].y == 2)) { // Goal
                    continue;
                }

                // Check if position already has a coin or hurdle
                validPosition = true;
                for (int c = 0; c < coinCount; c++) {
                    if (coins[c].x == hurdles[i].x && coins[c].y == hurdles[i].y) {
                        validPosition = false;
                        break;
                    }
                }

                if (!validPosition) continue;

                for (int h = 0; h < i; h++) {
                    if (hurdles[h].x == hurdles[i].x && hurdles[h].y == hurdles[i].y) {
                        validPosition = false;
                        break;
                    }
                }
            }

            hurdles[i].type = static_cast<HurdleType>(std::rand() % 5);
            hurdles[i].triggered = false;
        }
    }

    void drawGrid() {
        for (int y = 0; y < gridSize; y++) {
            for (int x = 0; x < gridSize; x++) {
                sf::RectangleShape cell(sf::Vector2f(cellSize - 2, cellSize - 2));
                cell.setPosition(x * cellSize + 1, y * cellSize + 1);

                // Special coloring for the goal cell
                if (x == 2 && y == 2)
                    cell.setFillColor(sf::Color(255, 215, 0)); // Gold color for the goal
                // Player 1 path - alternate colors for clarity
                else if ((y == 0 || y == 1 || (y == 2 && (x == 4 || x == 3))) &&
                    !((x == 0 && y == 4) || (x == 2 && y == 2)))
                    cell.setFillColor(sf::Color(255, 150, 150)); // Very light red for P1 path
                // Player 2 path - alternate colors for clarity
                else if ((y == 4 || y == 3 || (y == 2 && (x == 0 || x == 1))) &&
                    !((x == 4 && y == 0) || (x == 2 && y == 2)))
                    cell.setFillColor(sf::Color(150, 150, 255)); // Very light blue for P2 path
                // Special coloring for player starting positions
                else if (x == 4 && y == 0) // P1 start
                    cell.setFillColor(sf::Color(255, 200, 200)); // Light red for P1 start
                else if (x == 0 && y == 4) // P2 start
                    cell.setFillColor(sf::Color(200, 200, 255)); // Light blue for P2 start
                else
                    cell.setFillColor(sf::Color(240, 240, 240)); // Off-white for other cells

                window.draw(cell);
            }
        }

        // Draw path grid lines for better visibility
        for (int i = 0; i <= gridSize; i++) {
            sf::RectangleShape horizontalLine(sf::Vector2f(gridSize * cellSize, 1));
            horizontalLine.setPosition(0, i * cellSize);
            horizontalLine.setFillColor(sf::Color(100, 100, 100));
            window.draw(horizontalLine);

            sf::RectangleShape verticalLine(sf::Vector2f(1, gridSize * cellSize));
            verticalLine.setPosition(i * cellSize, 0);
            verticalLine.setFillColor(sf::Color(100, 100, 100));
            window.draw(verticalLine);
        }
    }

    void drawCoins() {
        for (int i = 0; i < coinCount; i++) {
            if (coins[i].collected) continue;

            sf::CircleShape coinShape(cellSize / 5);
            coinShape.setPosition(coins[i].x * cellSize + cellSize / 3, coins[i].y * cellSize + cellSize / 3);

            if (coins[i].type == GOLD) {
                coinShape.setFillColor(sf::Color(255, 215, 0)); // Gold color

                // Draw $ symbol inside gold coin
                sf::Text symbol;
                symbol.setFont(font);
                symbol.setString("$");
                symbol.setCharacterSize(22);
                symbol.setFillColor(sf::Color(150, 150, 0));
                symbol.setPosition(coins[i].x * cellSize + cellSize / 2 - 6,
                    coins[i].y * cellSize + cellSize / 2 - 12);
                window.draw(coinShape);
                window.draw(symbol);
            }
            else {
                coinShape.setFillColor(sf::Color(192, 192, 192)); // Silver color

                // Draw ¢ symbol inside silver coin
                sf::Text symbol;
                symbol.setFont(font);
                symbol.setString("¢");
                symbol.setCharacterSize(22);
                symbol.setFillColor(sf::Color(100, 100, 100));
                symbol.setPosition(coins[i].x * cellSize + cellSize / 2 - 6,
                    coins[i].y * cellSize + cellSize / 2 - 12);
                window.draw(coinShape);
                window.draw(symbol);
            }
        }
    }

    void drawHurdles() {
        for (int i = 0; i < hurdleCount; i++) {
            if (hurdles[i].triggered) continue;

            sf::CircleShape hurdle(cellSize / 5);
            hurdle.setPosition(hurdles[i].x * cellSize + cellSize / 3, hurdles[i].y * cellSize + cellSize / 3);

            // Different colors & symbols for different hurdle types
            sf::Text symbol;
            symbol.setFont(font);
            symbol.setCharacterSize(22);

            switch (hurdles[i].type) {
            case FIRE:
                hurdle.setFillColor(sf::Color(255, 80, 80));
                symbol.setString("F");
                symbol.setFillColor(sf::Color(255, 255, 150));
                break;
            case SNAKE:
                hurdle.setFillColor(sf::Color(100, 180, 100));
                symbol.setString("S");
                symbol.setFillColor(sf::Color(50, 100, 50));
                break;
            case GHOST:
                hurdle.setFillColor(sf::Color(200, 200, 255));
                symbol.setString("G");
                symbol.setFillColor(sf::Color(100, 100, 200));
                break;
            case LION:
                hurdle.setFillColor(sf::Color(255, 180, 100));
                symbol.setString("L");
                symbol.setFillColor(sf::Color(200, 100, 0));
                break;
            case LOCK:
                hurdle.setFillColor(sf::Color(200, 100, 200));
                symbol.setString("X");
                symbol.setFillColor(sf::Color(150, 0, 150));
                break;
            }

            symbol.setPosition(hurdles[i].x * cellSize + cellSize / 2 - 6,
                hurdles[i].y * cellSize + cellSize / 2 - 12);
            window.draw(hurdle);
            window.draw(symbol);
        }
    }

    void drawShop() {
        window.draw(shopPanel);

        // Draw shop title
        sf::Text shopTitle;
        shopTitle.setFont(font);
        shopTitle.setCharacterSize(14);
        shopTitle.setFillColor(sf::Color::Black);
        shopTitle.setPosition(10, gridSize * cellSize + 55);

        if (currentMode == BUY_MODE) {
            shopTitle.setString("SHOP - Press [H]elping Objects or [B]lockages");
        }
        else if (currentMode == PLACE_HURDLE_MODE) {
            std::string hurdleText;
            switch (selectedHurdleType) {
            case FIRE: hurdleText = "FIRE (50pts)"; break;
            case SNAKE: hurdleText = "SNAKE (30pts)"; break;
            case GHOST: hurdleText = "GHOST (20pts)"; break;
            case LION: hurdleText = "LION (50pts - Gold only)"; break;
            case LOCK: hurdleText = "LOCK (60pts - Silver only)"; break;
            }
            shopTitle.setString("Place " + hurdleText + " - Click on grid");
        }
        else {
            shopTitle.setString("Press [B] to buy items or [M] to move");
        }

        window.draw(shopTitle);

        // Draw instructions
        sf::Text instructions;
        instructions.setFont(font);
        instructions.setCharacterSize(12);
        instructions.setFillColor(sf::Color(80, 80, 80));
        instructions.setPosition(gridSize * cellSize - 240, gridSize * cellSize + 80);

        if (currentMode == MOVE_MODE) {
            instructions.setString("Press 1 for P1, 2 for P2, [B] to buy");
        }
        else if (currentMode == BUY_MODE) {
            instructions.setString("Press 1-5 for items, [Esc] to cancel");
        }
        else {
            instructions.setString("Click grid to place, [Esc] to cancel");
        }

        window.draw(instructions);
    }

    void drawScores() {
        // Draw info boxes
        window.draw(p1InfoBox);
        window.draw(p2InfoBox);

        // Draw player 1 score and inventory
        sf::Text p1Text;
        p1Text.setFont(font);
        p1Text.setCharacterSize(12);
        p1Text.setFillColor(sf::Color::Black);
        p1Text.setPosition(10, gridSize * cellSize + 10);
        p1Text.setString(p1.name + ": Score " + std::to_string(p1.getScore()) +
            " | Gold " + std::to_string(p1.goldCoins) +
            " | Silver " + std::to_string(p1.silverCoins));
        window.draw(p1Text);

        // Draw player 1 inventory
        sf::Text p1Inventory;
        p1Inventory.setFont(font);
        p1Inventory.setCharacterSize(10);
        p1Inventory.setFillColor(sf::Color(100, 0, 0));
        p1Inventory.setPosition(10, gridSize * cellSize + 25);
        p1Inventory.setString("Sword: " + std::to_string(p1.sword) +
            " | Shield: " + std::to_string(p1.shield) +
            " | Water: " + std::to_string(p1.water) +
            " | Key: " + std::to_string(p1.key));
        window.draw(p1Inventory);

        // Draw player 2 score and inventory
        sf::Text p2Text;
        p2Text.setFont(font);
        p2Text.setCharacterSize(12);
        p2Text.setFillColor(sf::Color::Black);
        p2Text.setPosition(gridSize * cellSize / 2 + 10, gridSize * cellSize + 10);
        p2Text.setString(p2.name + ": Score " + std::to_string(p2.getScore()) +
            " | Gold " + std::to_string(p2.goldCoins) +
            " | Silver " + std::to_string(p2.silverCoins));
        window.draw(p2Text);

        // Draw player 2 inventory
        sf::Text p2Inventory;
        p2Inventory.setFont(font);
        p2Inventory.setCharacterSize(10);
        p2Inventory.setFillColor(sf::Color(0, 0, 100));
        p2Inventory.setPosition(gridSize * cellSize / 2 + 10, gridSize * cellSize + 25);
        p2Inventory.setString("Sword: " + std::to_string(p2.sword) +
            " | Shield: " + std::to_string(p2.shield) +
            " | Water: " + std::to_string(p2.water) +
            " | Key: " + std::to_string(p2.key));
        window.draw(p2Inventory);

       
    }

    void drawGameStatus() {
        // Draw status message
        if (!statusMessage.empty() && statusClock.getElapsedTime().asSeconds() < 5.0f) {
            statusText.setString(statusMessage);
            window.draw(statusText);
        }

        // Draw game result if game is over
        if (gameOver) {
            sf::RectangleShape overlay(sf::Vector2f(gridSize * cellSize, gridSize * cellSize));
            overlay.setFillColor(sf::Color(0, 0, 0, 150)); // Semi-transparent black
            window.draw(overlay);

            sf::Text gameOverText;
            gameOverText.setFont(font);
            gameOverText.setCharacterSize(40);
            gameOverText.setStyle(sf::Text::Bold);
            gameOverText.setFillColor(sf::Color::White);

            if (p1.atGoal && p2.atGoal) {
                // Both reached goal, compare scores
                if (p1.getScore() > p2.getScore()) {
                    gameOverText.setString("Player 1 Wins!");
                    gameOverText.setFillColor(sf::Color(255, 100, 100));
                }
                else if (p2.getScore() > p1.getScore()) {
                    gameOverText.setString("Player 2 Wins!");
                    gameOverText.setFillColor(sf::Color(100, 100, 255));
                }
                else {
                    gameOverText.setString("It's a Tie!");
                    gameOverText.setFillColor(sf::Color::White);
                }
            }
            else if (p1.atGoal) {
                gameOverText.setString("Player 1 Wins!");
                gameOverText.setFillColor(sf::Color(255, 100, 100));
            }
            else if (p2.atGoal) {
                gameOverText.setString("Player 2 Wins!");
                gameOverText.setFillColor(sf::Color(100, 100, 255));
            }

            // Center the text
            sf::FloatRect textRect = gameOverText.getLocalBounds();
            gameOverText.setOrigin(textRect.left + textRect.width / 2.0f,
                textRect.top + textRect.height / 2.0f);
            gameOverText.setPosition(sf::Vector2f(gridSize * cellSize / 2.0f, gridSize * cellSize / 2.0f));
            window.draw(gameOverText);

            // Restart instructions
            sf::Text restartText;
            restartText.setFont(font);
            restartText.setCharacterSize(20);
            restartText.setFillColor(sf::Color::White);
            textRect = restartText.getLocalBounds();
            restartText.setOrigin(textRect.left + textRect.width / 2.0f,
                textRect.top + textRect.height / 2.0f);
            restartText.setPosition(sf::Vector2f(gridSize * cellSize / 2.0f, gridSize * cellSize / 2.0f + 50));
            window.draw(restartText);
        }
    }

    void setStatusMessage(const std::string& message) {
        statusMessage = message;
        statusClock.restart();
    }
        
    void checkCollisions() {
        // Check coin collections
        for (int i = 0; i < coinCount; i++) {
            p1.collectCoin(coins[i]);
            p2.collectCoin(coins[i]);
        }

        // Check hurdle interactions
        for (int i = 0; i < hurdleCount; i++) {
            p1.handleHurdle(hurdles[i]);
            p2.handleHurdle(hurdles[i]);
        }

        // Check if any player reached the goal
        if (p1.atGoal || p2.atGoal) {
            gameOver = true;
        }
    }

    void placeHurdle(int gridX, int gridY) {
        // Make sure grid position is valid
        if (gridX < 0 || gridX >= gridSize || gridY < 0 || gridY >= gridSize) {
            setStatusMessage("Invalid position for placing hurdle!");
            return;
        }

        // Don't place on player start or goal positions
        if ((gridX == 4 && gridY == 0) || // P1 start
            (gridX == 0 && gridY == 4) || // P2 start
            (gridX == 2 && gridY == 2)) { // Goal
            setStatusMessage("Cannot place hurdle on start or goal positions!");
            return;
        }

        // Check if position already has a coin or hurdle
        for (int c = 0; c < coinCount; c++) {
            if (!coins[c].collected && coins[c].x == gridX && coins[c].y == gridY) {
                setStatusMessage("Cannot place hurdle on a coin!");
                return;
            }
        }

        for (int h = 0; h < hurdleCount; h++) {
            if (!hurdles[h].triggered && hurdles[h].x == gridX && hurdles[h].y == gridY) {
                setStatusMessage("Cannot place hurdle on another hurdle!");
                return;
            }
        }

        // Check if current player can buy the selected hurdle
        Player& currentPlayerObj = (currentPlayer == 1) ? p1 : p2;
        std::string hurdleTypeStr;
        switch (selectedHurdleType) {
        case FIRE: hurdleTypeStr = "fire"; break;
        case SNAKE: hurdleTypeStr = "snake"; break;
        case GHOST: hurdleTypeStr = "ghost"; break;
        case LION: hurdleTypeStr = "lion"; break;
        case LOCK: hurdleTypeStr = "lock"; break;
        }

        if (currentPlayerObj.buyHurdle(hurdleTypeStr)) {
            // Find an inactive hurdle to replace or create a new one
            bool placed = false;
            for (int h = 0; h < hurdleCount; h++) {
                if (hurdles[h].triggered) {
                    hurdles[h] = Hurdle(gridX, gridY, selectedHurdleType);
                    placed = true;
                    break;
                }
            }

            if (!placed) {
                // Create a new hurdle by replacing a random one
                int idx = std::rand() % hurdleCount;
                hurdles[idx] = Hurdle(gridX, gridY, selectedHurdleType);
            }

            setStatusMessage(currentPlayerObj.name + " placed a " + hurdleTypeStr + " hurdle!");
            currentMode = MOVE_MODE;
        }
        else {
            setStatusMessage("Not enough coins to buy this hurdle!");
        }
    }

    void handleBuyItemMode(sf::Keyboard::Key key) {
        Player& currentPlayerObj = (currentPlayer == 1) ? p1 : p2;
        std::string itemType;

        if (key == sf::Keyboard::H) {
            // Show helping objects submenu
            setStatusMessage("Press: [1] Sword (40), [2] Shield (30), [3] Water (50), [4] Key (70)");
            return;
        }
        else if (key == sf::Keyboard::B) {
            // Show blockages submenu
            setStatusMessage("Press: [1] Fire (50), [2] Snake (30), [3] Ghost (20), [4] Lion (50-Gold), [5] Lock (60-Silver)");
            return;
        }
        else if (key == sf::Keyboard::Num1 || key == sf::Keyboard::Numpad1) {
            if (statusMessage.find("Sword") != std::string::npos) {
                itemType = "sword";
            }
            else if (statusMessage.find("Fire") != std::string::npos) {
                selectedHurdleType = FIRE;
                currentMode = PLACE_HURDLE_MODE;
                setStatusMessage("Click on the grid to place a FIRE hurdle");
                return;
            }
        }
        else if (key == sf::Keyboard::Num2 || key == sf::Keyboard::Numpad2) {
            if (statusMessage.find("Shield") != std::string::npos) {
                itemType = "shield";
            }
            else if (statusMessage.find("Snake") != std::string::npos) {
                selectedHurdleType = SNAKE;
                currentMode = PLACE_HURDLE_MODE;
                setStatusMessage("Click on the grid to place a SNAKE hurdle");
                return;
            }
        }
        else if (key == sf::Keyboard::Num3 || key == sf::Keyboard::Numpad3) {
            if (statusMessage.find("Water") != std::string::npos) {
                itemType = "water";
            }
            else if (statusMessage.find("Ghost") != std::string::npos) {
                selectedHurdleType = GHOST;
                currentMode = PLACE_HURDLE_MODE;
                setStatusMessage("Click on the grid to place a GHOST hurdle");
                return;
            }
        }
        else if (key == sf::Keyboard::Num4 || key == sf::Keyboard::Numpad4) {
            if (statusMessage.find("Key") != std::string::npos) {
                itemType = "key";
            }
            else if (statusMessage.find("Lion") != std::string::npos) {
                selectedHurdleType = LION;
                currentMode = PLACE_HURDLE_MODE;
                setStatusMessage("Click on the grid to place a LION hurdle");
                return;
            }
        }
        else if ((key == sf::Keyboard::Num5 || key == sf::Keyboard::Numpad5) && statusMessage.find("Lock") != std::string::npos) {
            selectedHurdleType = LOCK;
            currentMode = PLACE_HURDLE_MODE;
            setStatusMessage("Click on the grid to place a LOCK hurdle");
            return;
        }
        else if (key == sf::Keyboard::Escape || key == sf::Keyboard::M) {
            currentMode = MOVE_MODE;
            setStatusMessage("Returned to move mode");
            return;
        }

        if (!itemType.empty()) {
            if (currentPlayerObj.buyItem(itemType)) {
                setStatusMessage(currentPlayerObj.name + " bought a " + itemType + "!");
                currentMode = MOVE_MODE;
            }
            else {
                setStatusMessage("Not enough coins to buy this item!");
            }
        }
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (gameOver) {
                    continue; 
                }

                // Handle player movement keys
                if (currentMode == MOVE_MODE) {
                    if (event.key.code == sf::Keyboard::Num1 || event.key.code == sf::Keyboard::Numpad1) {
                        currentPlayer = 1;
                        p1.move();
                        checkCollisions();
                    }
                    else if (event.key.code == sf::Keyboard::Num2 || event.key.code == sf::Keyboard::Numpad2) {
                        currentPlayer = 2;
                        p2.move();
                        checkCollisions();
                    }
                    else if (event.key.code == sf::Keyboard::B) {
                        currentMode = BUY_MODE;
                        setStatusMessage("Buy Mode: Press [H]elping Objects or [B]lockages");
                    }
                }
                else if (currentMode == BUY_MODE) {
                    handleBuyItemMode(event.key.code);
                }
                else if (currentMode == PLACE_HURDLE_MODE) {
                    if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::M) {
                        currentMode = MOVE_MODE;
                        setStatusMessage("Returned to move mode");
                    }
                }

                // Common keys for all modes
                if (event.key.code == sf::Keyboard::M) {
                    currentMode = MOVE_MODE;
                    setStatusMessage("Move Mode");
                }
                else if (event.key.code == sf::Keyboard::Key::Space) {
                    p1.canMove = true;
                    p2.canMove = true;
                }
            }
            else if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::Num1 || event.key.code == sf::Keyboard::Numpad1 ||
                    event.key.code == sf::Keyboard::Num2 || event.key.code == sf::Keyboard::Numpad2) {
                    p1.canMove = true;
                    p2.canMove = true;
                }
            }
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (currentMode == PLACE_HURDLE_MODE) {
                    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
                    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

                    int gridX = worldPos.x / cellSize;
                    int gridY = worldPos.y / cellSize;



                    if (gridX >= 0 && gridX < gridSize && gridY >= 0 && gridY < gridSize) {
                        placeHurdle(gridX, gridY);
                    }
                }
            }
        }
    }

    void draw() {
        window.clear(sf::Color(50, 50, 50));

        drawGrid();
        drawCoins();
        drawHurdles();
        p1.draw(window);
        p2.draw(window);
        drawScores();
        drawShop();
        drawGameStatus();

        window.display();
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            draw();
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}