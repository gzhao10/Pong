/**
* Author: Gavin Zhao
* Assignment: Pong Clone
* Date due: 2023-10-21, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_GLEXT_PROTOTYPES 1
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"



const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED     = 0.0f,
            BG_BLUE    = 0.0f,
            BG_GREEN   = 0.0f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X      = 0,
          VIEWPORT_Y      = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char  V_SHADER_PATH[]          = "shaders/vertex_textured.glsl",
            F_SHADER_PATH[]          = "shaders/fragment_textured.glsl",
            PLAYER_SPRITE_FILEPATH[] = "/Users/gzhao10/Downloads/Bone.png",
            BALL_SPRITE_FILEPATH[]   = "/Users/gzhao10/Downloads/Pumpkin.png",
            GRAVE_SPRITE_FILEPATH[]  = "/Users/gzhao10/Downloads/Grave.png",
            PLAYER_WIN_FILEPATH[]    = "/Users/gzhao10/Downloads/P1Message.png",
            OTHER_WIN_FILEPATH[]     = "/Users/gzhao10/Downloads/P2Message.png";

const float MILLISECONDS_IN_SECOND     = 1000.0f,
            COLLISION_FACTOR           = 0.6f;

const int   NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL    = 0;
const GLint TEXTURE_BORDER     = 0;

SDL_Window* g_display_window;
bool  g_game_is_running = true;
float g_previous_ticks  = 0.0f;

ShaderProgram g_shader_program;
glm::mat4     g_view_matrix,
              g_model_matrix,
              g_projection_matrix,
              g_other_model_matrix,
              g_ball_model_matrix,
              g_grave_model_matrix,
              g_player_win_model_matrix,
              g_other_win_model_matrix;

GLuint g_player_texture_id,
       g_other_texture_id,
       g_ball_texture_id,
       g_grave_texture_id,
       g_player_win_texture_id,
       g_other_win_texture_id;

glm::vec3 g_player_position = glm::vec3(4.0f, 0.0f, 0.0f);
glm::vec3 g_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_other_position  = glm::vec3(-4.0f, 0.0f, 0.0f);
glm::vec3 g_other_movement  = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 PLAYER_INIT_SCA = glm::vec3(0.2f, 1.1f, 0.0f);
glm::vec3 BALL_INIT_SCA = glm::vec3(0.5f, 0.5f, 0.0f);

float g_player_speed = 3.0f,
      g_ball_speed   = 4.0f;

bool g_multiplayer_mode = true,
     g_moving_down      = true,
     g_game_over        = false,
     g_player_win       = true;

bool g_player_turn  = true,
     g_other_turn   = false;
     

GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    
    return textureID;
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Hello, Pong!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_model_matrix              = glm::mat4(1.0f);
    g_other_model_matrix        = glm::mat4(1.0f);
    g_ball_model_matrix         = glm::mat4(1.0f);
    g_grave_model_matrix        = glm::mat4(1.0f);
    g_player_win_model_matrix   = glm::mat4(1.0f);
    g_other_win_model_matrix    = glm::mat4(1.0f);
    
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_player_texture_id     = load_texture(PLAYER_SPRITE_FILEPATH);
    g_other_texture_id      = load_texture(PLAYER_SPRITE_FILEPATH);
    g_ball_texture_id       = load_texture(BALL_SPRITE_FILEPATH);
    g_grave_texture_id      = load_texture(GRAVE_SPRITE_FILEPATH);
    g_player_win_texture_id = load_texture(PLAYER_WIN_FILEPATH);
    g_other_win_texture_id  = load_texture(OTHER_WIN_FILEPATH);

    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void process_input()
{
    g_player_movement = glm::vec3(0.0f);
    g_other_movement = glm::vec3(0.0f);
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_SPACE:
                        g_player_turn = true,
                        g_other_turn = false;
                        g_game_over = false;
                        
                        g_player_position = glm::vec3(4.0f, 0.0f, 0.0f);
                        g_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);

                        g_other_position  = glm::vec3(-4.0f, 0.0f, 0.0f);
                        g_other_movement  = glm::vec3(0.0f, 0.0f, 0.0f);

                        g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
                        g_ball_movement = glm::vec3(1.0f, 0.0f, 0.0f);
                        
                        
                        break;
                        
                    case SDLK_t:
                        if (!g_game_over){
                            g_multiplayer_mode = !g_multiplayer_mode;
                        }
                        break;
                        
                    case SDLK_q:
                        g_game_is_running = false;
                        break;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    
    if (!g_game_over){
        
        if (key_state[SDL_SCANCODE_UP] && g_player_position.y < 3.5f)
        {
            g_player_movement.y = 1.0f;
        }
        else if (key_state[SDL_SCANCODE_DOWN] && g_player_position.y > -3.35f)
        {
            g_player_movement.y = -1.0f;
        }
        
        if (g_multiplayer_mode){
            
            if (key_state[SDL_SCANCODE_W] && g_other_position.y < 3.5f)
            {
                g_other_movement.y = 1.0f;
            }
            else if (key_state[SDL_SCANCODE_S] && g_other_position.y > -3.35f)
            {
                g_other_movement.y = -1.0f;
            }
        }
    }
}



bool check_collision(glm::vec3 &position_a, glm::vec3 &position_b){
    float x_distance = fabs(position_a.x - position_b.x) - ((PLAYER_INIT_SCA.x * COLLISION_FACTOR +                                                                 BALL_INIT_SCA.x * COLLISION_FACTOR) / 2.0f);
    float y_distance = fabs(position_a.y - position_b.y) - ((PLAYER_INIT_SCA.y * COLLISION_FACTOR +                                                                 BALL_INIT_SCA.y * COLLISION_FACTOR) / 2.0f);
    return (x_distance < 0.0f && y_distance < 0.0f);
}


void update()
{
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    g_model_matrix          = glm::mat4(1.0f);
    g_other_model_matrix    = glm::mat4(1.0f);
    g_ball_model_matrix     = glm::mat4(1.0f);
    
    //if singleplayer, automatically move left paddle up and down
    if (!g_multiplayer_mode && !g_game_over){
        if (g_moving_down){
            if (g_other_position.y < -3.35)
            {
                g_moving_down = !g_moving_down;
            }
            else{
                g_other_movement.y = -1.0f;
            }
        }
        else{
            if (g_other_position.y > 3.5){
                g_moving_down = !g_moving_down;
            }
            else{
                g_other_movement.y = 1.0f;
            }
        }
    }
    
    g_player_position += g_player_movement * g_player_speed * delta_time;
    g_other_position += g_other_movement * g_player_speed * delta_time;
    
    //handle ball bouncing off right paddle
    if (check_collision(g_player_position, g_ball_position) && g_player_turn){
        g_player_turn = !g_player_turn;
        g_other_turn = !g_other_turn;
        g_ball_movement.x = -1.0f;
        //add variation to bounces
        if (g_player_movement.y == -1.0f){
            g_ball_movement.y = -1.0f;
        }
        else if (g_player_movement.y == 0){
            g_ball_movement.y = 0.5f;
        }
        else{
            g_ball_movement.y = 1.0f;
        }
    }
    
    //handle ball bouncing off left paddle
    if (check_collision(g_other_position, g_ball_position) && g_other_turn){
        g_player_turn = !g_player_turn;
        g_other_turn = !g_other_turn;
        g_ball_movement.x = 1.0f;
        //add variation to bounces
        if (g_other_movement.y == 1.0f){
            g_ball_movement.y == 1.0f;
        }
        else if (g_other_movement.y == 0){
            g_ball_movement.y = -0.5f;
        }
        else{
            g_ball_movement.y = -1.0f;
        }
    }
    
    //ball bounces off top wall
    if (g_ball_position.y > 3.5f){
        g_ball_movement.y = -1.0f;
    }
    //ball bounces off bottom wall
    if (g_ball_position.y < -3.67f){
        g_ball_movement.y = 1.0f;
    }
    
    
    g_ball_position += g_ball_movement * g_ball_speed * delta_time;
    
    
    //perform transformations
    g_model_matrix          = glm::translate(g_model_matrix, g_player_position);
    g_other_model_matrix    = glm::translate(g_other_model_matrix, g_other_position);
    g_ball_model_matrix     = glm::translate(g_ball_model_matrix, g_ball_position);
    
    
    //see if someone lost
    if (g_ball_position.x > 4.95f || g_ball_position.x < -4.8f){
        g_game_over = true;
        g_ball_movement = glm::vec3(0.0f, 0.0f, 0.0f);
        g_grave_model_matrix = g_ball_position.x > 4.95f ? g_model_matrix : g_other_model_matrix;
        g_player_win = g_ball_position.x < -4.8f;
    }
    
}


void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    draw_object(g_model_matrix, g_player_texture_id);
    draw_object(g_other_model_matrix, g_other_texture_id);
    
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    float vertices_2[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };
    
    float texture_coordinates_2[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices_2);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates_2);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    draw_object(g_ball_model_matrix, g_ball_texture_id);
    
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    float vertices_3[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    float texture_coordinates_3[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices_3);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates_3);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    if (g_game_over){
        draw_object(g_grave_model_matrix, g_grave_texture_id);
    }
    
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    
    float vertices_4[] = {
        -2.5f, -2.5f, 2.5f, -2.5f, 2.5f, 2.5f,
        -2.5f, -2.5f, 2.5f, 2.5f, -2.5f, 2.5f
    };
    
    float texture_coordinates_4[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices_4);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates_4);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    if(g_game_over && g_player_win){
        draw_object(g_player_win_model_matrix, g_player_win_texture_id);
    }
    
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    float vertices_5[] = {
        -2.5f, -2.5f, 2.5f, -2.5f, 2.5f, 2.5f,
        -2.5f, -2.5f, 2.5f, 2.5f, -2.5f, 2.5f
    };

    float texture_coordinates_5[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices_5);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates_5);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    if(g_game_over && !g_player_win){
        draw_object(g_other_win_model_matrix, g_other_win_texture_id);
    }
    
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
