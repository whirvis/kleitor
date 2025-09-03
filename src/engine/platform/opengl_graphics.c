/*
 * the MIT License (MIT)
 *
 * Copyright (c) 2023 Trent Summerlin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * the above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#if defined(_WIN32) || defined(__linux__)

#include "graphics.h"

#include <stdbool.h>
#include <stdlib.h>

#include "../../libraries/cglm_no_io.h"

#include "../../runtime/common.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef struct PLAT_RBTK_MONITOR {
    GLFWmonitor *glfw_monitor;
} PLAT_RBTK_MONITOR;

typedef struct PLAT_RBTK_WINDOW {
    GLFWwindow *glfw_window;
    GLuint sprite_vao;
} PLAT_RBTK_WINDOW;

typedef struct PLAT_RBTK_GRAPHICS {
    GLuint depth_buffer;
    GLuint scene_texture;
    GLuint model_vbo;
    GLuint uv_vbo;
    struct {
        GLuint id;
        GLFWwindow *context;
    } frame_buffers[RBTK_MAX_WINDOW_COUNT];
} PLAT_RBTK_GRAPHICS;

typedef struct PLAT_RBTK_SPRITE {
    GLuint model_vbo;
    GLuint uv_vbo;
    GLuint texture;
} PLAT_RBTK_SPRITE;

static bool
compile_shader(GLint type, const char *src, GLuint *result)
{
    assert(src);
    assert(result);

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);

    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "GLSL compilation glew_error:\n%s", log);
        return false;
    }

    *result = shader;
    return true;
}

static bool
create_program(size_t shader_count, GLuint shaders[], bool delete_shaders,
    GLuint *result)
{
    assert(result);

    GLuint program = glCreateProgram();
    for (size_t i = 0; i < shader_count; i++) {
        glAttachShader(program, shaders[i]);
    }

    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "OpenGL program link glew_error:\n%s", log);
        return false;
    }

    if (delete_shaders) {
        for (size_t i = 0; i < shader_count; i++) {
            glDeleteShader(shaders[i]);
        }
    }

    *result = program;
    return true;
}

static const char *sprite_vert_src =
    "#version 330 core                                            \n"
    "                                                             \n"
    "layout(location = 0) in vec2 buf_coords;                     \n"
    "layout(location = 1) in vec2 tex_coords;                     \n"
    "                                                             \n"
    "out vec2 frag_tex_coords;                                    \n"
    "                                                             \n"
    "uniform mat4 proj;                                           \n"
    "uniform mat4 view;                                           \n"
    "uniform mat4 model;                                          \n"
    "                                                             \n"
    "void main()                                                  \n"
    "{                                                            \n"
    "    frag_tex_coords = tex_coords;                            \n"
    "                                                             \n"
    "    gl_Position = proj * view * model                        \n"
    "                * vec4(buf_coords, 0.0, 1.0);                \n"
    "}                                                            \n";

static const char *sprite_frag_src =
    "#version 330 core                                            \n"
    "                                                             \n"
    "uniform sampler2D sampler;                                   \n"
    "uniform vec4 obj_color;                                      \n"
    "                                                             \n"
    "in vec2 frag_tex_coords;                                     \n"
    "                                                             \n"
    "layout(location = 0) out vec4 color;                         \n"
    "                                                             \n"
    "void main()                                                  \n"
    "{                                                            \n"
    "    color = texture(sampler, frag_tex_coords);               \n"
    "    color *= obj_color;                                      \n"
    "}                                                            \n";

struct {
    GLuint id;
    struct {
        GLuint proj;
        GLuint view;
        GLuint model;
        GLuint sampler;
        GLuint color;
    } uniforms;
} gl_sprite_prog;

static RBTK_WINDOW *primary_window;
static bool initialized;

static bool
setup_glfw()
{
    if (glfwInit()) {
        return true;
    }
    else {
        const char *msg = NULL;
        int glfw_error = glfwGetError(&msg);
        if (msg) {
            rbtk_signal_error(RBTK_ERROR_STARTUP,
                "GLFW failed to initialize: %s (%d)", msg, glfw_error);
        }
        else {
            rbtk_signal_error(RBTK_ERROR_STARTUP,
                "GLFW failed to initialize: %d", glfw_error);
        }
        return false;
    }
}

static bool
setup_monitors()
{
    if (!setup_glfw()) {
        return false;
    }

    int monitor_count;
    GLFWmonitor **glfw_monitors = glfwGetMonitors(&monitor_count);
    if (monitor_count <= 0 || !glfw_monitors) {
        const char *msg = NULL;
        int glfw_error = glfwGetError(&msg);
        if (glfw_error == GLFW_NO_ERROR) {
            rbtk_signal_error(RBTK_ERROR_STARTUP,
                "no monitors attached to the computer");
        }
        else if (msg) {
            rbtk_signal_error(RBTK_ERROR_STARTUP,
                "GLFW could not locate mointors: %s (%d)",
                msg, glfw_error);
        }
        else {
            rbtk_signal_error(RBTK_ERROR_STARTUP,
                "GLFW could not locate monitors: %d",
                glfw_error);
        }
        return false;
    }

    for (int i = 0; i < monitor_count; i++) {
        GLFWmonitor *glfw_monitor = glfw_monitors[i];
        assert(glfw_monitor); /* just to be safe */

        PLAT_RBTK_MONITOR *plat = NULL;
        RBTK_MALLOC_OR_RETURN(&plat, false,
            "could not allocate memory for monitor %d", i);
        plat->glfw_monitor = glfw_monitor;

        RBTK_MONITOR *monitor = NULL;
        RBTK_MALLOC_OR_RETURN(&monitor, false,
            "could not allocate memory for monitor %d", i);
        monitor->plat = plat;

        if (!priv_rbtk_add_monitor(monitor)) {
            rbtk_signal_error(RBTK_ERROR_STARTUP,
                "failed to register monitor %d", i);
            return false;
        }
    }

    return true;
}

