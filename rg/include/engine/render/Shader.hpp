#ifndef MATF_RG_PROJECT_SHADER_HPP
#define MATF_RG_PROJECT_SHADER_HPP
#include <engine/util/Utils.hpp>
#include <string>
#include <utility>
#include <glm/glm.hpp>

namespace rg {
    using ShaderName = std::string;
    struct ShaderParsingResult;

    enum class ShaderType { Vertex, Fragment, Geometry };

    inline std::string_view to_string(ShaderType type) {
        switch (type) {
        case ShaderType::Vertex: return "vertex";
        case ShaderType::Fragment: return "fragment";
        case ShaderType::Geometry: return "geometry";
        default: RG_SHOULD_NOT_REACH_HERE("Unhandled shader type");
        }
    }

    class ShaderProgram {
        friend class ShaderController;
        friend class ShaderCompiler;

    public:
        void use() const;

        void destroy() const;

        unsigned id() const;

        void set_bool(const std::string &name, bool value) const;

        void set_int(const std::string &name, int value) const;

        void set_float(const std::string &name, float value) const;

        void set_vec2(const std::string &name, const glm::vec2 &value) const;

        void set_vec2(const std::string &name, float x, float y) const;

        void set_vec3(const std::string &name, const glm::vec3 &value) const;

        void set_vec3(const std::string &name, float x, float y, float z) const;

        void set_vec4(const std::string &name, const glm::vec4 &value) const;

        void set_vec4(const std::string &name, float x, float y, float z, float w);

        void set_mat2(const std::string &name, const glm::mat2 &mat) const;

        void set_mat3(const std::string &name, const glm::mat3 &mat) const;

        void set_mat4(const std::string &name, const glm::mat4 &mat) const;

    private:
        explicit ShaderProgram(unsigned shaderId);

        unsigned m_shaderId;
    };

} // namespace rg

#endif//MATF_RG_PROJECT_SHADER_HPP
