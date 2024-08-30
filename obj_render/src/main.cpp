//Start of the obj_render project 

#include <iostream>
#include <cmath>
#include <vector>
#include <array>

typedef unsigned int uint;
typedef std::array<float, 3> coord;

#define SFML_STATIC
#include <SFML/Graphics.hpp>

#define WIN_WIDTH  1920.F
#define WIN_HEIGHT 1080.F

#define MOV_SPEED 2.5F
#define ROT_SPEED 1.5F

#define SENSITIVITY 0.005

struct Model
{
    std::vector<coord> vertices;
    std::vector<coord> normals;
    std::vector<std::array<uint , 3>> faces;
    sf::Vector2f* projectedBuffer;
};

struct Mat3
{
    float a1;
    float a2;
    float a3;
    float b1;
    float b2;
    float b3;
    float c1;
    float c2;
    float c3;
};

Mat3 operator*(Mat3 m1, Mat3 m2)
{
    return Mat3({
        (m1.a1*m2.a1) + (m1.a2*m2.b1) + (m1.a3*m2.c1),
        (m1.a1*m2.a2) + (m1.a2*m2.b2) + (m1.a3*m2.c2),
        (m1.a1*m2.a3) + (m1.a2*m2.b3) + (m1.a3*m2.c3),
        (m1.b1*m2.a1) + (m1.b2*m2.b1) + (m1.b3*m2.c1),
        (m1.b1*m2.a2) + (m1.b2*m2.b2) + (m1.b3*m2.c2),
        (m1.b1*m2.a3) + (m1.b2*m2.b3) + (m1.b3*m2.c3),
        (m1.c1*m2.a1) + (m1.c2*m2.b1) + (m1.c3*m2.c1),
        (m1.c1*m2.a2) + (m1.c2*m2.b2) + (m1.c3*m2.c2),
        (m1.c1*m2.a3) + (m1.c2*m2.b3) + (m1.c3*m2.c3),
    });
}
void operator*=(Mat3 m1, Mat3 m2)
{
    m1 = m1*m2;
}
coord operator*(Mat3 m, coord c)
{
    return coord({
        (c[0]*m.a1) +(c[1]*m.b1) +(c[2]*m.c1),
        (c[0]*m.a2) +(c[1]*m.b2) +(c[2]*m.c2),
        (c[0]*m.a3) +(c[1]*m.b3) +(c[2]*m.c3),
    });
}
coord operator-(coord c1, coord c2)
{
    return coord({c1[0]-c2[0], c1[1]-c2[1], c1[2]-c2[2]});
}

bool can_be_drawed(sf::Vertex vertices[2])
{
    return ((vertices[0].position.x <= 1) && (vertices[0].position.x >= -1) && (vertices[0].position.y <= 1) && (vertices[0].position.y >= -1)) || \
           ((vertices[1].position.x <= 1) && (vertices[1].position.x >= -1) && (vertices[1].position.y <= 1) && (vertices[1].position.y >= -1));
}

struct Camera
{
    coord position;
    float angle[2];
    float fovx;
    float fovy;
    Mat3 rotationMatrix;
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

Model get_model_info(const char* dataStart, size_t dataSize)
{
    Model model;

    const char* charPtr = dataStart;

    std::array<float, 3> tempF;
    std::array<uint, 3> tempUI;

    std::vector<coord>* target;

    while (true)
    {
        if (charPtr > dataStart + dataSize) break;
        else if (*charPtr == '\0') break;
        else if (*charPtr == '#')
        {
            while (not(*charPtr == '\n' || *charPtr == '\r'))
            {
                charPtr++;
            }
        }
        else if (*charPtr == 'v')
        {
            charPtr++;
            if (*charPtr == ' ') target = &(model.vertices);
            else if (*charPtr == 'n') target = &(model.normals);
            charPtr++;
            for (int i = 0; i < 3; i++)
            {
                tempF[i] = read_float(charPtr);
                charPtr++;
            }
            charPtr--;
            target->push_back(tempF);
        }
        else if (*charPtr == 'f')
        {
            charPtr += 2;
            for (int i = 0; i < 3; i++)
            {
                tempUI[i] = read_uint(charPtr)-1;
                charPtr++;
            }
            charPtr--;
            model.faces.push_back(tempUI);
            // std::cout << "< " << tempUI[0] << ", " << tempUI[1] << ", " << tempUI[2] << " >\n";
        }
        charPtr++;
    }
    
    // std::cout << model.vertices.size() << " vertices have been found.\n";
    // std::cout << model.faces.size() << " triangles have been found.\n";

    model.projectedBuffer = new sf::Vector2f[model.vertices.size()];

    return model;
}

Model get_model_info(const char* fileName)
{
    FILE* file = fopen(fileName, "r");
    Model model;

    // TODO : everything

    return model;
}

sf::Vector2f projection(coord& vertexCoord, Camera& camera)
{
    coord proj = camera.rotationMatrix*(vertexCoord-camera.position);
    if (__signbitf(proj[2]))
    {
        return sf::Vector2f(NAN, NAN);
    }
    
    return sf::Vector2f(
        proj[0]/(proj[2]*tanf(camera.fovx)),
        proj[1]/(proj[2]*tanf(camera.fovy))
    );
}

bool Update(sf::RenderWindow& window, Camera& camera, sf::Clock& clock, bool& isMousePressed, sf::Vector2i& mousePos)
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
        case sf::Event::KeyPressed:
            if (event.key.code == sf::Keyboard::Escape)
            {
                window.close();
                return false;
            }
            break;
        
