#include <iostream>
#include <vector>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>
#include <windows.h>

using namespace std;

// Game constants
const char WALL = '#';
const char FLOOR = '.';
const char PLAYER = '@';
const char SLIME = 's';
const char GOBLIN = 'g';
const char TROLL = 'T';
const char KEY = 'K';
const char DOOR = 'D';
const char HEALTH = '+';
const char WEAPON = 'W';
const char ARMOR = 'A';
const char TRAP = '^';
const char GOLD = '$';
const char STAIRS = '>';

// Colors for Windows console
enum Color {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    BROWN = 6,
    LIGHTGRAY = 7,
    DARKGRAY = 8,
    LIGHTBLUE = 9,
    LIGHTGREEN = 10,
    LIGHTCYAN = 11,
    LIGHTRED = 12,
    LIGHTMAGENTA = 13,
    YELLOW = 14,
    WHITE = 15
};

// Map dimensions
const int WIDTH = 50;
const int HEIGHT = 25;

// Item and weapon classes
class Item {
public:
    string name;
    string description;
    
    Item(string n, string desc) : name(n), description(desc) {}
    virtual ~Item() {}
    virtual void use(class Player& player) = 0;
};

class HealthPotion : public Item {
public:
    int healAmount;
    
    HealthPotion(int amount) : Item("Health Potion", "Restores " + to_string(amount) + " health"), healAmount(amount) {}
    
    void use(class Player& player) override;
};

class Weapon {
public:
    string name;
    int damage;
    int durability;
    
    Weapon(string n, int dmg, int dur) : name(n), damage(dmg), durability(dur) {}
};

class Armor {
public:
    string name;
    int defense;
    
    Armor(string n, int def) : name(n), defense(def) {}
};

// Player class with expanded functionality
class Player {
public:
    int x, y;
    int health = 100;
    int maxHealth = 100;
    int attack = 10;
    int defense = 0;
    int gold = 0;
    int score = 0;
    int level = 1;
    int experience = 0;
    int experienceToLevel = 100;
    bool hasKey = false;
    Weapon* equippedWeapon = nullptr;
    Armor* equippedArmor = nullptr;
    vector<Item*> inventory;
    int inventorySize = 10;
    int dungeonLevel = 1;

    Player(int startX, int startY) : x(startX), y(startY) {
        equippedWeapon = new Weapon("Dagger", 5, 50);
    }
    
    ~Player() {
        delete equippedWeapon;
        delete equippedArmor;
        for (auto item : inventory) {
            delete item;
        }
    }

    void move(int dx, int dy, const vector<vector<char>>& map) {
        int newX = x + dx;
        int newY = y + dy;
        if (newX >= 0 && newX < WIDTH && newY >= 0 && newY < HEIGHT) {
            if (map[newY][newX] != WALL) {
                x = newX;
                y = newY;
            }
        }
    }
    
    int getTotalAttack() const {
        return attack + (equippedWeapon ? equippedWeapon->damage : 0);
    }
    
    int getTotalDefense() const {
        return defense + (equippedArmor ? equippedArmor->defense : 0);
    }
    
    void gainExperience(int exp) {
        experience += exp;
        if (experience >= experienceToLevel) {
            levelUp();
        }
    }
    
    void levelUp() {
        level++;
        experience -= experienceToLevel;
        experienceToLevel = level * 100;
        maxHealth += 10;
        health = maxHealth;
        attack += 2;
        defense += 1;
    }
    
    void equipWeapon(Weapon* weapon) {
        delete equippedWeapon;
        equippedWeapon = weapon;
    }
    
    void equipArmor(Armor* armor) {
        delete equippedArmor;
        equippedArmor = armor;
    }
    
    void addToInventory(Item* item) {
        if (inventory.size() < inventorySize) {
            inventory.push_back(item);
        }
    }
    
    void useItem(int index) {
        if (index >= 0 && index < inventory.size()) {
            inventory[index]->use(*this);
            delete inventory[index];
            inventory.erase(inventory.begin() + index);
        }
    }
};

// Implementation of HealthPotion::use
void HealthPotion::use(Player& player) {
    player.health = min(player.maxHealth, player.health + healAmount);
}

// Enemy base class
class Enemy {
public:
    int x, y;
    int health;
    int maxHealth;
    int attack;
    int defense;
    int experienceValue;
    int goldValue;
    char symbol;
    string name;
    bool hasAttacked = false;
    int moveCooldown = 0;

    Enemy(int startX, int startY, int hp, int atk, int def, int exp, int gold, char sym, string n) 
        : x(startX), y(startY), health(hp), maxHealth(hp), attack(atk), defense(def), 
          experienceValue(exp), goldValue(gold), symbol(sym), name(n) {}
          
    virtual ~Enemy() {}

