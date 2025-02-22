#include <glad/glad.h>
#include <filesystem>
#include <array>
#include <stb_image.h>
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/Shader.hpp>
#include <engine/resources/ShaderCompiler.hpp>
#include <engine/resources/Skybox.hpp>
#include <engine/util/Errors.hpp>
#include <engine/util/Utils.hpp>

namespace engine::graphics {
    int32_t OpenGL::shader_type_to_opengl_type(resources::ShaderType type) {
        switch (type) {
        case resources::ShaderType::Vertex: return GL_VERTEX_SHADER;
        case resources::ShaderType::Fragment: return GL_FRAGMENT_SHADER;
        case resources::ShaderType::Geometry: return GL_GEOMETRY_SHADER;
        default: RG_SHOULD_NOT_REACH_HERE("Unhandled ShaderType");
        }
    }

    uint32_t OpenGL::generate_texture(const std::filesystem::path &path, bool flip_uvs) {
        uint32_t texture_id = 0;
        CHECKED_GL_CALL(glGenTextures, 1, &texture_id);

        int32_t width, height, nr_components;
        stbi_set_flip_vertically_on_load(flip_uvs);
        uint8_t *data = stbi_load(path.c_str(), &width, &height, &nr_components, 0);
        defer {
            stbi_image_free(data);
        };
        if (data) {
            int32_t format = texture_format(nr_components);

            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, texture_id);
            CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            CHECKED_GL_CALL(glGenerateMipmap, GL_TEXTURE_2D);

            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
            throw util::EngineError(util::EngineError::Type::AssetLoadingError,
                                    std::format("Failed to load texture {}", path.string()));
        }
        return texture_id;
    }

    int32_t OpenGL::texture_format(int32_t number_of_channels) {
        switch (number_of_channels) {
        case 1: return GL_RED;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default: RG_SHOULD_NOT_REACH_HERE("Unknown channels {}", number_of_channels);
        }
    }

    uint32_t OpenGL::init_skybox_cube() {
        static unsigned int skybox_vao = 0;
        if (skybox_vao != 0) {
            return skybox_vao;
        }
        float vertices[] = {
                // formatter: off
#include <skybox_vertices.include>
                // formatter: on
        };
        uint32_t skybox_vbo = 0;
        CHECKED_GL_CALL(glGenVertexArrays, 1, &skybox_vao);
        CHECKED_GL_CALL(glGenBuffers, 1, &skybox_vbo);
        CHECKED_GL_CALL(glBindVertexArray, skybox_vao);
        CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, skybox_vbo);
        CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
        CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
        CHECKED_GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0); // NOLINT
        return skybox_vao;
    }

    bool OpenGL::shader_compiled_successfully(uint32_t shader_id) {
        int success;
        CHECKED_GL_CALL(glGetShaderiv, shader_id, GL_COMPILE_STATUS, &success);
        return success;
    }

    uint32_t OpenGL::compile_shader(const std::string &shader_source,
                                    resources::ShaderType shader_type) {
        uint32_t shader_id             = CHECKED_GL_CALL(glCreateShader, shader_type_to_opengl_type(shader_type));
        const char *shader_source_cstr = shader_source.c_str();
        CHECKED_GL_CALL(glShaderSource, shader_id, 1, &shader_source_cstr, nullptr);
        CHECKED_GL_CALL(glCompileShader, shader_id);
        return shader_id;
    }

    std::string OpenGL::get_compilation_error_message(uint32_t shader_id) {
        char infoLog[512];
        CHECKED_GL_CALL(glGetShaderInfoLog, shader_id, 512, nullptr, infoLog);
        return infoLog;
    }

    std::string_view gl_call_error_description(GLenum error) {
        switch (error) {
        case GL_NO_ERROR: return
                    "GL_NO_ERROR: No error has been recorded. The value of this symbolic constant is guaranteed to be 0. ";
        case GL_INVALID_ENUM: return
                    "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.  ";
        case GL_INVALID_VALUE: return
                    "GL_INVALID_VALUE: A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.  ";
        case GL_INVALID_OPERATION: return
                    "GL_INVALID_OPERATION: The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.  ";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return
                    "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete."
                    "The offending command is ignored and has no other side effect than to set the error flag.";
        case GL_OUT_OF_MEMORY: return
                    "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded. . ";
        default: return "No Description";
        }
    }

    void OpenGL::assert_no_error(std::source_location location) {
        if (auto error = glGetError(); error != GL_NO_ERROR) {
            throw util::EngineError(util::EngineError::Type::OpenGLError,
                                    std::format("OpenGL call error: '{}'", gl_call_error_description(error)),
                                    location);
        };
    }

    uint32_t face_index(std::string_view name);

    uint32_t OpenGL::load_skybox_textures(const std::filesystem::path &path, bool flip_uvs) {
        RG_GUARANTEE(std::filesystem::is_directory(path),
                     "Directory '{}' doesn't exist. Please specify path to be a directory to where the cubemap textures are located. The cubemap textures should be named: right, left, top, bottom, front, back; by their respective faces in the cubemap.",
                     path.string())
        ;
        uint32_t texture_id;
        CHECKED_GL_CALL(glGenTextures, 1, &texture_id);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, texture_id);

        int width, height, nr_channels;
        for (const auto &file: std::filesystem::directory_iterator(path)) {
            stbi_set_flip_vertically_on_load(flip_uvs);
            unsigned char *data = stbi_load(absolute(file).c_str(), &width, &height, &nr_channels, 0);
            defer {
                stbi_image_free(data);
            };
            if (data) {
                uint32_t i = face_index(file.path().stem().c_str());
                CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB,
                                GL_UNSIGNED_BYTE,
                                data);
            } else {
                throw util::EngineError(util::EngineError::Type::AssetLoadingError,
                                        std::format("Failed to load skybox texture {}", path.string()));
            }
        }
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return texture_id;
    }

    void OpenGL::enable_depth_testing() {
        CHECKED_GL_CALL(glEnable, GL_DEPTH_TEST);
    }

    void OpenGL::disable_depth_testing() {
        CHECKED_GL_CALL(glDisable, GL_DEPTH_TEST);
    }

    void OpenGL::clear_buffers() {
        CHECKED_GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    unsigned int OpenGL::hdrFBO;
    unsigned int OpenGL::pingpongFBO[2];
    unsigned int OpenGL::pingpongColorbuffers[2];
    unsigned int OpenGL::colorBuffers[2];
    unsigned int OpenGL::quadVAO;
    unsigned int OpenGL::quadVBO;

    void OpenGL::begin_bloom() {
        CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, hdrFBO);
    }

    void OpenGL::initialize_bloom(int SCR_WIDTH, int SCR_HEIGHT, resources::Shader *shaderBlur,
                                  resources::Shader *shaderBloom) {
        quadVAO = 0;

        glGenFramebuffers(1, &hdrFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glGenTextures(2, colorBuffers);
        for (unsigned int i = 0; i < 2; i++) {
            glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0,
                         GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
        }

        unsigned int rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, attachments);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw util::EngineError(util::EngineError::Type::ConfigurationError, "Framebuffer not complete!");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glGenFramebuffers(2, pingpongFBO);
        glGenTextures(2, pingpongColorbuffers);
        for (unsigned int i = 0; i < 2; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
            glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                throw util::EngineError(util::EngineError::Type::ConfigurationError, "Framebuffer not complete!");
        }

        shaderBlur->use();
        shaderBlur->set_int("image", 0);
        shaderBloom->use();
        shaderBloom->set_int("scene", 0);
        shaderBloom->set_int("bloomBlur", 1);
    }

    void OpenGL::render_quad() {
        if (quadVAO == 0) {
            float quadVertices[] = {
                    // positions        // texture Coords
                    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                    1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            // setup plane VAO
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
        }
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    void OpenGL::end_bloom(resources::Shader *shaderBlur, resources::Shader *shaderBloom, float bloom, float exposure) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // 2. blur bright fragments with two-pass Gaussian Blur
        // --------------------------------------------------
        bool horizontal     = true, first_iteration = true;
        unsigned int amount = 10;

        shaderBlur->use();
        for (unsigned int i = 0; i < amount; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            shaderBlur->set_int("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
            // bind texture of other framebuffer (or scene if first iteration)
            render_quad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        // --------------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderBloom->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);

        render_quad();

        shaderBloom->set_int("bloom", bloom);
        shaderBloom->set_float("exposure", exposure);

        render_quad();
    }

    uint32_t face_index(std::string_view name) {
        if (name == "right") {
            return 0;
        } else if (name == "left") {
            return 1;
        } else if (name == "top") {
            return 2;
        } else if (name == "bottom") {
            return 3;
        } else if (name == "front") {
            return 4;
        } else if (name == "back") {
            return 5;
        } else {
            RG_SHOULD_NOT_REACH_HERE(
                    "Unknown face name: {}. The cubemap textures should be named: right, left, top, bottom, front, back; by their respective faces in the cubemap. The extension of the image file is ignored.",
                    name);
        }
    }

    int32_t stbi_number_of_channels_to_gl_format(int32_t number_of_channels) {
        switch (number_of_channels) {
        case 1: return GL_RED;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default: RG_SHOULD_NOT_REACH_HERE("Unknown channels {}", number_of_channels);
        }
    }

};
