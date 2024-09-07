//Start of the obj_render project 

#ifndef MAIN_INCLUDES
#define MAIN_INCLUDES

    #include <iostream> // I/O
    #include <cmath>    // Meth
    #include <vector>   // Variable-length storage
    #include <array>    // Static-length storage

    // Shortcuts
    typedef unsigned int uint;
    typedef std::array<float, 3> coord;

    #define SFML_STATIC
    #include <SFML/Graphics.hpp>    // Graphics

#endif

#include <windows.h>
#include <commdlg.h>
#include "model.hpp"

// Initial window size
#define WIN_WIDTH  1920.F
#define WIN_HEIGHT 1080.F

// Movement and rotation speed through key inputs (units/s)
#define MOV_SPEED 2.5F
#define ROT_SPEED 1.5F

// Mouse drag sensitivity
#define SENSITIVITY 0.005


struct Mat3     // 3x3 Matrix data
{
    float a1;   // Top left
    float a2;   // Top middle
    float a3;   // Top right
    float b1;   // Middle left
    float b2;   // Center
    float b3;   // Middle right
    float c1;   // Bottom left
    float c2;   // Bottom middle
    float c3;   // Bottom right
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

// Returns wether a projected line segment is contained within the view frustum and must be drawed
bool can_be_drawed(sf::Vertex vertices[2], int nb_vertices)
{
    for (int i = 0; i < nb_vertices; i++)
    {
        if ((vertices[0].position.x <= 1) && (vertices[0].position.x >= -1) && (vertices[0].position.y <= 1) && (vertices[0].position.y >= -1)) return true;
    }
    return false;
    
    // return ((vertices[0].position.x <= 1) && (vertices[0].position.x >= -1) && (vertices[0].position.y <= 1) && (vertices[0].position.y >= -1)) // \
    //        ((vertices[1].position.x <= 1) && (vertices[1].position.x >= -1) && (vertices[1].position.y <= 1) && (vertices[1].position.y >= -1));
}

bool can_be_drawed(sf::Vertex vertex)
{
    return ((vertex.position.x <= 1) && (vertex.position.x >= -1) && (vertex.position.y <= 1) && (vertex.position.y >= -1));
}

struct Camera   // Camera data storage unit
{
    coord position;         // Position of the camera
    float angle[2];         // Angles of the camera (xz plane angle then up-down angle)
    float fovx;             // Fov of the camera
    float fovy;             // Please set this value to camera.fovx * (window.height / window.width)
    Mat3 rotationMatrix;    // Rotation matrix of the camera, to be updated after each camera angle change
};

// Changes the value of the variable given to be between the low and high thresholds
template <typename T>
void clamp(T& val, T low, T high){
    val = std::min(std::max(low, val), high);
}


// Projects a coord to screen space [-1,1] relative to a camera
sf::Vector2f projection(coord& vertexCoord, Camera& camera)
{
    // Center and align the vertices relative to the camera
    coord proj = camera.rotationMatrix*(vertexCoord-camera.position);

    // Exclude vertices behind the camera
    if (__signbitf(proj[2]))
    {
        return sf::Vector2f(NAN, NAN);
    }
    
    // Normalize the height relative to the frustum
    return sf::Vector2f(
        proj[0]/(proj[2]*tanf(camera.fovx)),
        proj[1]/(proj[2]*tanf(camera.fovy))
    );
}

// Handles key inputs and updates the camera
bool Update(sf::RenderWindow& window, Camera& camera, sf::Clock& clock, bool& isMousePressed, sf::Vector2i& mousePos, Model* model)
{
    // Get deltaTime to make a smooth experience
    float dt = clock.restart().asSeconds();

    // Cycle through received events
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
            else if (event.key.code == sf::Keyboard::O && event.key.control)
            {
                char filename[ MAX_PATH ];

                OPENFILENAMEA ofn;
                    ZeroMemory( &filename, sizeof( filename ) );
                    ZeroMemory( &ofn,      sizeof( ofn ) );
                    ofn.lStructSize  = sizeof( ofn );
                    ofn.hwndOwner    = window.getSystemHandle();  // If you have a window to center over, put its HANDLE here
                    ofn.lpstrFilter  = "Any File\0*.*\0";
                    ofn.lpstrFile    = filename;
                    ofn.nMaxFile     = MAX_PATH;
                    ofn.lpstrTitle   = "Select a model to open :";
                    ofn.Flags        = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
                
                if (GetOpenFileNameA( &ofn ))
                {
                    *model = get_model_info_file(filename);
                }
                else std::cout << "Error: Could not open model.\n";
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

    if (window.hasFocus()) {

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

    // Lock the up-down rotation to not look behind upside down
    clamp<float>(camera.angle[1], (float)-M_PI_2, (float)M_PI_2);

    // Update the camera's rotation matrix
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

    } // if has focus end
    return true;
}

// Render a single Model to the window using a camera as viewpoint
void Render(sf::RenderWindow& window, Model& model, Camera& camera)
{
    if (window.hasFocus()) {

    // Clear the previous frame
    window.clear();

    // Project the vertices to the camera's screen space and store them in the model's buffer
    for (int i = 0; i < model.vertices.size(); i++)
    {
        model.projectedBuffer[i] = projection(model.vertices[i], camera);
    }

    // Wireframe render, line by line of each triangle
    sf::Vertex temp[3];
    for (std::array<uint, 3> face : model.faces)
    {
        // temp[0] = model.projectedBuffer[face[0]];
        // temp[1] = model.projectedBuffer[face[1]];
        // if (can_be_drawed(temp)) window.draw(temp, 2, sf::Lines);
        // temp[1] = model.projectedBuffer[face[2]];
        // if (can_be_drawed(temp)) window.draw(temp, 2, sf::Lines);
        // temp[0] = model.projectedBuffer[face[1]];
        // if (can_be_drawed(temp)) window.draw(temp, 2, sf::Lines);

        temp[0] = model.projectedBuffer[face[0]];
        temp[1] = model.projectedBuffer[face[1]];
        temp[2] = model.projectedBuffer[face[2]];

        if (can_be_drawed(temp, 3)) window.draw(temp, 3, sf::Triangles);

    }

    } // if has focus end

    // Display the result to the screen
    window.display();
}

int main(int argc, char const *argv[])
{
    // Linked model data
    extern const char _binary_obj_cow_obj_start[], _binary_obj_cow_obj_size[];
    const char* binDataStart = _binary_obj_cow_obj_start;
    size_t binDataSize = (size_t)_binary_obj_cow_obj_size;

    // Extract model info
    Model model = get_model_info(binDataStart, binDataSize);
    
    // Setup the window
    sf::RenderWindow window(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "Model rendering", sf::Style::Close | sf::Style::Titlebar); //| sf::Style::Resize);
    window.setVerticalSyncEnabled(true);
    window.setView(sf::View(sf::FloatRect(-1, 1, 2, -2)));

    sf::Clock clock;

    // Set the initial state of the camera
    Camera camera;
    camera.position[0] = 0;
    camera.position[1] = 0.5;
    camera.position[2] = -5;
    camera.angle[0] = 0;
    camera.angle[1] = 0;
    camera.fovx = 0.79;
    camera.fovy = (WIN_HEIGHT/WIN_WIDTH) * camera.fovx;

    // Required info for mouse drag
    bool isMousePressed(false);
    sf::Vector2i mousePos(0,0);
    
    // Update-draw loop
    while (Update(window, camera, clock, isMousePressed, mousePos, &model))
    {
        Render(window, model, camera);
    }
    
    return 0;
}