    virtual void move(int playerX, int playerY, const vector<vector<char>>& map) {
        if (moveCooldown > 0) {
            moveCooldown--;
            return;
        }
        
        // Simple A* pathfinding
        int dx = 0, dy = 0;
        
        // Calculate direction to player
        if (x < playerX) dx = 1;
        else if (x > playerX) dx = -1;
        
        if (y < playerY) dy = 1;
        else if (y > playerY) dy = -1;
        
        // Try to move
        int newX = x + dx;
        int newY = y + dy;
        
        // Check if the move is valid
        if (newX >= 0 && newX < WIDTH && newY >= 0 && newY < HEIGHT && map[newY][newX] != WALL) {
            // Check for collision with other enemies (would need game manager access)
            x = newX;
            y = newY;
        } else {
            // Try moving in just one direction if diagonal movement is blocked
            newX = x + dx;
            newY = y;
            if (newX >= 0 && newX < WIDTH && map[newY][newX] != WALL) {
                x = newX;
            } else {
                newX = x;
                newY = y + dy;
                if (newY >= 0 && newY < HEIGHT && map[newY][newX] != WALL) {
                    y = newY;
                }
            }
        }
    }
    
    virtual int attackPlayer(Player& player) {
        int damage = max(1, attack - player.getTotalDefense() / 2);
        player.health -= damage;
        hasAttacked = true;
        return damage;
    }
    
    virtual void takeDamage(int damage) {
        health -= max(1, damage - defense / 2);
    }
    
    bool isAdjacent(int playerX, int playerY) const {
        return abs(x - playerX) <= 1 && abs(y - playerY) <= 1;
    }
};

// Enemy subtypes
class Slime : public Enemy {
public:
    Slime(int x, int y) : Enemy(x, y, 15, 5, 1, 10, 2, SLIME, "Slime") {
        moveCooldown = 2; // Slimes move slower
    }
    
    void move(int playerX, int playerY, const vector<vector<char>>& map) override {
        // Slimes move randomly 50% of the time
        if (rand() % 2 == 0) {
            int dx = rand() % 3 - 1;
            int dy = rand() % 3 - 1;
            int newX = x + dx;
            int newY = y + dy;
            if (newX >= 0 && newX < WIDTH && newY >= 0 && newY < HEIGHT && map[newY][newX] != WALL) {
                x = newX;
                y = newY;
            }
        } else {
            Enemy::move(playerX, playerY, map);
        }
    }
};

class Goblin : public Enemy {
public:
    Goblin(int x, int y) : Enemy(x, y, 25, 8, 2, 20, 5, GOBLIN, "Goblin") {}
    
    int attackPlayer(Player& player) override {
        // Goblins sometimes do critical hits
        if (rand() % 5 == 0) {
            int damage = max(1, attack * 2 - player.getTotalDefense() / 2);
            player.health -= damage;
            hasAttacked = true;
            return damage;
        }
        return Enemy::attackPlayer(player);
    }
};

class Troll : public Enemy {
public:
    Troll(int x, int y) : Enemy(x, y, 60, 15, 5, 50, 15, TROLL, "Troll") {
        moveCooldown = 1; // Trolls move a bit slower
    }
    
    void takeDamage(int damage) override {
        Enemy::takeDamage(damage);
        // Trolls regenerate health
        if (rand() % 4 == 0) {
            health = min(maxHealth, health + 2);
        }
    }
};

// Game Manager
class GameManager {
private:
    vector<vector<char>> map;
    Player player;
    vector<Enemy*> enemies;
    bool gameOver;
    vector<string> messages;
    int maxMessages = 5;
    int turns = 0;
    HANDLE consoleHandle;

public:
    GameManager() : player(1, 1), gameOver(false) {
        consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        srand(time(0)); // Initialize random seed
        initializeMap(1); // Start with level 1
    }
    
    ~GameManager() {
        for (auto enemy : enemies) {
            delete enemy;
        }
    }

