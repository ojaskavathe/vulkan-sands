#include <type_traits>
enum class ElementType {
  Air = 0x00,
  Sand = 0x01,
  Water = 0x02,
};

inline ElementType operator | (ElementType lhs, ElementType rhs) {
  using T = std::underlying_type_t<ElementType>;
  return static_cast<ElementType>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
