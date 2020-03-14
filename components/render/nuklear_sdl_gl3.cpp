#include "nuklear_sdl_gl3.h"
#include "../../apps/freeablo/fagui/guimanager.h"
#include <glad/glad.h>
#include <iostream>
#include <misc/assert.h>
#include <misc/misc.h>
#include <misc/stringops.h>
#include <render/OpenGL/vertexarrayobjectopengl.h>
#include <render/renderinstance.h>
#include <render/vertextypes.h>
#include <string.h>

static const struct nk_draw_vertex_layout_element vertex_layout[] = {{NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct Render::NuklearVertex, position)},
                                                                     {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct Render::NuklearVertex, uv)},
                                                                     {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct Render::NuklearVertex, color)},
                                                                     {NK_VERTEX_LAYOUT_END}};
#define NK_SHADER_VERSION "#version 330\n"

void nk_sdl_device_create(nk_gl_device& dev, Render::RenderInstance& renderInstance)
{
    GLint status;

    std::string vertexShaderSource = Misc::StringUtils::readAsString(Misc::getResourcesPath().str() + "/shaders/gui.vert");
    std::string fragmentShaderSource = Misc::StringUtils::readAsString(Misc::getResourcesPath().str() + "/shaders/gui.frag");

    nk_buffer_init_default(&dev.cmds);
    dev.prog = glCreateProgram();
    dev.vert_shdr = glCreateShader(GL_VERTEX_SHADER);
    dev.frag_shdr = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar* srcPtr = nullptr;

    srcPtr = vertexShaderSource.c_str();
    glShaderSource(dev.vert_shdr, 1, &srcPtr, nullptr);

    srcPtr = fragmentShaderSource.c_str();
    glShaderSource(dev.frag_shdr, 1, &srcPtr, nullptr);

    glCompileShader(dev.vert_shdr);
    glCompileShader(dev.frag_shdr);
    glGetShaderiv(dev.vert_shdr, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        int length = 1024;
        std::vector<GLchar> errorLog(length);
        glGetShaderInfoLog(dev.vert_shdr, length, &length, &errorLog[0]);
        std::cout << "SHADER ERROR: << " << &errorLog[0] << std::endl;
    }

    release_assert(status == GL_TRUE);
    glGetShaderiv(dev.frag_shdr, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        int length = 1024;
        std::vector<GLchar> errorLog(length);
        glGetShaderInfoLog(dev.frag_shdr, length, &length, &errorLog[0]);
        std::cout << "SHADER ERROR: << " << &errorLog[0] << std::endl;
    }
    release_assert(status == GL_TRUE);
    glAttachShader(dev.prog, dev.vert_shdr);
    glAttachShader(dev.prog, dev.frag_shdr);
    glLinkProgram(dev.prog);
    glGetProgramiv(dev.prog, GL_LINK_STATUS, &status);
    release_assert(status == GL_TRUE);

    dev.uniform_hoverColor = glGetUniformLocation(dev.prog, "hoverColor");
    dev.uniform_checkerboarded = glGetUniformLocation(dev.prog, "checkerboarded");
    dev.uniform_imageSize = glGetUniformLocation(dev.prog, "imageSize");
    dev.uniform_atlasOffset = glGetUniformLocation(dev.prog, "atlasOffset");
    dev.uniform_atlasSize = glGetUniformLocation(dev.prog, "atlasSize");
    dev.uniform_tex = glGetUniformLocation(dev.prog, "Texture");
    dev.uniform_proj = glGetUniformLocation(dev.prog, "ProjMtx");
    dev.attrib_pos = glGetAttribLocation(dev.prog, "Position");
    dev.attrib_uv = glGetAttribLocation(dev.prog, "TexCoord");
    dev.attrib_col = glGetAttribLocation(dev.prog, "Color");

    dev.vertexArrayObject = renderInstance.createVertexArrayObject({0}, {Render::NuklearVertex::layout()}, 1).release();
}