    void initializeMap(int dungeonLevel) {
        player.dungeonLevel = dungeonLevel;
        
        // Clear previous map and enemies
        map = vector<vector<char>>(HEIGHT, vector<char>(WIDTH, FLOOR));
        for (auto enemy : enemies) {
            delete enemy;
        }
        enemies.clear();
        
        // Create walls around the edges
        for (int i = 0; i < WIDTH; i++) {
            map[0][i] = WALL;
            map[HEIGHT-1][i] = WALL;
        }
        for (int i = 0; i < HEIGHT; i++) {
            map[i][0] = WALL;
            map[i][WIDTH-1] = WALL;
        }
        
        // Generate interior walls based on dungeon level
        int numWalls = 10 + dungeonLevel * 2;
        for (int i = 0; i < numWalls; i++) {
            int wallLength = 5 + rand() % 10;
            int startX = 2 + rand() % (WIDTH - 4);
            int startY = 2 + rand() % (HEIGHT - 4);
            int direction = rand() % 2; // 0 for horizontal, 1 for vertical
            
            for (int j = 0; j < wallLength; j++) {
                int x = startX + (direction == 0 ? j : 0);
                int y = startY + (direction == 1 ? j : 0);
                if (x < WIDTH - 1 && y < HEIGHT - 1) {
                    map[y][x] = WALL;
                }
            }
            
            // Create openings/doorways
            if (wallLength > 3 && wallLength < 10) {
                int doorPosition = rand() % wallLength;
                int x = startX + (direction == 0 ? doorPosition : 0);
                int y = startY + (direction == 1 ? doorPosition : 0);
                if (x < WIDTH - 1 && y < HEIGHT - 1) {
                    map[y][x] = FLOOR;
                }
            }
        }
        
        // Create some rooms
        int numRooms = 3 + dungeonLevel;
        for (int i = 0; i < numRooms; i++) {
            int roomWidth = 5 + rand() % 8;
            int roomHeight = 5 + rand() % 5;
            int startX = 2 + rand() % (WIDTH - roomWidth - 2);
            int startY = 2 + rand() % (HEIGHT - roomHeight - 2);
            
            // Clear room area
            for (int y = startY; y < startY + roomHeight; y++) {
                for (int x = startX; x < startX + roomWidth; x++) {
                    if (x < WIDTH - 1 && y < HEIGHT - 1 && x > 0 && y > 0) {
                        map[y][x] = FLOOR;
                    }
                }
            }
            
            // Create walls around room
            for (int y = startY; y < startY + roomHeight; y++) {
                if (y < HEIGHT - 1 && y > 0) {
                    map[y][startX] = WALL;
                    map[y][startX + roomWidth - 1] = WALL;
                }
            }
            for (int x = startX; x < startX + roomWidth; x++) {
                if (x < WIDTH - 1 && x > 0) {
                    map[startY][x] = WALL;
                    map[startY + roomHeight - 1][x] = WALL;
                }
            }
            
            // Add doors
            int doorSide = rand() % 4;
            int doorPos;
            switch (doorSide) {
                case 0: // North
                    doorPos = startX + 1 + rand() % (roomWidth - 2);
                    map[startY][doorPos] = FLOOR;
                    break;
                case 1: // East
                    doorPos = startY + 1 + rand() % (roomHeight - 2);
                    map[doorPos][startX + roomWidth - 1] = FLOOR;
                    break;
                case 2: // South
                    doorPos = startX + 1 + rand() % (roomWidth - 2);
                    map[startY + roomHeight - 1][doorPos] = FLOOR;
                    break;
                case 3: // West
                    doorPos = startY + 1 + rand() % (roomHeight - 2);
                    map[doorPos][startX] = FLOOR;
                    break;
            }
            
            // Add some items in the room
            if (rand() % 3 == 0) {
                int itemX = startX + 1 + rand() % (roomWidth - 2);
                int itemY = startY + 1 + rand() % (roomHeight - 2);
                if (rand() % 2 == 0) {
                    map[itemY][itemX] = HEALTH;
                } else {
                    map[itemY][itemX] = GOLD;
                }
            }
        }
        
        // Place player in a safe spot
        player.x = 2 + rand() % 5;
        player.y = 2 + rand() % 5;
        
        // Place key, must be far from player
        int keyX, keyY;
        do {
            keyX = 5 + rand() % (WIDTH - 10);
            keyY = 5 + rand() % (HEIGHT - 10);
        } while (map[keyY][keyX] != FLOOR || abs(keyX - player.x) + abs(keyY - player.y) < 15);
        map[keyY][keyX] = KEY;
        
        // Place door, must be far from both player and key
        int doorX, doorY;
        do {
            doorX = 5 + rand() % (WIDTH - 10);
            doorY = 5 + rand() % (HEIGHT - 10);
        } while (map[doorY][doorX] != FLOOR || 
                 (abs(doorX - player.x) + abs(doorY - player.y) < 15) || 
                 (abs(doorX - keyX) + abs(doorY - keyY) < 15));
        map[doorY][doorX] = DOOR;
        
        // Place stairs to next level
        if (dungeonLevel < 5) {
            int stairsX, stairsY;
            do {
                stairsX = 5 + rand() % (WIDTH - 10);
                stairsY = 5 + rand() % (HEIGHT - 10);
            } while (map[stairsY][stairsX] != FLOOR || 
                     (abs(stairsX - player.x) + abs(stairsY - player.y) < 15));
            map[stairsY][stairsX] = STAIRS;
        }
        
        // Place items
        placeItems();
        
        // Spawn enemies appropriate to level
        spawnEnemies(dungeonLevel);
        
        // Reset key if changing levels
        if (dungeonLevel > 1) {
            player.hasKey = false;
        }
        
        addMessage("Welcome to dungeon level " + to_string(dungeonLevel) + "!");
    }

