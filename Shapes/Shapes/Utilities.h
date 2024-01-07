#ifndef DEM002_UTILITIES_H_H
#define DEM002_UTILITIES_H_H

#include <SFML/Graphics.hpp>
template<typename T>
void centerOrigin(T& t) {
    auto bounds = t.getLocalBounds();
    //auto center = sf::Vector2f{
    //        (bounds.width / 2.f) + bounds.left,
    //        (bounds.height / 2.f) + bounds.top };
    //t.setOrigin(center);
    t.setOrigin((bounds.width / 2.f) + bounds.left,
        (bounds.height / 2.f) + bounds.top);
}

// operator overloading
template <typename T>
std::ostream& operator<<(std::ostream& os, const sf::Vector2<T>& v) {
    os << "{" << v.x << ", " << v.y << "}";
    return os;
}
template <typename T>
std::ostream& operator<<(std::ostream& os, const sf::Rect<T>& r) {
    os << "{{" << r.left << ", " << r.top << "}, {" << r.width << ", " << r.height << "}";
    return os;
}
template <typename T>
void displayBounds(const T& t) {
    std::cout << "Local bounds : " << t.getLocalBounds() << "\n";
    std::cout << "Global bounds: " << t.getGlobalBounds() << "\n";
}

#endif //DEM002_UTILITIES_H_H