static bool
setup_windows()
{
    if (!setup_glfw()) {
        return false;
    }

    /* don't show the window until the user needs to see */
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    /* use OpenGL 3.3, forward compatibility for MacOS */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    primary_window = priv_rbtk_create_window(800, 600);
    if (!primary_window) {
        return false;
    }

    return true;
}

static bool
load_sprite_program()
{
    GLuint shaders[2] = { 0 };
    if (!compile_shader(GL_VERTEX_SHADER, sprite_vert_src, &shaders[0])) {
        return false;
    }
    if (!compile_shader(GL_FRAGMENT_SHADER, sprite_frag_src, &shaders[1])) {
        return false;
    }

    size_t shader_count = sizeof(shaders) / sizeof(GLuint);
    if (!create_program(shader_count, shaders, true, &gl_sprite_prog.id)) {
        return false;
    }

    gl_sprite_prog.uniforms.proj = glGetUniformLocation(gl_sprite_prog.id, "proj");
    gl_sprite_prog.uniforms.view = glGetUniformLocation(gl_sprite_prog.id, "view");
    gl_sprite_prog.uniforms.model = glGetUniformLocation(gl_sprite_prog.id, "model");
    gl_sprite_prog.uniforms.sampler = glGetUniformLocation(gl_sprite_prog.id, "sampler");
    gl_sprite_prog.uniforms.color = glGetUniformLocation(gl_sprite_prog.id, "obj_color");

    return true;
}