    void placeItems() {
        // Health potions
        int numHealthPotions = 3 + rand() % 3;
        for (int i = 0; i < numHealthPotions; i++) {
            int x, y;
            do {
                x = 1 + rand() % (WIDTH - 2);
                y = 1 + rand() % (HEIGHT - 2);
            } while (map[y][x] != FLOOR);
            map[y][x] = HEALTH;
        }
        
        // Gold
        int numGold = 5 + rand() % 5;
        for (int i = 0; i < numGold; i++) {
            int x, y;
            do {
                x = 1 + rand() % (WIDTH - 2);
                y = 1 + rand() % (HEIGHT - 2);
            } while (map[y][x] != FLOOR);
            map[y][x] = GOLD;
        }
        
        // Weapons
        int numWeapons = 1 + player.dungeonLevel / 2;
        for (int i = 0; i < numWeapons; i++) {
            int x, y;
            do {
                x = 1 + rand() % (WIDTH - 2);
                y = 1 + rand() % (HEIGHT - 2);
            } while (map[y][x] != FLOOR);
            map[y][x] = WEAPON;
        }
        
        // Armor
        int numArmor = player.dungeonLevel / 2;
        for (int i = 0; i < numArmor; i++) {
            int x, y;
            do {
                x = 1 + rand() % (WIDTH - 2);
                y = 1 + rand() % (HEIGHT - 2);
            } while (map[y][x] != FLOOR);
            map[y][x] = ARMOR;
        }
        
        // Traps
        int numTraps = 2 + player.dungeonLevel;
        for (int i = 0; i < numTraps; i++) {
            int x, y;
            do {
                x = 1 + rand() % (WIDTH - 2);
                y = 1 + rand() % (HEIGHT - 2);
            } while (map[y][x] != FLOOR || (abs(x - player.x) + abs(y - player.y) < 5));
            map[y][x] = TRAP;
        }
    }

    void spawnEnemies(int dungeonLevel) {
        // Number of enemies scales with dungeon level
        int numSlimes = 4 + dungeonLevel;
        int numGoblins = 2 + dungeonLevel;
        int numTrolls = dungeonLevel / 2;
        
        // Spawn slimes
        for (int i = 0; i < numSlimes; i++) {
            int x, y;
            do {
                x = 1 + rand() % (WIDTH - 2);
                y = 1 + rand() % (HEIGHT - 2);
            } while (map[y][x] != FLOOR || 
                    (abs(x - player.x) + abs(y - player.y) < 10) ||
                    isEnemyAt(x, y));
            enemies.push_back(new Slime(x, y));
        }
        
        // Spawn goblins
        for (int i = 0; i < numGoblins; i++) {
            int x, y;
            do {
                x = 1 + rand() % (WIDTH - 2);
                y = 1 + rand() % (HEIGHT - 2);
            } while (map[y][x] != FLOOR || 
                    (abs(x - player.x) + abs(y - player.y) < 10) ||
                    isEnemyAt(x, y));
            enemies.push_back(new Goblin(x, y));
        }
        
        // Spawn trolls
        for (int i = 0; i < numTrolls; i++) {
            int x, y;
            do {
                x = 1 + rand() % (WIDTH - 2);
                y = 1 + rand() % (HEIGHT - 2);
            } while (map[y][x] != FLOOR || 
                    (abs(x - player.x) + abs(y - player.y) < 10) ||
                    isEnemyAt(x, y));
            enemies.push_back(new Troll(x, y));
        }
    }
    
    bool isEnemyAt(int x, int y) const {
        for (const auto& enemy : enemies) {
            if (enemy->x == x && enemy->y == y) return true;
        }
        return false;
    }
    
    Enemy* getEnemyAt(int x, int y) {
        for (auto& enemy : enemies) {
            if (enemy->x == x && enemy->y == y) return enemy;
        }
        return nullptr;
    }

    void setConsoleColor(Color textColor, Color bgColor = BLACK) {
        SetConsoleTextAttribute(consoleHandle, textColor + (bgColor << 4));
    }

    void resetConsoleColor() {
        SetConsoleTextAttribute(consoleHandle, WHITE);
    }