/*GLuint nk_sdl_device_upload_atlas(const void *image, int width, int height)
{
    GLuint font_tex;
    glGenTextures(1, &font_tex);
    glBindTexture(GL_TEXTURE_2D, font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    return font_tex;
}*/

void nk_sdl_device_destroy(nk_gl_device& dev)
{
    glDetachShader(dev.prog, dev.vert_shdr);
    glDetachShader(dev.prog, dev.frag_shdr);
    glDeleteShader(dev.vert_shdr);
    glDeleteShader(dev.frag_shdr);
    glDeleteProgram(dev.prog);
    delete dev.vertexArrayObject;
    nk_buffer_free(&dev.cmds);
}

void nk_sdl_render_dump(Render::SpriteCacheBase* cache, NuklearFrameDump& dump, SDL_Window* win, const Render::AtlasTexture& atlasTexture)
{
    int width, height;
    int display_width, display_height;
    struct nk_vec2 scale;
    GLfloat ortho[4][4] = {
        {2.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, -2.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, -1.0f, 0.0f},
        {-1.0f, 1.0f, 0.0f, 1.0f},
    };
    SDL_GetWindowSize(win, &width, &height);

    // Note: if SDL_GL_GetDrawableSize() is used SDL_WINDOW_ALLOW_HIGHDPI option must
    // also be enabled. However this seems to cause an issue where black lines appear
    // between each tile on high-DPI displays so is currently disabled.
    // SDL_GL_GetDrawableSize(win, &display_width, &display_height);
    display_width = width;
    display_height = height;

    ortho[0][0] /= (GLfloat)width;
    ortho[1][1] /= (GLfloat)height;

    scale.x = (float)display_width / (float)width;
    scale.y = (float)display_height / (float)height;

    // setup global state
    glViewport(0, 0, display_width, display_height);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

    nk_gl_device& dev = dump.getDevice();

    // setup program
    glUseProgram(dev.prog);

    atlasTexture.bind();
    glUniform1i(dev.uniform_tex, 0);
    auto& atlasLookupMap = atlasTexture.getLookupMap();

    glUniformMatrix4fv(dev.uniform_proj, 1, GL_FALSE, &ortho[0][0]);
    {
        // convert from command queue into draw list and draw to screen
        const nk_draw_index* offset = NULL;

        dev.vertexArrayObject->getVertexBuffer(0)->setData(dump.vbuf.memory.ptr, dump.vbuf.size);
        dev.vertexArrayObject->getIndexBuffer()->setData(dump.ebuf.memory.ptr, dump.ebuf.size);

        Render::ScopedBindGL vaoBind(static_cast<Render::VertexArrayObjectOpenGL*>(dev.vertexArrayObject));

        // iterate over and execute each draw command
        for (size_t i = 0; i < dump.drawCommands.size(); i++)
        {
            const struct nk_draw_command& cmd = dump.drawCommands[i];
            if (!cmd.elem_count)
                continue;

            uint32_t cacheIndex = ((uint32_t*)cmd.texture.ptr)[0];
            uint32_t frameNum = ((uint32_t*)cmd.texture.ptr)[1];
            auto effect = static_cast<FAGui::EffectType>(cmd.userdata.id);

            Render::SpriteGroup* sprite = cache->get(cacheIndex);
            auto s = sprite->operator[](frameNum);

            auto& atlasEntry = atlasLookupMap.at((GLuint)(intptr_t)s);

            int item_hl_color[] = {0xB9, 0xAA, 0x77};
            glUniform4f(dev.uniform_hoverColor,
                        item_hl_color[0] / 255.f,
                        item_hl_color[1] / 255.f,
                        item_hl_color[2] / 255.f,
                        effect == FAGui::EffectType::highlighted ? 1.0f : 0.0f);
            glUniform1i(dev.uniform_checkerboarded, effect == FAGui::EffectType::checkerboarded ? 1 : 0);
            glUniform2f(dev.uniform_imageSize, atlasEntry.mWidth, atlasEntry.mHeight);
            glUniform3f(dev.uniform_atlasOffset, atlasEntry.mX, atlasEntry.mY, atlasEntry.mLayer);
            glUniform2f(dev.uniform_atlasSize, atlasTexture.getTextureWidth(), atlasTexture.getTextureHeight());

            glScissor((GLint)(cmd.clip_rect.x * scale.x),
                      (GLint)((height - (GLint)(cmd.clip_rect.y + cmd.clip_rect.h)) * scale.y),
                      (GLint)(cmd.clip_rect.w * scale.x),
                      (GLint)(cmd.clip_rect.h * scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd.elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd.elem_count;
        }
    }

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
}

/*
static void
nk_sdl_clipbard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    const char *text = SDL_GetClipboardText();
    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
    (void)usr;
}

static void
nk_sdl_clipbard_copy(nk_handle usr, const char *text, int len)
{
    char *str = 0;
    (void)usr;
    if (!len) return;
    str = (char*)malloc((size_t)len+1);
    if (!str) return;
    memcpy(str, text, (size_t)len);
    str[len] = '\0';
    SDL_SetClipboardText(str);
    free(str);
}
*/

/*void nk_sdl_init(nk_sdl& nkSdl, SDL_Window *win)
{
    sdl = nkSdl;

    nkSdl.win = win;
    nk_init_default(&nkSdl.ctx, 0);
    sdl.ctx.clip.copy = nullptr;// nk_sdl_clipbard_copy;
    sdl.ctx.clip.paste = nullptr;// nk_sdl_clipbard_paste;
    sdl.ctx.clip.userdata = nk_handle_ptr(0);
    nk_sdl_device_create();
}*/

/*void nk_sdl_font_stash_begin(nk_font_atlas& atlas)
{
    nk_font_atlas_init_default(&atlas);
    nk_font_atlas_begin(&atlas);
}

GLuint nk_sdl_font_stash_end(nk_context* ctx, nk_font_atlas& atlas, nk_draw_null_texture& nullTex)
{
    const void *image; int w, h;
    image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    GLuint font_tex = nk_sdl_device_upload_atlas(image, w, h);
    nk_font_atlas_end(&atlas, nk_handle_id((int)font_tex), &nullTex);
    if (atlas.default_font)
        nk_style_set_font(ctx, &atlas.default_font->handle);

    return font_tex;
}*/

#if 0
NK_API int
nk_sdl_handle_event(SDL_Event *evt)
{
    struct nk_context *ctx = &sdl.ctx;
    if (evt->type == SDL_KEYUP || evt->type == SDL_KEYDOWN) {
        /* key events */
        int down = evt->type == SDL_KEYDOWN;
        const Uint8* state = SDL_GetKeyboardState(0);
        SDL_Keycode sym = evt->key.keysym.sym;
        if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT)
            nk_input_key(ctx, NK_KEY_SHIFT, down);
        else if (sym == SDLK_DELETE)
            nk_input_key(ctx, NK_KEY_DEL, down);
        else if (sym == SDLK_RETURN)
            nk_input_key(ctx, NK_KEY_ENTER, down);
        else if (sym == SDLK_TAB)
            nk_input_key(ctx, NK_KEY_TAB, down);
        else if (sym == SDLK_BACKSPACE)
            nk_input_key(ctx, NK_KEY_BACKSPACE, down);
        else if (sym == SDLK_HOME) {
            nk_input_key(ctx, NK_KEY_TEXT_START, down);
            nk_input_key(ctx, NK_KEY_SCROLL_START, down);
        } else if (sym == SDLK_END) {
            nk_input_key(ctx, NK_KEY_TEXT_END, down);
            nk_input_key(ctx, NK_KEY_SCROLL_END, down);
        } else if (sym == SDLK_PAGEDOWN) {
            nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down);
        } else if (sym == SDLK_PAGEUP) {
            nk_input_key(ctx, NK_KEY_SCROLL_UP, down);
        } else if (sym == SDLK_z)
            nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_r)
            nk_input_key(ctx, NK_KEY_TEXT_REDO, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_c)
            nk_input_key(ctx, NK_KEY_COPY, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_v)
            nk_input_key(ctx, NK_KEY_PASTE, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_x)
            nk_input_key(ctx, NK_KEY_CUT, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_b)
            nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_e)
            nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_UP)
            nk_input_key(ctx, NK_KEY_UP, down);
        else if (sym == SDLK_DOWN)
            nk_input_key(ctx, NK_KEY_DOWN, down);
        else if (sym == SDLK_LEFT) {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
            else nk_input_key(ctx, NK_KEY_LEFT, down);
        } else if (sym == SDLK_RIGHT) {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
            else nk_input_key(ctx, NK_KEY_RIGHT, down);
        } else return 0;
        return 1;
    } else if (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONUP) {
        /* mouse button */
        int down = evt->type == SDL_MOUSEBUTTONDOWN;
        const int x = evt->button.x, y = evt->button.y;
        if (evt->button.button == SDL_BUTTON_LEFT) {
            if (evt->button.clicks > 1)
                nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y, down);
            nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
        } else if (evt->button.button == SDL_BUTTON_MIDDLE)
            nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
        else if (evt->button.button == SDL_BUTTON_RIGHT)
            nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
        return 1;
    } else if (evt->type == SDL_MOUSEMOTION) {
        /* mouse motion */
        if (ctx->input.mouse.grabbed) {
            int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
            nk_input_motion(ctx, x + evt->motion.xrel, y + evt->motion.yrel);
        } else nk_input_motion(ctx, evt->motion.x, evt->motion.y);
        return 1;
    } else if (evt->type == SDL_TEXTINPUT) {
        /* text input */
        nk_glyph glyph;
        memcpy(glyph, evt->text.text, NK_UTF_SIZE);
        nk_input_glyph(ctx, glyph);
        return 1;
    } else if (evt->type == SDL_MOUSEWHEEL) {
        /* mouse wheel */
        nk_input_scroll(ctx,nk_vec2((float)evt->wheel.x,(float)evt->wheel.y));
        return 1;
    }
    return 0;
}
#endif

