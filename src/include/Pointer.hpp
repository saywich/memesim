#ifndef MEMESIM_POINTER_HPP_
#define MEMESIM_POINTER_HPP_

#include <type_traits>

namespace memesim {

template<typename P>
struct PointerTraits {
    static P Nullptr() { return P{}; }
    using RefT = decltype(std::declval<P>().operator*());
};

template<typename P>
struct PointerTraits<P*> {
    static constexpr P* Nullptr() { return nullptr; }
    using RefT = P&;
};

template<typename A, typename P>
class Pointer {
public:
    static_assert(std::is_integral_v<A>, "address must be of integral type");

    Pointer() : address_{}, pointer_{PointerTraits<P>::Nullptr()} {}
    Pointer(A address, P pointer) : address_{address}, pointer_{pointer} {}

    Pointer(const Pointer& other) = default;
    Pointer(Pointer&& other) = default;

    Pointer& operator=(const Pointer& other) = default;

    operator A() {
        return address_;
    }

    operator P() {
        return pointer_;
    }

    typename PointerTraits<P>::RefT operator*() {
        return *pointer_;
    }

    bool operator==(const Pointer& other) {
        return address_ == other.address_ && pointer_ == other.pointer_;
    }

private:
    A address_;
    P pointer_;
};

}

#endif // MEMESIM_POINTER_HPP_