    void draw() {
        system("cls");
        
        // Draw HUD at top
        setConsoleColor(LIGHTCYAN);
        cout << "=== DUNGEON CRAWLER Level " << player.dungeonLevel << " ===" << endl;
        
        // Draw map
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                if (x == player.x && y == player.y) {
                    setConsoleColor(YELLOW);
                    cout << PLAYER;
                } else {
                    // Check for enemy
                    bool isEnemyPos = false;
                    for (const auto& enemy : enemies) {
                        if (x == enemy->x && y == enemy->y) {
                            switch (enemy->symbol) {
                                case SLIME:
                                    setConsoleColor(GREEN);
                                    break;
                                case GOBLIN:
                                    setConsoleColor(LIGHTRED);
                                    break;
                                case TROLL:
                                    setConsoleColor(RED);
                                    break;
                                default:
                                    setConsoleColor(LIGHTGRAY);
                            }
                            cout << enemy->symbol;
                            isEnemyPos = true;
                            break;
                        }
                    }
                    
                    if (!isEnemyPos) {
                        // Set color based on tile type
                        switch (map[y][x]) {
                            case WALL:
                                setConsoleColor(DARKGRAY);
                                break;
                            case FLOOR:
                                setConsoleColor(LIGHTGRAY);
                                break;
                            case KEY:
                                setConsoleColor(YELLOW);
                                break;
                            case DOOR:
                                setConsoleColor(BROWN);
                                break;
                            case HEALTH:
                                setConsoleColor(LIGHTGREEN);
                                break;
                            case WEAPON:
                                setConsoleColor(CYAN);
                                break;
                            case ARMOR:
                                setConsoleColor(BLUE);
                                break;
                            case TRAP:
                                setConsoleColor(RED);
                                break;
                            case GOLD:
                                setConsoleColor(YELLOW);
                                break;
                            case STAIRS:
                                setConsoleColor(MAGENTA);
                                break;
                            default:
                                setConsoleColor(WHITE);
                        }
                        cout << map[y][x];
                    }
                }
            }
            cout << endl;
        }

        // Draw player stats
        setConsoleColor(WHITE);
        cout << "\nHealth: " << player.health << "/" << player.maxHealth 
             << " | Level: " << player.level 
             << " | XP: " << player.experience << "/" << player.experienceToLevel 
             << " | Gold: " << player.gold 
             << " | Score: " << player.score << endl;
        
        cout << "Weapon: " << (player.equippedWeapon ? player.equippedWeapon->name : "None") 
             << " (ATK: " << player.getTotalAttack() << ")" 
             << " | Armor: " << (player.equippedArmor ? player.equippedArmor->name : "None") 
             << " (DEF: " << player.getTotalDefense() << ")" 
             << " | Key: " << (player.hasKey ? "YES" : "NO") << endl;
        
        // Draw messages
        setConsoleColor(LIGHTCYAN);
        cout << "\n--- Messages ---" << endl;
        setConsoleColor(WHITE);
        if (messages.empty()) {
            cout << "No messages yet." << endl;
        } else {
            for (const auto& msg : messages) {
                cout << msg << endl;
            }
        }
        
        // Draw controls
        setConsoleColor(LIGHTCYAN);
        cout << "\n--- Controls ---" << endl;
        setConsoleColor(WHITE);
        cout << "Move: WASD | Attack: Space | Inventory: I | Use Health Potion: H | Quit: Q" << endl;
        
        resetConsoleColor();
    }
    
    void addMessage(const string& message) {
        messages.push_back(message);
        if (messages.size() > maxMessages) {
            messages.erase(messages.begin());
        }
    }

    void playerAttack() {
        bool hitEnemy = false;
        int x = player.x;
        int y = player.y;
        
        // Check all adjacent positions for enemies
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue; // Skip player's own position
                
                int targetX = x + dx;
                int targetY = y + dy;
                Enemy* target = getEnemyAt(targetX, targetY);
                
                if (target) {
                    hitEnemy = true;
                    int damage = player.getTotalAttack();
                    // Chance for critical hit
                    if (rand() % 10 == 0) {
                        damage *= 2;
                        addMessage("Critical hit! You strike the " + target->name + " for " + to_string(damage) + " damage!");
                    } else {
                        addMessage("You hit the " + target->name + " for " + to_string(damage) + " damage!");
                    }
                    
                    target->takeDamage(damage);
                    
                    // Weapon durability
                    if (player.equippedWeapon && player.equippedWeapon->durability > 0) {
                        player.equippedWeapon->durability--;
                        if (player.equippedWeapon->durability <= 0) {
                            addMessage("Your " + player.equippedWeapon->name + " broke!");
                            delete player.equippedWeapon;
                            player.equippedWeapon = new Weapon("Fists", 3, -1); // -1 for infinite durability
                        }
                    }
                    break;
                }
            }
            if (hitEnemy) break;
        }

        // Remove dead enemies
        auto it = enemies.begin();
        while (it != enemies.end()) {
            if ((*it)->health <= 0) {
                addMessage("You defeated the " + (*it)->name + "! Gained " + 
                           to_string((*it)->experienceValue) + " XP and " + 
                           to_string((*it)->goldValue) + " gold.");
                player.gainExperience((*it)->experienceValue);
                player.gold += (*it)->goldValue;
                player.score += (*it)->experienceValue * 10;
                delete *it;
                it = enemies.erase(it);
            } else {
                ++it;
            }
        }

        if (!hitEnemy) {
            addMessage("You swing at nothing!");
        }
    }
    
    void showInventory() {
        system("cls");
        cout << "=== INVENTORY ===" << endl;
        cout << "Items: " << player.inventory.size() << "/" << player.inventorySize << endl << endl;
        
        if (player.inventory.empty()) {
            cout << "Your inventory is empty." << endl;
        } else {
            for (size_t i = 0; i < player.inventory.size(); i++) {
                cout << (i + 1) << ". " << player.inventory[i]->name << " - " 
                     << player.inventory[i]->description << endl;
            }
            
            cout << "\nEnter the number of the item to use, or 0 to return: ";
            int choice;
            cin >> choice;

            if (choice > 0 && choice <= player.inventory.size()) {
                player.useItem(choice - 1);
                addMessage("You used " + player.inventory[choice - 1]->name);
            }
        }
        
        cout << "\nPress any key to return to the game...";
        _getch();
    }
    
    void useHealthPotion() {
        // Find a health potion in inventory
        for (size_t i = 0; i < player.inventory.size(); i++) {
            if (dynamic_cast<HealthPotion*>(player.inventory[i])) {
                player.useItem(i);
                addMessage("You used a Health Potion. Health restored!");
                return;
            }
        }
        addMessage("You don't have any health potions!");
    }

    void update() {
        turns++;
        
        // Check items at player's position
        switch (map[player.y][player.x]) {
            case KEY:
                player.hasKey = true;
                map[player.y][player.x] = FLOOR;
                addMessage("You picked up the key!");
                break;
                
            case HEALTH:
                map[player.y][player.x] = FLOOR;
                // Add potion to inventory
                player.addToInventory(new HealthPotion(20 + rand() % 21)); // 20-40 healing
                addMessage("You found a health potion!");
                break;
                
            case GOLD:
                {
                    int goldAmount = 5 + rand() % (10 * player.dungeonLevel);
                    player.gold += goldAmount;
                    player.score += goldAmount;
                    map[player.y][player.x] = FLOOR;
                    addMessage("You found " + to_string(goldAmount) + " gold!");
                }
                break;
                
            case WEAPON:
                {
                    // Generate a weapon based on dungeon level
                    string weaponName;
                    int damage, durability;
                    
                    int weaponType = rand() % 5;
                    switch (weaponType) {
                        case 0:
                            weaponName = "Sword";
                            damage = 8 + player.dungeonLevel * 2;
                            durability = 50 + player.dungeonLevel * 5;
                            break;
                        case 1:
                            weaponName = "Axe";
                            damage = 10 + player.dungeonLevel * 2;
                            durability = 40 + player.dungeonLevel * 4;
                            break;
                        case 2:
                            weaponName = "Mace";
                            damage = 12 + player.dungeonLevel * 2;
                            durability = 35 + player.dungeonLevel * 3;
                            break;
                        case 3:
                            weaponName = "Spear";
                            damage = 9 + player.dungeonLevel * 2;
                            durability = 45 + player.dungeonLevel * 4;
                            break;
                        case 4:
                            weaponName = "Dagger";
                            damage = 6 + player.dungeonLevel * 2;
                            durability = 60 + player.dungeonLevel * 6;
                            break;
                    }
                    
                    // Add quality prefix
                    string prefix;
                    int qualityBonus = rand() % 4;
                    switch (qualityBonus) {
                        case 0: prefix = "Rusty"; damage -= 2; break;
                        case 1: prefix = "Normal"; break;
                        case 2: prefix = "Sharp"; damage += 2; break;
                        case 3: prefix = "Masterwork"; damage += 4; durability += 10; break;
                    }
                    
                    Weapon* newWeapon = new Weapon(prefix + " " + weaponName, damage, durability);
                    
                    // Compare with current weapon
                    string currentWeaponInfo = player.equippedWeapon ? 
                        player.equippedWeapon->name + " (DMG: " + to_string(player.equippedWeapon->damage) + ")" : "None";
                    
                    addMessage("Found: " + newWeapon->name + " (DMG: " + to_string(newWeapon->damage) + 
                              "). Current: " + currentWeaponInfo);
                    
                    // Ask player if they want to equip the new weapon
                    draw();
                    cout << "\nEquip " << newWeapon->name << "? (Y/N): ";
                    char choice = _getch();
                    if (tolower(choice) == 'y') {
                        player.equipWeapon(newWeapon);
                        addMessage("Equipped " + newWeapon->name + "!");
                    } else {
                        delete newWeapon;
                        addMessage("You leave the " + weaponName + " behind.");
                    }
                    
                    map[player.y][player.x] = FLOOR;
                }
                break;
                
            case ARMOR:
                {
                    // Generate armor based on dungeon level
                    string armorName;
                    int defense;
                    
                    int armorType = rand() % 4;
                    switch (armorType) {
                        case 0:
                            armorName = "Leather Armor";
                            defense = 3 + player.dungeonLevel;
                            break;
                        case 1:
                            armorName = "Chain Mail";
                            defense = 5 + player.dungeonLevel;
                            break;
                        case 2:
                            armorName = "Plate Armor";
                            defense = 7 + player.dungeonLevel;
                            break;
                        case 3:
                            armorName = "Scale Mail";
                            defense = 4 + player.dungeonLevel;
                            break;
                    }
                    
                    // Add quality prefix
                    string prefix;
                    int qualityBonus = rand() % 4;
                    switch (qualityBonus) {
                        case 0: prefix = "Tattered"; defense -= 1; break;
                        case 1: prefix = "Standard"; break;
                        case 2: prefix = "Reinforced"; defense += 1; break;
                        case 3: prefix = "Mastercraft"; defense += 2; break;
                    }
                    
                    Armor* newArmor = new Armor(prefix + " " + armorName, defense);
                    
                    // Compare with current armor
                    string currentArmorInfo = player.equippedArmor ? 
                        player.equippedArmor->name + " (DEF: " + to_string(player.equippedArmor->defense) + ")" : "None";
                    
                    addMessage("Found: " + newArmor->name + " (DEF: " + to_string(newArmor->defense) + 
                              "). Current: " + currentArmorInfo);
                    
                    // Ask player if they want to equip the new armor
                    draw();
                    cout << "\nEquip " << newArmor->name << "? (Y/N): ";
                    char choice = _getch();
                    if (tolower(choice) == 'y') {
                        player.equipArmor(newArmor);
                        addMessage("Equipped " + newArmor->name + "!");
                    } else {
                        delete newArmor;
                        addMessage("You leave the " + armorName + " behind.");
                    }
                    
                    map[player.y][player.x] = FLOOR;
                }
                break;
                
            case TRAP:
                {
                    // Different trap effects
                    int trapType = rand() % 3;
                    switch (trapType) {
                        case 0: // Damage trap
                            {
                                int damage = 5 + rand() % (5 * player.dungeonLevel);
                                player.health -= damage;
                                addMessage("You stepped on a spike trap! Took " + to_string(damage) + " damage!");
                            }
                            break;
                        case 1: // Poison trap
                            {
                                int damage = 3 + player.dungeonLevel;
                                player.health -= damage;
                                addMessage("You triggered a poison gas trap! Took " + to_string(damage) + " damage!");
                            }
                            break;
                        case 2: // Alarm trap
                            {
                                addMessage("You triggered an alarm! Nearby enemies are alerted!");
                                // Make nearby enemies move faster towards player
                                for (auto& enemy : enemies) {
                                    if (abs(enemy->x - player.x) + abs(enemy->y - player.y) < 10) {
                                        enemy->moveCooldown = 0;
                                    }
                                }
                            }
                            break;
                    }
                    map[player.y][player.x] = FLOOR; // Trap is triggered only once
                }
                break;
                
            case DOOR:
                if (player.hasKey) {
                    map[player.y][player.x] = FLOOR;
                    player.hasKey = false;
                    player.score += 100 * player.dungeonLevel;
                    addMessage("You unlocked the door! +100 score points!");
                } else {
                    addMessage("The door is locked. You need a key!");
                }
                break;
                
            case STAIRS:
                // Ask player if they want to go to the next level
                draw();
                cout << "\nDescend to the next level? (Y/N): ";
                char choice = _getch();
                if (tolower(choice) == 'y') {
                    player.dungeonLevel++;
                    initializeMap(player.dungeonLevel);
                } else {
                    addMessage("You decide to stay on this level for now.");
                }
                break;
        }

        // Check if player died
        if (player.health <= 0) {
            gameOver = true;
            system("cls");
            setConsoleColor(RED);
            cout << "\n\n";
            cout << "  #####     #    #     # #######    ####### #     # ####### ######  \n";
            cout << " #     #   # #   ##   ## #          #     # #     # #       #     # \n";
            cout << " #        #   #  # # # # #          #     # #     # #       #     # \n";
            cout << " #  #### #     # #  #  # #####      #     # #     # #####   ######  \n";
            cout << " #     # ####### #     # #          #     #  #   #  #       #   #   \n";
            cout << " #     # #     # #     # #          #     #   # #   #       #    #  \n";
            cout << "  #####  #     # #     # #######    #######    #    ####### #     # \n";
            cout << "\n\n";

            resetConsoleColor();
            cout << "You died on dungeon level " << player.dungeonLevel << "!\n";
            cout << "Final score: " << player.score << endl;
            cout << "Gold collected: " << player.gold << endl;
            cout << "Player level: " << player.level << endl;
            cout << "Enemies defeated: " << (player.score / 10) - (player.gold) - (100 * (player.dungeonLevel - 1)) << endl;
            cout << "\nPress any key to exit...";
            _getch();
            return;
        }

        // Enemy movement and combat
        for (auto& enemy : enemies) {
            // Reset attack flag at the start of each turn
            enemy->hasAttacked = false;

            // Only move if not adjacent to player
            if (!enemy->isAdjacent(player.x, player.y)) {
                enemy->move(player.x, player.y, map);
            }
            
            // Check if enemy can attack player
            if (enemy->isAdjacent(player.x, player.y) && !enemy->hasAttacked) {
                int damage = enemy->attackPlayer(player);
                addMessage("The " + enemy->name + " attacks you for " + to_string(damage) + " damage!");
            }
        }
        
        // Regenerate small amount of health every few turns
        if (turns % 10 == 0 && player.health < player.maxHealth) {
            player.health = min(player.maxHealth, player.health + 1);
        }
    }

    void run() {
        while (!gameOver) {
            draw();
            
            char input = _getch();
            switch(tolower(input)) {
                case 'w': player.move(0, -1, map); update(); break;
                case 's': player.move(0, 1, map); update(); break;
                case 'a': player.move(-1, 0, map); update(); break;
                case 'd': player.move(1, 0, map); update(); break;
                case ' ': playerAttack(); update(); break;
                case 'i': showInventory(); break;
                case 'h': useHealthPotion(); update(); break;
                case 'q': 
                    cout << "\nAre you sure you want to quit? (Y/N): ";
                    char choice = _getch();
                    if (tolower(choice) == 'y') {
                        gameOver = true;
                    }
                    break;
            }
        }
    }
};

