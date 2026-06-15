# Assignment 1: Fleet Systems - Lifecycles & Ownership

## How to Compile
Run this command in your terminal to compile the simulation:
`g++ -std=c++14 main.cpp -o fleet_sim`

---

## Part 1: Relationships & Ownership Table
Here is how I modeled the architecture.

| Object / System | Relationship | C++ Implementation & Justification |
| :--- | :--- | :--- |
| **Entity** | Inheritance (IS-A) | `Spacecraft` inherits from `Entity`. This allows polymorphic behavior so the mission display can track any ship as a generic entity. |
| **FuelTank & Telemetry** | Composition (HAS-A) | Implemented directly by value as member objects. They are internal parts of the ship, so they strictly share its lifecycle (they are born and die with the ship). |
| **Fleet & Module** | Aggregation (HAS-A) | Modeled using raw pointers (`Fleet*`, `Module*`). Since a module can be salvaged and the fleet survives even if a ship is destroyed, they don't share the same lifecycle. *(Note: Ownership of the ships by the Fleet is handled via `unique_ptr` later).* |
| **Maneuver** | Dependency (USES-A) | Passed as a simple parameter to a method. It's a one-shot command that executes and immediately vanishes from the stack. |

---

## Part 2: Construction, Destruction & Slicing

**Predicted Order:**
When a `Spacecraft` is created, C++ builds from the base class up, and then initializes the internal members:
1. `Entity` (Base)
2. `FuelTank` & `Telemetry` (Members)
3. `Spacecraft` (Derived)

When the ship is destroyed, the teardown happens in the **exact reverse order** (Spacecraft -> Telemetry -> FuelTank -> Entity) to guarantee no dependencies are left hanging.

**Avoiding Object Slicing:**
Slicing happens when you assign a derived object (like `Engine`) to a base class variable passed by value (`Module m = Engine(...);`). The specific "Engine" data gets chopped off, leaving only the base "Module" part. 
To fix this, I stored the modules as pointers (`Module*`) and gave the base `Module` class a `virtual` destructor. If the destructor wasn't virtual, deleting a module pointer would only clean up the base class, causing a memory leak by leaving the `Engine` parts in RAM.

---

## Part 4: Modernizing to the Rule of 0

For the `ModernCargoHold`, I replaced the manual raw heap array from Part 3 with a `std::vector<int>`. Since the vector automatically manages its own dynamic memory, I was able to delete all the custom destructors and copy/move constructors, perfectly applying the **Rule of 0**.

**Fleet Ownership:**
I used `std::vector<std::unique_ptr<Spacecraft>>` to manage the ships. I chose `unique_ptr` over a raw pointer to prevent manual `delete` mistakes, and over `shared_ptr` because the Fleet is the strict, single owner of the ships. There's no need for the overhead of reference counting here. When a ship is popped from the vector, `unique_ptr` guarantees it gets safely wiped from memory while the rest of the fleet survives.