/*void nk_sdl_shutdown(void)
{
    nk_font_atlas_clear(&sdl.atlas);
    nk_free(&sdl.ctx);
    nk_sdl_device_destroy();
    memset(&sdl, 0, sizeof(sdl));
}*/

NuklearFrameDump::NuklearFrameDump(nk_gl_device& dev) { init(dev); }

void NuklearFrameDump::init(nk_gl_device& dev)
{
    this->dev = &dev;
    nk_buffer_init_default(&vbuf);
    nk_buffer_init_default(&ebuf);
    nk_buffer_init_default(&cmds);

    memset(&config, 0, sizeof(config));
    config.vertex_layout = vertex_layout;
    config.vertex_size = sizeof(Render::NuklearVertex);
    config.vertex_alignment = NK_ALIGNOF(Render::NuklearVertex);
    config.null = dev.null;
    config.circle_segment_count = 22;
    config.curve_segment_count = 22;
    config.arc_segment_count = 22;
    config.global_alpha = 1.0f;
    config.shape_AA = nk_anti_aliasing::NK_ANTI_ALIASING_ON;
    config.line_AA = nk_anti_aliasing::NK_ANTI_ALIASING_ON;
}

NuklearFrameDump::~NuklearFrameDump()
{
    debug_assert(dev);
    nk_buffer_free(&vbuf);
    nk_buffer_free(&ebuf);
    nk_buffer_free(&cmds);
}

void NuklearFrameDump::fill(nk_context* ctx)
{
    debug_assert(dev);

    nk_buffer_clear(&vbuf);
    nk_buffer_clear(&ebuf);
    nk_buffer_clear(&cmds);
    nk_convert(ctx, &cmds, &vbuf, &ebuf, &config);

    drawCommands.clear();

    const nk_draw_command* cmd;
    nk_draw_foreach(cmd, ctx, &cmds) { drawCommands.push_back(*cmd); }
}

nk_gl_device& NuklearFrameDump::getDevice()
{
    debug_assert(dev);
    return *dev;
}