int main() {
    // Set console code page to support extended ASCII characters
    SetConsoleCP(437);
    SetConsoleOutputCP(437);
    
    // Seed random number generator
    srand(static_cast<unsigned>(time(0)));
    
    // Show title screen
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(consoleHandle, LIGHTCYAN);
    cout << "\n\n";
    cout << " ########  ##     ## ##    ##  ######   ########  #######  ##    ## \n";
    cout << " ##     ## ##     ## ###   ## ##    ##  ##       ##     ## ###   ## \n";
    cout << " ##     ## ##     ## ####  ## ##        ##       ##     ## ####  ## \n";
    cout << " ##     ## ##     ## ## ## ## ##   #### ######   ##     ## ## ## ## \n";
    cout << " ##     ## ##     ## ##  #### ##    ##  ##       ##     ## ##  #### \n";
    cout << " ##     ## ##     ## ##   ### ##    ##  ##       ##     ## ##   ### \n";
    cout << " ########   #######  ##    ##  ######   ########  #######  ##    ## \n";
    cout << "                         CRAWLER                                    \n";
    cout << "\n\n";
    
    SetConsoleTextAttribute(consoleHandle, WHITE);
    cout << "Welcome to the Dungeon Crawler game!" << endl;
    cout << "Navigate the dangerous dungeons, defeat monsters, and collect treasures!" << endl;
    cout << "\nControls:" << endl;
    cout << "Move with WASD" << endl;
    cout << "Attack with SPACE" << endl;
    cout << "Open inventory with I" << endl;
    cout << "Use health potion with H" << endl;
    cout << "Quit with Q" << endl;
    
    cout << "\nLegend:" << endl;
    SetConsoleTextAttribute(consoleHandle, YELLOW);
    cout << "@";
    SetConsoleTextAttribute(consoleHandle, WHITE);
    cout << " - Player" << endl;
    
    SetConsoleTextAttribute(consoleHandle, GREEN);
    cout << "s";
    SetConsoleTextAttribute(consoleHandle, WHITE);
    cout << " - Slime" << endl;
    
    SetConsoleTextAttribute(consoleHandle, LIGHTRED);
    cout << "g";
    SetConsoleTextAttribute(consoleHandle, WHITE);
    cout << " - Goblin" << endl;
    
    SetConsoleTextAttribute(consoleHandle, RED);
    cout << "T";
    SetConsoleTextAttribute(consoleHandle, WHITE);
    cout << " - Troll" << endl;
    
    SetConsoleTextAttribute(consoleHandle, YELLOW);
    cout << "K";
    SetConsoleTextAttribute(consoleHandle, WHITE);
    cout << " - Key" << endl;
    
    SetConsoleTextAttribute(consoleHandle, BROWN);
    cout << "D";
    SetConsoleTextAttribute(consoleHandle, WHITE);
    cout << " - Door" << endl;
    
    SetConsoleTextAttribute(consoleHandle, LIGHTGREEN);
    cout << "+";
    SetConsoleTextAttribute(consoleHandle, WHITE);
    cout << " - Health Potion" << endl;
    
    SetConsoleTextAttribute(consoleHandle, CYAN);
    cout << "W";
    SetConsoleTextAttribute(consoleHandle, WHITE);
    cout << " - Weapon" << endl;
    
    SetConsoleTextAttribute(consoleHandle, BLUE);
    cout << "A";
    SetConsoleTextAttribute(consoleHandle, WHITE);
    cout << " - Armor" << endl;
    
    SetConsoleTextAttribute(consoleHandle, RED);
    cout << "^";
    SetConsoleTextAttribute(consoleHandle, WHITE);
    cout << " - Trap" << endl;
    
    SetConsoleTextAttribute(consoleHandle, YELLOW);
    cout << "$";
    SetConsoleTextAttribute(consoleHandle, WHITE);
    cout << " - Gold" << endl;
    
    SetConsoleTextAttribute(consoleHandle, MAGENTA);
    cout << ">";
    SetConsoleTextAttribute(consoleHandle, WHITE);
    cout << " - Stairs to next level" << endl;
    
    cout << "\nPress any key to start your adventure...";
    _getch();
    
    GameManager game;
    game.run();
    
    return 0;
}
