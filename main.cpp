//
//  main.cpp
//  Assignment_1_Fleet_Systems
//
//  Created by Arturo Sebastian García Soto on 13/06/26.
//

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <utility>


//Part 5 - Exception safety & RAII

class DockingException : public std::runtime_error {
public:
    DockingException(const std::string& msg) : std::runtime_error(msg) {}
};


// Prat 1 & 2 - Model the relationships | Inheritance, polymorphism & construction order

class Maneuver {
public:
    void perform() const { std::cout << "Maneuver executed\n"; }
};

class FuelTank {
public:
    FuelTank() { std::cout << "FuelTank constructed\n"; }
    ~FuelTank() { std::cout << "FuelTank destroyed\n"; }
    int fuelLevel = 100;
};

class Telemetry {
public:
    Telemetry() { std::cout << "Telemetry constructed\n"; }
    ~Telemetry() { std::cout << "Telemetry destroyed\n"; }
};

class Entity {
public:
    Entity() { std::cout << "Entity constructed\n"; }
    virtual ~Entity() { std::cout << "Entity destroyed\n"; }
};

class Module {
protected:
    std::string name;
    int powerDraw;
public:
    Module(std::string n, int p) : name(std::move(n)), powerDraw(p) {
        std::cout << "Module " << name << " constructed\n";
    }
    virtual ~Module() { std::cout << "Module " << name << " destroyed\n"; }
    virtual void activate() = 0;
};

class Engine final : public Module {
public:
    Engine(std::string n, int p) : Module(std::move(n), p) {
        std::cout << "Engine constructed\n";
    }
    ~Engine() override { std::cout << "Engine destroyed\n"; }
    void activate() override { std::cout << "Engine activating, Power: " << powerDraw << "\n"; }
};

class Shield final : public Module {
public:
    Shield(std::string n, int p) : Module(std::move(n), p) {}
    void activate() override { std::cout << "Shields up\n"; }
};


// Part 3 - A resource-owning class the hard way: Rule of 5

class CargoHold {
private:
    int* cargoIds;
    size_t size;
public:
    //Constructor
    CargoHold(size_t s) : size(s), cargoIds(new int[s]) { std::cout << "CargoHold allocated\n"; }
    
    //Destructor
    ~CargoHold() { delete[] cargoIds; std::cout << "CargoHold deallocated\n"; }
    
    //Copy Constructor
    CargoHold(const CargoHold& other) : size(other.size), cargoIds(new int[other.size]) {
        for(size_t i = 0; i < size; ++i) cargoIds[i] = other.cargoIds[i];
    }
    
    //Copy Assignment
    CargoHold& operator=(const CargoHold& other) {
        if (this != &other) {
            delete[] cargoIds;
            size = other.size;
            cargoIds = new int[size];
            for(size_t i = 0; i < size; ++i) cargoIds[i] = other.cargoIds[i];
        }
        return *this;
    }
    
    //Move Constructor
    CargoHold(CargoHold&& other) noexcept : cargoIds(other.cargoIds), size(other.size) {
        other.cargoIds = nullptr;
        other.size = 0;
    }
    
    //Move Assignment
    CargoHold& operator=(CargoHold&& other) noexcept {
        if (this != &other) {
            delete[] cargoIds;
            cargoIds = other.cargoIds;
            size = other.size;
            other.cargoIds = nullptr;
            other.size = 0;
        }
        return *this;
    }
    bool isEmpty() const { return cargoIds == nullptr; }
};


//Part 4 -  Modernize to the Rule of 0

class ModernCargoHold {
private:
    std::vector<int> cargoIds;
};

class Fleet;

class Spacecraft : public Entity {
private:
    FuelTank tank;
    Telemetry telem;
    Fleet* missionFleet;
    Module* activeModule;
    ModernCargoHold hold;

public:
    Spacecraft() : missionFleet(nullptr), activeModule(nullptr) {
        std::cout << "Spacecraft constructed\n";
    }
    ~Spacecraft() override {
        std::cout << "Spacecraft destroyed\n";
    }

    void setFleet(Fleet* f) { missionFleet = f; }
    void mountModule(Module* m) { activeModule = m; }
    
    void executeManeuver(Maneuver m) {
        m.perform();
    }

    void tick() {
        if (activeModule) activeModule->activate();
    }

    void dock() {
        if (tank.fuelLevel < 10) {
            throw DockingException("Insufficient fuel to initiate docking");
        }
        std::cout << "Docking successful\n";
    }
    
    void drainFuel() { tank.fuelLevel = 0; }
};

class Fleet {
private:
    std::vector<std::unique_ptr<Spacecraft>> ships;
public:
    void addShip(std::unique_ptr<Spacecraft> ship) {
        ship->setFleet(this);
        ships.push_back(std::move(ship));
    }
    void removeShip() {
        if (!ships.empty()) {
            std::cout << "Destroying one ship\n";
            ships.pop_back();
        }
    }
    size_t getFleetSize() const { return ships.size(); }
};


int main() {
    std::cout << "Part 3: Rule of 5 Demonstration\n";
    CargoHold original(5);
    CargoHold copy = original;
    CargoHold moved = std::move(original);
    std::cout << "Original is empty after move: " << (original.isEmpty() ? "True" : "False") << "\n\n";

    std::cout << "Part 5\n";
    Fleet starFleet;
    Engine warpDrive("WarpCore", 500);

    try {
        auto ship1 = std::make_unique<Spacecraft>();
        ship1->mountModule(&warpDrive);
        
        Maneuver orbit;
        ship1->executeManeuver(orbit);
        ship1->tick();
        
        ship1->drainFuel();
        ship1->dock();

    } catch (const DockingException& e) {
        std::cerr << "Caught Exception: " << e.what() << "\n";
    }

    std::cout << "\nPart 4: Fleet Teardown\n";
    starFleet.addShip(std::make_unique<Spacecraft>());
    starFleet.addShip(std::make_unique<Spacecraft>());
    
    std::cout << "Fleet size before: " << starFleet.getFleetSize() << "\n";
    starFleet.removeShip();
    std::cout << "Fleet size after: " << starFleet.getFleetSize() << "\n";
    std::cout << "End of simulation.\n";
    return 0;
}