        case sf::Event::MouseButtonPressed:
            isMousePressed = true;
            mousePos = sf::Mouse::getPosition();
            break;
        case sf::Event::MouseButtonReleased:
            isMousePressed = false;
            break;
            
        case sf::Event::Resized:
            camera.fovy = (((float)(event.size.height))/((float)(event.size.width))) * camera.fovx;
            break;

        default:
            break;
        }
    }

    /* Camera movement */ {
        float dm = dt * MOV_SPEED;
        float s = sin(-camera.angle[0]) * dm;
        float c = cos(-camera.angle[0]) * dm;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) { camera.position[0] += s; camera.position[2] += c; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { camera.position[0] -= s; camera.position[2] -= c; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) { camera.position[0] -= c; camera.position[2] += s; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { camera.position[0] += c; camera.position[2] -= s; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { camera.position[1] += dm; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) { camera.position[1] -= dm; }
    }

    /* Camera rotation */ {
        float dr = dt * ROT_SPEED;

        if (isMousePressed)
        {
            sf::Vector2i npos(sf::Mouse::getPosition());
            sf::Vector2i moffset(mousePos - npos);
            mousePos = npos;
            camera.angle[0] -= moffset.x * SENSITIVITY;
            camera.angle[1] -= moffset.y * SENSITIVITY;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) { camera.angle[0] -= dr; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::J)) { camera.angle[0] += dr; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::K)) { camera.angle[1] -= dr; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::I)) { camera.angle[1] += dr; }
    }

    clamp<float>(camera.angle[1], (float)-M_PI_2, (float)M_PI_2);

    camera.rotationMatrix = 
        Mat3({
            cosf(camera.angle[0]), 0,-sinf(camera.angle[0]),
            0,                     1, 0,
            sinf(camera.angle[0]), 0, cosf(camera.angle[0])
    }) *Mat3({
            1, 0,                     0,
            0, cosf(camera.angle[1]), sinf(camera.angle[1]),
            0,-sinf(camera.angle[1]), cosf(camera.angle[1])
    });

    return true;
}

void Render(sf::RenderWindow& window, Model& model, Camera& camera)
{
    window.clear();

    for (int i = 0; i < model.vertices.size(); i++)
    {
        model.projectedBuffer[i] = projection(model.vertices[i], camera);
    }
    
    sf::Vertex temp[2];

    for (std::array<uint, 3> face : model.faces)
    {
        // std::cout << "< " << face[0] << ", " << face[1] << ", " << face[2] << " >\n";
        temp[0] = model.projectedBuffer[face[0]];
        // printf("hey 1\n");
        temp[1] = model.projectedBuffer[face[1]];
        // printf("hey 2\n");
        if (can_be_drawed(temp)) window.draw(temp, 2, sf::Lines);
        temp[1] = model.projectedBuffer[face[2]];
        // printf("hey 3\n");
        if (can_be_drawed(temp)) window.draw(temp, 2, sf::Lines);
        temp[0] = model.projectedBuffer[face[2]];
        // printf("hey 4\n");
        if (can_be_drawed(temp)) window.draw(temp, 2, sf::Lines);
    }

    window.display();
}

int main(int argc, char const *argv[])
{
    extern const char _binary_obj_cow_obj_start[], _binary_obj_cow_obj_size[];
    const char* binDataStart = _binary_obj_cow_obj_start;
    size_t binDataSize = (size_t)_binary_obj_cow_obj_size;

    Model model = get_model_info(binDataStart, binDataSize);
    
    sf::RenderWindow window(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "Model rendering", sf::Style::Close | sf::Style::Titlebar); //| sf::Style::Resize);
    window.setVerticalSyncEnabled(true);

    sf::Clock clock;

    Camera camera;
    camera.position[0] = 0;
    camera.position[1] = 0.5;
    camera.position[2] = -5;
    camera.angle[0] = 0;
    camera.angle[1] = 0;
    camera.fovx = 0.79;
    camera.fovy = (WIN_HEIGHT/WIN_WIDTH) * camera.fovx;

    window.setView(sf::View(sf::FloatRect(-1, 1, 2, -2)));

    bool isMousePressed(false);
    sf::Vector2i mousePos(0,0);

    

    while (Update(window, camera, clock, isMousePressed, mousePos))
    {
        Render(window, model, camera);
    }
    

    return 0;
}