static bool
setup_opengl() {
    /* setup OpenGL on the primary window's context */
    GLFWwindow *primary_context = primary_window->plat->glfw_window;
    glfwMakeContextCurrent(primary_context);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    if (!load_sprite_program()) {
        return false;
    }

    GLenum gl_error = glGetError();
    if (gl_error != GL_NO_ERROR) {
        glDeleteProgram(gl_sprite_prog.id);
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "OpenGL error loading sprite program: %d", gl_error);
        return false;
    }

    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_graphics_init()
{
    if (initialized) {
        return true;
    }

    if (!setup_monitors()) {
        return false;
    }
    if (!setup_windows()) {
        return false;
    }
    if (!setup_opengl()) {
        return false;
    }

    initialized = true;
    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_graphics_terminate(void)
{
    if (!initialized) {
        return true;
    }

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDeleteProgram(gl_sprite_prog.id);
    glfwTerminate();

    RBTK_ZERO_MEMORY(&gl_sprite_prog);
    primary_window = NULL;

    initialized = false;
    return true;
}

RBTK_PLATFORM void
plat_rbtk_destroy_monitor(RBTK_MONITOR *monitor)
{
    assert(monitor);
    free(monitor->plat);
}

static void
glfw_window_size_callback(GLFWwindow *glfw_window, int width, int height)
{
    assert(glfw_window);
    RBTK_WINDOW *window = glfwGetWindowUserPointer(glfw_window);
    window->width = width;
    window->height = height;
}

/* used by glfw_input.c */
GLFWwindow *plat_rbtk_focused_glfw_window;

/* used by win32_engine.c */
void
plat_rbtk_update_glfw(void)
{
    glfwPollEvents();

    size_t num_windows;
    RBTK_WINDOWS windows = rbtk_get_windows(&num_windows);
    for (size_t i = 0; i < num_windows; i++) {
        RBTK_WINDOW *window = windows[i];

        GLFWwindow *glfw_window = window->plat->glfw_window;
        if (glfwWindowShouldClose(glfw_window)) {
            window->should_close = true;
            glfwSetWindowShouldClose(glfw_window, false);
        }
    }
}

static void
glfw_window_focus_callback(GLFWwindow *glfw_window, int focused)
{
    assert(glfw_window);
    RBTK_WINDOW *window = glfwGetWindowUserPointer(glfw_window);
    window->focused = focused;

    if (focused) {
        plat_rbtk_focused_glfw_window = glfw_window;
    }
    else if (plat_rbtk_focused_glfw_window == glfw_window) {
        plat_rbtk_focused_glfw_window = NULL;
    }
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_create_window(RBTK_WINDOW *window,
    unsigned int width, unsigned int height)
{
    assert(window);

    PLAT_RBTK_WINDOW *plat = NULL;
    RBTK_MALLOC_OR_RETURN(&plat, false,
        "could not allocate platform specifc memory for GLFW window");

    GLFWwindow *sharing = NULL;
    if (primary_window) {
        sharing = primary_window->plat->glfw_window;
    }

    GLFWwindow *glfw_window = glfwCreateWindow(width, height,
        "" /* default to no title */, NULL, sharing);

    if (!glfw_window) {
        const char *glfw_error_msg;
        int glfw_error = glfwGetError(&glfw_error_msg);
        rbtk_signal_error(RBTK_ERROR_PLATFORM,
            "GLFW failed to create window (error code %d): %s",
            glfw_error, glfw_error_msg);
        return false;
    }

    glfwSetWindowSizeCallback(glfw_window, glfw_window_size_callback);
    glfwSetWindowFocusCallback(glfw_window, glfw_window_focus_callback);
    glfwSetWindowUserPointer(glfw_window, window);

    GLFWwindow *previous_context = glfwGetCurrentContext();

    glfwMakeContextCurrent(glfw_window);
    glfwSwapInterval(0);     /* explicitly disable V-sync */
    glewExperimental = true; /* required for core profile */

    GLenum glew_error = glewInit();
    if (glew_error != GL_NO_ERROR) {
        glfwTerminate();
        rbtk_destroy_window(primary_window);
        rbtk_signal_error(RBTK_ERROR_STARTUP,
            "GLEW failed to initialize (error code %d): %s",
            glew_error, glewGetErrorString(glew_error));
        return false;
    }

    plat->glfw_window = glfw_window;
    glGenVertexArrays(1, &plat->sprite_vao);

    glfwMakeContextCurrent(previous_context);

    rbtk_window_caps caps = {
        .can_minimize = true,
        .can_close = true,
        .can_move = true,
        .resizeable = true,
        .has_title = true,
        .has_icon = true,
    };

    window->caps = caps;
    window->plat = plat;
    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_destroy_window(RBTK_WINDOW *window) {
    assert(window);
    assert(window != primary_window);

    PLAT_RBTK_WINDOW *plat = window->plat;
    window->plat = NULL;
    glfwDestroyWindow(plat->glfw_window);
    free(plat);

    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_show_window(RBTK_WINDOW *window)
{
    assert(window);
    glfwShowWindow(window->plat->glfw_window);
    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_hide_window(RBTK_WINDOW *window)
{
    assert(window);
    glfwHideWindow(window->plat->glfw_window);
    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_set_display_mode(RBTK_UNUSED RBTK_WINDOW *window,
        RBTK_UNUSED rbtk_display_mode mode)
{
    assert(window);
    rbtk_signal_error(RBTK_ERROR_UNSUPPORTED,
        "setting display mode not yet supported");
    return false;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_set_window_icon(RBTK_WINDOW *window,
    unsigned int width, unsigned int height, unsigned char *pixels)
{
    assert(window);
    PLAT_RBTK_WINDOW *plat = window->plat;

    if (!pixels) {
        glfwSetWindowIcon(plat->glfw_window, 0, NULL);
        return true;
    }

    GLFWimage icon = { 0 };
    icon.width = width;
    icon.height = height;
    icon.pixels = pixels;

    glfwSetWindowIcon(plat->glfw_window, 1, &icon);
    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_set_window_title(RBTK_WINDOW *window)
{
    assert(window);
    PLAT_RBTK_WINDOW *plat = window->plat;
    if (window->title) {
        glfwSetWindowTitle(plat->glfw_window, window->title);
    }
    else {
        glfwSetWindowTitle(plat->glfw_window, "");
    }
    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_get_window_size(const RBTK_WINDOW *window,
    unsigned int *width, unsigned int *height)
{
    assert(window);
    assert(width && height);

    PLAT_RBTK_WINDOW *plat = window->plat;

    /*
     * For some reason, glfwGetWindowSize() expects pointers to two signed
     * integers (instead of unsigned integers). This is even more baffling
     * that when on error the window size is set to zero, not negative one.
     * Either way, using intermediate variables for this silences warnings
     * that come from passing unsigned integer pointers to this method.
     */
    int signed_width = 0, signed_height = 0;
    glfwGetWindowSize(plat->glfw_window, &signed_width, &signed_height);
    *width = signed_width;
    *height = signed_height;

    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_set_window_size(RBTK_WINDOW *window,
    unsigned int width, unsigned int height)
{
    assert(window);
    PLAT_RBTK_WINDOW *plat = window->plat;
    glfwSetWindowSize(plat->glfw_window, width, height);
    return true;
}

static void
set_sprite_buffers(GLuint model_vbo, GLuint uv_vbo,
    float x, float y,
    float width, float height,
    float texture_width, float texture_height)
{
    /*
    * This (as well as the UV buffer) is intentionally declared on the
    * stack as OpenGL will copy the data to its own internal buffers.
    */
    float model_buffer[] = {
        x,         y,          /* top left     */
        x,         y + height, /* bottom left  */
        x + width, y + height, /* bottom right */

        x + width, y + height, /* bottom right */
        x + width, y,          /* top right    */
        x,         y,          /* top left     */
    };

    /*
     * The UV coordinates for this texture are intentionally upside down,
     * since OpenGL displays the textures upside down. I prefer this way
     * over others since it's faster and requires less instructions.
     */
    float uv_buffer[] = {
         x          / texture_width,  y           / texture_height, /* bottom left  */
         x          / texture_width, (y + height) / texture_height, /* top left     */
        (x + width) / texture_width, (y + height) / texture_height, /* top right    */

        (x + width) / texture_width, (y + height) / texture_height, /* top right    */
        (x + width) / texture_width,  y           / texture_height, /* bottom right */
         x          / texture_width,  y           / texture_height, /* bottom left  */
    };

    glBindBuffer(GL_ARRAY_BUFFER, model_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(model_buffer),
        model_buffer, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer),
        uv_buffer, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0); /* prevent accidental changes */
}

RBTK_PLATFORM PLAT_RBTK_SPRITE *
plat_rbtk_load_sprite(unsigned int width, unsigned int height,
    unsigned short channels, unsigned char *pixels)
{
    assert(channels >= 3 && channels <= 4);

    PLAT_RBTK_SPRITE *plat = NULL;
    RBTK_MALLOC_OR_RETURN(&plat, NULL,
        "could not allocate sprite for current plaform");

    glGenBuffers(1, &plat->model_vbo);
    glGenBuffers(1, &plat->uv_vbo);
    set_sprite_buffers(plat->model_vbo, plat->uv_vbo,
        0.0f, 0.0f, (float) width, (float) height,
        (float) width, (float) height);

    /*
     * Finally, we must generate an OpenGL texture and store the pixels given
     * to us into it. OpenGL will also copy these pixels into its own memory.
     * However, we don't assume the expected lifeteime of the pixels given to
     * us. It is the callers responsibility to free that memory.
     */
    glGenTextures(1, &plat->texture);
    glBindTexture(GL_TEXTURE_2D, plat->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        channels > 3 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0); /* prevent accidental changes */

    return plat;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_unload_sprite(RBTK_SPRITE *sprite)
{
    assert(sprite);

    /*
     * Unloading a sprite for OpenGL is pretty straight forward, all we need
     * do is delete all of the VAOs, VBOs, etc. that we previously allocated
     * for it. These are also stored in OpenGL's global state, so it doesn't
     * matter which context is currently in use.
     */
    PLAT_RBTK_SPRITE *plat = sprite->plat;
    glDeleteBuffers(1, &plat->model_vbo);
    glDeleteBuffers(1, &plat->uv_vbo);
    glDeleteTextures(1, &plat->texture);

    return true;
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_create_scene(RBTK_GRAPHICS *scene,
    unsigned int width, unsigned int height)
{
    assert(scene);

    PLAT_RBTK_GRAPHICS *plat = NULL;
    RBTK_MALLOC_OR_RETURN(&plat, false,
        "could not allocate graphics scene for current platform");

    /*
     * The first step is to create a render buffer. This is used for depth
     * testing, which isn't usually necessary in 2D graphics but absolutely
     * needed in 3D graphics.
     */
    glGenRenderbuffers(1, &plat->depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, plat->depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
        width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0); /* prevent accidental changes */

    /*
     * The code below is intentionally disabled via the #if 0 directive.
     * I often have trouble with framebuffers, and as a testing measure
     * I will set the pixels to some color to help debugging. I usually
     * do this to make sure the framebuffer's texture is being drawn.
     */
    unsigned char *pixels = NULL;
#if 0
    /* 4 channels because RGBA */
    size_t pixels_size = width * height * 4;
    pixels = malloc(pixels_size);
    assert(pixels); /* we cannot recover from this */

    for (size_t i = 0; i < pixels_size; i += 4) {
        pixels[i + 0] = 0xFF; /* red   */
        pixels[i + 1] = 0x00; /* green */
        pixels[i + 2] = 0x00; /* blue  */
        pixels[i + 3] = 0xFF; /* alpha */
    }
#endif

    /*
     * Next, we update the texture for the scene's sprite. We can specify
     * NULL to OpenGL which will zero initialize it (set all pixels black).
     * We set the width and height of the texture to the requested size as
     * we want it to fill the entire scene.
     */
    plat->scene_texture = scene->sprite->plat->texture;
    glBindTexture(GL_TEXTURE_2D, plat->scene_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0); /* prevent accidental changes */
    free(pixels); /* freeing NULL is okay, it's just a no-op */

    glGenBuffers(1, &plat->model_vbo);
    glGenBuffers(1, &plat->uv_vbo);
    set_sprite_buffers(plat->model_vbo, plat->uv_vbo,
        0.0f, 0.0f, (float) width, (float) height,
        (float) width, (float) height);

    /*
     * Set all windows to NULL to it's known which slots are available.
     * Also set all the frame buffer ID's to -1, as 0 is the ID for the
     * screen's frame buffer.
     */
    for (size_t i = 0; i < RBTK_MAX_WINDOW_COUNT; i++) {
        plat->frame_buffers[i].context = NULL;
        plat->frame_buffers[i].id = 0;
    }

    scene->plat = plat;
    return true;
}

RBTK_PLATFORM void
plat_rbtk_update_sprite_section(RBTK_SPRITE *sprite)
{
    assert(sprite);
    PLAT_RBTK_SPRITE *plat = sprite->plat;
    set_sprite_buffers(plat->model_vbo, plat->uv_vbo,
        (float) sprite->section.x, (float) sprite->section.y,
        (float) sprite->section.width, (float) sprite->section.height,
        (float) sprite->width, (float) sprite->height);
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_destroy_scene(RBTK_GRAPHICS *scene)
{
    assert(scene);
    PLAT_RBTK_GRAPHICS *plat = scene->plat;

    for (size_t i = 0; i < RBTK_MAX_WINDOW_COUNT; i++) {
        if (plat->frame_buffers[i].context) {
            glDeleteFramebuffers(1, &plat->frame_buffers[i].id);
        }
    }

    glDeleteTextures(1, &plat->scene_texture);
    glDeleteBuffers(1, &plat->uv_vbo);
    glDeleteBuffers(1, &plat->model_vbo);
    glDeleteRenderbuffers(1, &plat->depth_buffer);

    free(plat);
    return true;
}

static RBTK_NO_DISCARD GLint
get_sprite_vao_for_current_context()
{
    GLFWwindow *current_context = glfwGetCurrentContext();
    RBTK_WINDOW *window = glfwGetWindowUserPointer(current_context);
    return window->plat->sprite_vao;
}

static RBTK_NO_DISCARD bool
bind_scene_for_current_context(RBTK_GRAPHICS *scene)
{
    assert(scene);
    PLAT_RBTK_GRAPHICS *plat = scene->plat;

    GLFWwindow *current_context = glfwGetCurrentContext();
    for (size_t i = 0; i < RBTK_MAX_WINDOW_COUNT; i++) {
        if (plat->frame_buffers[i].context == current_context) {
            glBindFramebuffer(GL_FRAMEBUFFER, plat->frame_buffers[i].id);
            return true;
        }
    }

    /*
     * If there's no frame buffer for the current context, find the first
     * available slot and fill that for the window. If none can be found,
     * something has gone wrong and the function needs to signal an error.
     */
    size_t slot = 0;
    bool open_slot = false;
    for (size_t i = 0; i < RBTK_MAX_WINDOW_COUNT; i++) {
        if (!plat->frame_buffers[i].context) {
            slot = i;
            open_slot = true;
            break;
        }
    }
    if (!open_slot) {
        rbtk_signal_error(RBTK_ERROR_OUT_OF_MEMORY,
            "no open slots for frame buffer on the current context");
        return false;
    }

    /*
     * Now that we've found an empty slot, we can create a frame buffer for
     * the current context. We have to create frame buffers for each context
     * as they are not stored in OpenGL's global state, while render buffers
     * and textures are.
     */
    GLuint frame_buffer = 0;
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER, plat->depth_buffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, plat->scene_texture, 0);

    /*
     * This tells OpenGL which color outputs we want to enable when rendering
     * to this frame buffer. GL_COLOR_ATTACHMENT0 means we'll write to layout
     * location 0 in the fragment shader. We can easily enable more by adding
     * other attachments to the array if needed.
     */
    GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
    size_t num_draw_buffers = sizeof(draw_buffers) / sizeof(GLenum);
    glDrawBuffers((GLsizei) num_draw_buffers, draw_buffers);

    /* ensure frame buffer is complete before returning */
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        plat->frame_buffers[slot].context = NULL; /* free up slot      */
        plat->frame_buffers[slot].id = 0;         /* invalidate buffer */

        GLenum gl_error = glGetError();
        if (gl_error != GL_NO_ERROR) {
            rbtk_signal_error(RBTK_ERROR_PLATFORM,
                "OpenGL error creating frame buffer: %d", gl_error);
        }
        else {
            rbtk_signal_error(RBTK_ERROR_PLATFORM,
                "error creating frame buffer for current platform");
        }
        return false;
    }

    plat->frame_buffers[slot].context = current_context;
    plat->frame_buffers[slot].id = frame_buffer;

    return true;
}

RBTK_PLATFORM void
plat_rbtk_clear_scene(RBTK_GRAPHICS *scene)
{
    assert(scene);

    /*
     * Clearing the contents is quite simple. We first bind to the scene's
     * framebuffer and then call glClear() for the color and depth buffers.
     * Clearing the depth buffer is required for depth testing to function
     * correctly. Afterwards, unbind the frame buffer to prevent accidental
     * changes.
     */
    bool bound = bind_scene_for_current_context(scene);
    if(!bound) {
        fprintf(stderr, "Failed to bind scene for current context\n");
        abort(); /* we cannot recover from this */
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void
draw_sprite_gl(RBTK_SPRITE *sprite,
    const mat4 proj, const mat4 view, const mat4 model)
{
    assert(sprite);
    PLAT_RBTK_SPRITE *plat = sprite->plat;

    /*
     * Before doing anything, we should make sure that texture 0 is active
     * (otherwise the sampler won't see anything). We should also switch to
     * the sprite program in-case it wasn't in use already.
     *
     * Since switching programs in OpenGL is quite costly, we do not switch
     * out of the program at the end of this function. Setting uniforms does
     * not require a program to be in use. As such, it is not likely another
     * program will accidentally modify the sprite program.
     */
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(gl_sprite_prog.id);

    glUniform1i(gl_sprite_prog.uniforms.sampler, 0);
    glUniformMatrix4fv(gl_sprite_prog.uniforms.proj, 1, GL_FALSE, &proj[0][0]);
    glUniformMatrix4fv(gl_sprite_prog.uniforms.view, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(gl_sprite_prog.uniforms.model, 1, GL_FALSE, &model[0][0]);
    glUniform4f(gl_sprite_prog.uniforms.color, sprite->color.red,
        sprite->color.green, sprite->color.blue, sprite->color.alpha);

    GLint sprite_vao = get_sprite_vao_for_current_context();
    glBindVertexArray(sprite_vao);

    glBindBuffer(GL_ARRAY_BUFFER, plat->model_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, plat->uv_vbo);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); /* prevent accidental changes */

    /*
     * Now that we know we are using the correct program and all the uniforms
     * have been set, we can bind the sprite's texture and VAO and draw it to
     * the currently bound frame buffer.
     */
    glBindTexture(GL_TEXTURE_2D, plat->texture);
    glBindVertexArray(sprite_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0); /* prevent accidental changes */
    glBindVertexArray(0);            /* prevent accidental changes */
}

RBTK_PLATFORM void
plat_rbtk_draw_sprite(RBTK_GRAPHICS *scene, RBTK_SPRITE *sprite,
    float x, float y, float z)
{
    assert(scene);
    assert(sprite);

    vec3 model_translate = {
        x - sprite->section.x,
        y - sprite->section.y,
        z
    };
    vec3 model_scale = {
        sprite->scale.x,
        sprite->scale.y,
        sprite->scale.z
    };
    if (sprite->flipped.horizontally) {
        model_scale[0] *= -1;
        model_translate[0] += sprite->section.width;
    }
    if (sprite->flipped.vertically) {
        model_scale[1] *= -1;
        model_translate[1] += sprite->section.height;
    }

    RBTK_CAMERA *camera = scene->camera;
    vec3 camera_pos = {
        camera->pos[0] * -1.0f,
        camera->pos[1] * -1.0f,
        camera->pos[2] * -1.0f,
    };
    vec3 camera_target = {
        camera_pos[0],
        camera_pos[1],
        0, /* look to the front */
    };
    vec3 camera_up = {
        0, /* leave X-axis alone */
        1, /* look upwards       */
        0, /* leave Z-axis alone */
    };

    /* always initialize to identity just to be safe */
    mat4 model_matrix = GLM_MAT4_IDENTITY_INIT;
    mat4 view_matrix = GLM_MAT4_IDENTITY_INIT;
    mat4 proj_matrix = GLM_MAT4_IDENTITY_INIT;

    /*
     * Now that we have all the necessary information, we can calculate the
     * model view projection matrices, which will determine the final result
     * of drawing this sprite.
     *
     * Note that here we are rendering to a vertically flipped matrix. This
     * has to due with OpenGL, where it wants to render to the frame buffer
     * upside down for some reason. Flipping it while rendering here allows
     * for proper results when rendering to the screen.
     *
     * The following steps below (before copying the final result) must be
     * done in that specific order. The matrix operations used here are not
     * commutative!
     */

    /* 1. Translate the sprite to the requested position.     */
    glm_translate(model_matrix, model_translate);

    /* 2. Apply the specified rotation for each axis.         */
    vec3 x_axis = { 1, 0, 0 };
    glm_rotate(model_matrix, sprite->rotation.x, x_axis);
    vec3 y_axis = { 0, 1, 0 };
    glm_rotate(model_matrix, sprite->rotation.y, y_axis);
    vec3 z_axis = { 0, 0, 1 };
    glm_rotate(model_matrix, sprite->rotation.z, z_axis);

    /* 3. Scale the model to the requested size.              */
    glm_scale(model_matrix, model_scale);

    /*
     * All done, we can now copy our results to the matrices.
     *
     * Note: We have to cast the projection matrix to a non-const
     * pointer to silence a warning caused by discarding constness.
     * This is because the first parameter of glm_mat4_copy() (the
     * source matrix) is not const. The reason for this is unknown.
     */
    glm_mat4_copy(*((mat4 *) &scene->proj->matrix), proj_matrix);
    glm_lookat(camera_pos, camera_target, camera_up, view_matrix);

    /*
     * Here we switch to the requested scene for rendering. After binding
     * to the scene's frame buffer for the current context, we must set
     * the OpenGL viewport. This makes sure OpenGL renders to the entire
     * frame buffer and not just part of it.
     */
    bool bound = bind_scene_for_current_context(scene);
    if(!bound) {
        fprintf(stderr, "Failed to bind scene for current context\n");
        abort(); /* we cannot recover from this */
    }

    glViewport(0, 0, scene->width, scene->height);
    glClear(GL_DEPTH_BUFFER_BIT); /* depth testing */
    draw_sprite_gl(sprite,
        *(const mat4 *) &proj_matrix,
        *(const mat4 *) &view_matrix,
        *(const mat4 *) &model_matrix);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RBTK_PLATFORM RBTK_NO_DISCARD bool
plat_rbtk_render_window_scene(const RBTK_WINDOW *window)
{
    assert(window);
    if (!window->scene) {
        return false;
    }

    PLAT_RBTK_WINDOW *plat = window->plat;
    GLFWwindow *glfw_window = plat->glfw_window;
    RBTK_SPRITE *sprite = window->scene->sprite;

    /*
     * Before rendering, we must make sure we are on the correct context
     * for the window. Otherwise, we will end up rendering to an entirely
     * different window.
     */
    GLFWwindow *current_context = glfwGetCurrentContext();
    if (current_context != glfw_window) {
        glfwMakeContextCurrent(glfw_window);
        current_context = glfw_window;
    }

    /*
     * Now that we know we're using the correct context, we can bind to
     * framebuffer 0. This framebuffer is what contains the contents of
     * the screen. We must next clear the previously rendered contents
     * and the depth buffer, otherwise nothing will display properly.
     */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window->width, window->height);
    draw_sprite_gl(sprite, window->scene_proj,
        window->scene_view, window->scene_model);
    glfwSwapBuffers(glfw_window); /* update window contents */

    return true;
}

#endif /* defined(_WIN32) || defined(__linux__) */
