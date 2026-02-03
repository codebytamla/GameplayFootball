Must Update
volatile + platform-specific atomics → std::atomic<T> (refcounted.hpp/cpp — current code is technically UB)
Raw new/delete everywhere — memory leaks are likely; vectors of raw owning pointers (std::vector<Player*>, std::vector<Animation*>, etc.)
Suppressed compiler warnings — -Wno-unused-variable -Wno-sign-compare -Wno-strict-aliasing in CMakeLists.txt are hiding real bugs
Recommended Updates
Boost → std — boost::thread → std::thread, boost::mutex → std::mutex, boost::shared_ptr → std::shared_ptr, boost::filesystem → std::filesystem, boost::bind → lambdas. Drops the entire Boost dependency.
C++ standard — bump from C++14 to C++17 (needed for std::filesystem anyway)
CMake 3.5 → 3.15+ — current style uses global flags and deprecated FIND_PACKAGE patterns instead of modern target-based CMake
Custom RefCounted/intrusive_ptr → std::shared_ptr — hand-rolled ref counting is fragile
Manual lock/unlock in Lockable<T> → std::lock_guard / std::scoped_lock (RAII)
NULL / 0 for pointers → nullptr throughout
printf() calls (15+ in teamAIcontroller.cpp alone) → proper logging
Optional
typedef → using aliases
Add override / final / constexpr / noexcept keywords where appropriate
C-style arrays (char tmp[2048]) → std::array or std::string
Custom singleton → Meyer's singleton (thread-safe since C++11)
Clean up dead OpenGL fixed-function declarations in sdl_glfuncs.h (glBegin, glVertex, etc. — declared but unused)
Remove deprecated Text2D class (has a TODO saying exactly this)
Replace ODE physics wrappers — ODE is mostly unmaintained since ~2013, and the physics module isn't even compiled
OK / Leave As-Is
SDL2 — fine, actively maintained
OpenGL rendering core — already uses VAOs/VBOs/shaders (modern path)
OpenAL audio — works, no pressing reason to change
SQLite3 — stable, just moved to system install
Custom ball physics — simple, purpose-built, works well for the game
AI architecture (Eliza controller, team AI, mental image system) — gameplay logic, not a tech debt issue
Game data model (match, team, player structs) — functional, no urgency