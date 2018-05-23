#include "geometries/vertex_t.hpp"

namespace vpsk {


    vertex_t::vertex_t(const glm::vec3& _pos, const glm::vec3& _norm, const glm::vec3& _tangent, const glm::vec2& _uv) : pos(_pos), normal(_norm), tangent(_tangent), uv(_uv) {}

    vertex_t::vertex_t(const vertex_t& other) noexcept : pos(other.pos), normal(other.normal), tangent(other.tangent), uv(other.uv) {}

    vertex_t::vertex_t(vertex_t&& other) noexcept : pos(std::move(other.pos)), normal(std::move(other.normal)), tangent(std::move(other.tangent)), uv(std::move(other.uv))  {}

    vertex_t& vertex_t::operator=(const vertex_t& other) noexcept {
        pos = other.pos;
        normal = other.normal;
        tangent = other.tangent;
        uv = other.uv;
        return *this;
    }

    vertex_t& vertex_t::operator=(vertex_t&& other) noexcept {
        pos = std::move(other.pos);
        normal = std::move(other.normal);
        tangent = std::move(other.tangent);
        uv = std::move(other.uv);
        return *this;
    }

    bool vertex_t::operator==(const vertex_t& other) const noexcept  {
        return (pos == other.pos) && (normal == other.normal) && (uv == other.uv);
    }


}