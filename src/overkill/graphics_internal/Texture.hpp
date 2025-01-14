#pragma once
#include <vector>
#include <glad/glad.h>

namespace OK
{
    //TODO: Cleanup, don't inline everything

    struct RawTexture
    {

        uint16_t width = 0u;
        uint16_t height = 0u;
        uint16_t channels = 4u;
        std::vector<unsigned char> pixels;
    };

    struct TextureSamplingOptions
    {
        GLenum
            minFilter = GL_NEAREST_MIPMAP_LINEAR,
            maxFilter = GL_LINEAR;
        GLenum
            sWrap = GL_REPEAT,
            tWrap = GL_REPEAT;

        TextureSamplingOptions() = default;
        TextureSamplingOptions(
            GLenum filterMode, GLenum wrapMode
        ) : minFilter{ filterMode },
            maxFilter{ filterMode },
            sWrap{ wrapMode },
            tWrap{ wrapMode }
        {}

        TextureSamplingOptions(
            GLenum minFilterMode,
            GLenum maxFilterMode,
            GLenum sWrapMode,
            GLenum tWrapMode
        ) : minFilter{ minFilterMode },
            maxFilter{ maxFilterMode },
            sWrap{ sWrapMode },
            tWrap{ tWrapMode }
        {}
    };

    class TextureBase // TODO: Remove TextureBase -> we only really need Texture<TextureType>
    {
    protected:
        GLuint id;
        uint16_t width, height, channels;
        TextureSamplingOptions samplingOptions;
    public:
        TextureBase() = default;
        TextureBase(uint16_t width, uint16_t height, uint16_t channels, TextureSamplingOptions options = TextureSamplingOptions());

        inline void clean() { glDeleteTextures(1, &id); }

        inline const bool valid() const { return id != 0; }
        inline const GLuint getID() const { return id; }

        inline const uint16_t const getWidth() { return width; }
        inline const uint16_t const getHeight() { return height; }
        inline const uint16_t const getChannels() { return channels; }

        // TODO: Do we need these to be virtual?
        virtual void bind(GLuint slot = 0) const = 0; 
        virtual void unbind() const = 0;
        virtual void setSamplingOptions(TextureSamplingOptions options) = 0;
        virtual RawTexture fetchRaw() const = 0;
    };

    template <GLenum TextureType>
    class Texture : public TextureBase
    {
    protected:
    public:
        Texture() = default; //TODO: can we maybe put the sampling options into base?
        Texture(uint16_t width, uint16_t height, uint16_t channels, TextureSamplingOptions options = TextureSamplingOptions())
            : TextureBase(width, height, channels) {

        }

        /// <summary>
        /// Create a texture object filled with the data passed
        /// </summary>
        Texture(RawTexture raw, TextureSamplingOptions options = TextureSamplingOptions())
            : TextureBase(raw.width, raw.height, raw.channels)
        {
            glGenTextures(1, &id);
            glBindTexture(TextureType, id);
            glTexImage2D(TextureType, 0, GL_RGBA, width,  height, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw.pixels.data());
            glGenerateMipmap(TextureType);
            glTexParameteri(TextureType, GL_TEXTURE_MIN_FILTER, options.minFilter);
            glTexParameteri(TextureType, GL_TEXTURE_MAG_FILTER, options.maxFilter);

            glTexParameteri(TextureType, GL_TEXTURE_WRAP_S, options.sWrap);
            glTexParameteri(TextureType, GL_TEXTURE_WRAP_T, options.tWrap);
        }


        inline void bind(GLuint slot = 0) const override {
            glBindTextureUnit(slot, id);
            //glActiveTexture(GL_TEXTURE0 + slot);
            //glBindTexture(TextureType, id);
        }
        inline void unbind() const override {
            glBindTexture(TextureType, 0);
        }

        inline void setSamplingOptions(TextureSamplingOptions options) override
        {
            glBindTexture(TextureType, id);

            glTexParameteri(TextureType, GL_TEXTURE_MIN_FILTER, options.minFilter);
            glTexParameteri(TextureType, GL_TEXTURE_MAG_FILTER, options.maxFilter);

            glTexParameteri(TextureType, GL_TEXTURE_WRAP_S, options.sWrap);
            glTexParameteri(TextureType, GL_TEXTURE_WRAP_T, options.tWrap);

            //NOTE: Might be worth restoring previous bound texture...
        }


        /// <summary>
        /// fetch the uploaded texture from the GPU-memory as raw pixels.
        /// </summary>
        inline RawTexture fetchRaw() const override
        {
            RawTexture data = { width, height, 4u, std::vector<uint8_t>(width * height * channels) };
            glGetTextureImage(TextureType, 0, GL_RGBA, GL_UNSIGNED_BYTE, width * height, &(data.pixels)[0]);
            return data;
        }
    };

    using Texture2D = Texture<GL_TEXTURE_2D>; //declare the type as an implementation

    class Texture2DArray final : public Texture<GL_TEXTURE_2D_ARRAY>
    {
    private:
        uint16_t depth/*, sliceDimension*/;
    public:
        Texture2DArray(RawTexture raw, TextureSamplingOptions options = TextureSamplingOptions());
        inline uint16_t getDepth() { return depth; }

        inline RawTexture fetchRaw(int elementX, int elementY) const
        {
            RawTexture data = { width, height, 4u, std::vector<uint8_t>(width * height * channels) };
            glGetTextureImage(id, 0, GL_RGBA, GL_UNSIGNED_BYTE, width * height, &(data.pixels)[0]);
            return data;
        }
    };
}