#include "ecs/types.hpp"

#include <chrono>

namespace ecs{


class ISystem{
public:
    ISystem(const ISystem&) = delete;
    ISystem& operator=(const ISystem&) = delete;

    // std::chrono::nanoseconds are used because t1-t0 (std::chrono::high_resolution_clock::now()) yields this type
    virtual void execute(std::chrono::nanoseconds dt) = 0;
private:
    const SystemID id{};
};


}