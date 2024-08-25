//Start of the obj_render project 

#include <iostream>
#include <cmath>
#include <vector>
#include <array>

typedef unsigned int uint;

#define SFML_STATIC
#include <SFML/Graphics.hpp>

#define WIN_WIDTH  1920.F
#define WIN_HEIGHT 1080.F

#define MOV_SPEED 0.5F
#define ROT_SPEED 0.3F

struct Model
{
    std::vector<std::array<float, 3>> vertices;
    std::vector<std::array<uint , 3>> faces;
};

struct Camera
{
    float position[3];
    float angle[2];
    float fovx;
    float fovy;
};

template <typename T>
void clamp(T& val, T low, T high){
    val = std::min(std::max(low, val), high);
}

float read_float(const char* (& data))
{
    float result = 0;
    float sign = +1;
    float where = 0;

    while (*data != ' ' && *data != '\r' && *data != '\n' && *data != '\0')
    {
        if(*data == '-')
        {
            sign = -1;
        }
        else if (*data == '.')
        {
            where = 1;
        }
        else
        {
            if (where == 0) result *= 10;
            result += (*data - '0') / powf(10.F, where);
            if (where != 0) where += 1;
        }
        
        data++;
    }

    result *= sign;

    return result;
}

uint read_uint(const char* (& data))
{
    float result = 0;

    while (*data != ' ' && *data != '\r' && *data != '\n' && *data != '\0')
    {
        result *= 10;
        result += *data - '0';
        data++;
    }

    return result;
}

Model get_teapot_info()
{
    Model teapot;

    extern const char _binary_obj_teapot_obj_start[], _binary_obj_teapot_obj_size[];
    const char* binDataStart = _binary_obj_teapot_obj_start;
    size_t binDataSize = (size_t)_binary_obj_teapot_obj_size;

    const char* charPtr = binDataStart;

    std::array<float, 3> tempF;
    std::array<uint, 3> tempUI;

    while (true)
    {
        if (*charPtr == '\0') break;
        else if (*charPtr == '#')
        {
            while (not(*charPtr == '\n' || *charPtr == '\r'))
            {
                charPtr++;
            }
        }
        
        else if (*charPtr == 'v')
        {
            charPtr += 2;
            for (int i = 0; i < 3; i++)
            {
                tempF[i] = read_float(charPtr);
                charPtr++;
            }
            teapot.vertices.push_back(tempF);
        }
        else if (*charPtr == 'f')
        {
            charPtr += 2;
            for (int i = 0; i < 3; i++)
            {
                tempUI[i] = read_uint(charPtr);
                charPtr++;
            }
            teapot.faces.push_back(tempUI);
        }
        charPtr++;
    }
    
    std::cout << teapot.vertices.size() << " vertices have been found.\n";
    std::cout << teapot.faces.size() << " triangles have been found.\n";

    return teapot;
}

sf::Vector2f projection(std::array<float, 3>& array, Camera& camera)
{
    return sf::Vector2f(
        tanf(atan2f(array[0] - camera.position[0], array[2] - camera.position[2]) - camera.angle[0]),
        tanf(atan2f(array[1] - camera.position[1], array[2] - camera.position[2]) - camera.angle[1])
    );
}

bool Update(sf::RenderWindow& window, Camera& camera, sf::Clock& clock)
{
    float dt = clock.restart().asSeconds();

    sf::Event event;
    while (window.pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::Closed:
            window.close();
            return false;
        
        default:
            break;
        }
    }

    /* Camera movement */ {
        float dm = dt * MOV_SPEED;
        float s = sin(camera.angle[0]) * dm;
        float c = cos(camera.angle[0]) * dm;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) { camera.position[0] += s; camera.position[2] += c; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { camera.position[0] -= s; camera.position[2] -= c; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) { camera.position[0] -= c; camera.position[2] += s; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { camera.position[0] += c; camera.position[2] -= s; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { camera.position[1] += dm; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) { camera.position[1] -= dm; }
    }

    /* Camera rotation */ {
        float dr = dt * ROT_SPEED;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::K)) { camera.angle[1] += dr; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::I)) { camera.angle[1] -= dr; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::J)) { camera.angle[0] -= dr; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) { camera.angle[0] += dr; }
    }

    return true;
}

void Render(sf::RenderWindow& window, Model& model, Camera& camera)
{
    window.clear();

    sf::Vertex temp[2];

    for (std::array<uint, 3> face : model.faces)
    {
        temp[0] = projection(model.vertices[face[0]], camera);
        temp[1] = projection(model.vertices[face[1]], camera);
        window.draw(temp, 2, sf::Lines);
        temp[1] = projection(model.vertices[face[2]], camera);
        window.draw(temp, 2, sf::Lines);
        temp[0] = projection(model.vertices[face[2]], camera);
        window.draw(temp, 2, sf::Lines);
    }

    window.display();
}

int main(int argc, char const *argv[])
{
    Model teapot = get_teapot_info();

    sf::RenderWindow window(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "Hello my little sillies :3", sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(20);

    sf::Clock clock;

    Camera camera;
    camera.position[0] = 0;
    camera.position[1] = 0;
    camera.position[2] = -3;
    camera.angle[0] = 0;
    camera.angle[1] = 0;
    camera.fovx = 0.79;
    camera.fovy = (WIN_HEIGHT/WIN_WIDTH) * camera.fovx;

    window.setView(sf::View(sf::FloatRect(-camera.fovx, camera.fovy, 2*camera.fovx, -2*camera.fovy)));

    while (Update(window, camera, clock))
    {
        Render(window, teapot, camera);
    }
    

    return 0;
}

