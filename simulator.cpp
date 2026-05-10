#include <cstring>
#include <iostream>
#include <string>

#include <address.hpp>
#include <simulator.hpp>

namespace {

std::mutex cout_mutex;

void Worker(memesim::Process& process, int id) {
    memesim::VPointer ptr = process.Allocate(memesim::vaddress::kPageSize);
    memesim::VPointer ptr2 = process.Allocate(memesim::vaddress::kPageSize);
    
    char* memory = ptr;
    
    std::string message = "process " + std::to_string(id) + " says hello";
    std::strcpy(memory, message.c_str());

    {
        std::lock_guard lg{cout_mutex};
        std::cout
            << "pid=" << process.PidValue()
            << " vaddr=" << ptr.Address()
            << " text=\"" << static_cast<char*>(ptr) << "\"\n";
    }

    char* memory2 = ptr2;
    std::strcpy(memory2, message.c_str());

    {
        std::lock_guard lg{cout_mutex};
        std::cout
            << "pid=" << process.PidValue()
            << " vaddr=" << ptr2.Address()
            << " text=\"" << static_cast<char*>(ptr2) << "\"\n";
    }

    process.Deallocate(ptr, memesim::vaddress::kPageSize);
    process.Deallocate(ptr2, memesim::vaddress::kPageSize);
}

}

int main() {
    memesim::Simulator simulator{
        5,
        memesim::paddress::kPageSize * 16,
        "simulator.txt"
    };

    simulator.SpawnProcess([](memesim::Process& process) {
        Worker(process, 1);
    });

    simulator.SpawnProcess([](memesim::Process& process) {
        Worker(process, 2);
    });

    simulator.SpawnProcess([](memesim::Process& process) {
        Worker(process, 3);
    });

    simulator.JoinAll();
    return 0;
}
