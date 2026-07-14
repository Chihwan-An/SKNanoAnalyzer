#ifndef SKNANO_TEST_YAML_H
#define SKNANO_TEST_YAML_H

#include <cstddef>
#include <string>
#include <vector>

namespace YAML {
class Node {
public:
    using const_iterator = std::vector<Node>::const_iterator;
    Node operator[](const char *) const { return {}; }
    Node operator[](std::size_t) const { return {}; }
    bool IsDefined() const { return false; }
    std::size_t size() const { return children_.size(); }
    const_iterator begin() const { return children_.begin(); }
    const_iterator end() const { return children_.end(); }
    template <typename T> T as() const { return T{}; }
private:
    std::vector<Node> children_;
};
inline Node LoadFile(const std::string &) { return {}; }
} // namespace YAML

#endif
