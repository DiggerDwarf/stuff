#ifndef MODEL_INCLUDED
#define MODEL_INCLUDED

#ifndef MAIN_INCLUDES
#define MAIN_INCLUDES

    #include <iostream> // I/O
    #include <cmath>    // Meth
    #include <vector>   // Variable-length storage
    #include <list>     // double-linked list; can keep pointers stable
    #include <array>    // Static-length storage

    // Shortcuts
    typedef unsigned int uint;
    typedef std::array<float, 3> coord;

    #define SFML_STATIC
    #include <SFML/Graphics.hpp>    // Graphics

#endif


struct Model    // Model data storage unit
{
    coord position;
    std::vector<coord> vertices;                // Geometric vertex data in x, y, z form
    std::vector<coord> normals;                 // Vertex normal vectors
    std::vector<std::array<uint , 3>> faces;    // Faces vertex indices
    sf::Vector2f* projectedBuffer;              // Buffer for storing the last projection of the vertices
    bool* clipMask;                             // Buffer for storing wether a vertex should be drawn according to its projected coordinates
};

struct File
{
private:
    FILE* file;
    char current;
    fpos_t pos;
    bool isOpen = false;
public:
    File& operator++()
    {
        if (!this->isOpen) return *this;
        pos++;
        fsetpos(this->file, &this->pos);
        if ((this->current = fgetc(this->file)) == EOF)
        {
            this->current = '\0';
        }
        fsetpos(this->file, &this->pos);
        return *this;
    }
    File  operator++(int)
    {
        return ++*this;
    }
    File& operator--()
    {
        if (!this->isOpen) return *this;
        pos--;
        fsetpos(this->file, &this->pos);
        if ((this->current = fgetc(this->file)) == EOF)
        {
            this->current = '\0';
        }
        fsetpos(this->file, &this->pos);
        return *this;
    }
    File  operator--(int)
    {
        return --*this;
    }
    char  operator()()
    {
        return this->isOpen ? this->current : '\0';
    }
    void open(const char* fileName)
    {
        this->isOpen = true;
        this->file = fopen(fileName, "rb");
        fgetpos(this->file, &this->pos);
        this->current = fgetc(this->file);
        fsetpos(this->file, &this->pos);
    }
    void close()
    {
        this->isOpen = false;
        fclose(this->file);
    }
};

float read_float(const char* (& data));
uint read_uint(const char* (& data));
float read_float(File& data);
uint read_uint(File& data);


// Reads and outputs the value of a float from a char buffer
// The char pointer must be located on the first character of the float
// The char pointer will be advanced to the character after the float
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

// Reads and outputs the value of a uint from a char buffer
// The char pointer must be located on the first character of the uint
// The char pointer will be advanced to the character after the uint
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


// Reads and outputs the value of a float from a file
// The reading position must be located on the first character of the float
// The reading position will be advanced to the character after the float
float read_float(File &data)
{
    float result = 0;
    float sign = +1;
    float where = 0;
    int expval = 0;

    while (data() != ' ' && data() != '\r' && data() != '\n' && data() != '\0')
    {
        if(data() == '-')
        {
            sign = -1;
        }
        else if (data() == '.')
        {
            where = 1;
        }
        else
        {
            if (where == 0) result *= 10;
            result += (data() - '0') / powf(10.F, where);
            if (where != 0) where += 1;
        }
        
        data++;

        if (data() == 'e' || data() == 'E')
        {
            data++;
            expval = (data() == '-') ? -1 : +1;
            data++;
            expval *= static_cast<int>(read_uint(data));
        }
        
    }

    result *= sign;
    result *= pow(10, expval);

    return result;
}

// Reads and outputs the value of a uint from a file
// The reading position must be located on the first character of the uint
// The reading position will be advanced to the character after the uint
uint read_uint(File &data)
{
    float result = 0;

    while (data() != ' ' && data() != '\r' && data() != '\n' && data() != '\0')
    {
        result *= 10;
        result += data() - '0';
        data++;
    }

    return result;
}


// Extracts model data in the wavefront obj format from a char buffer
// If the size of the data chunk is unknown,
// leave blank and the buffer will be read up to the first null-terminator
// Currently supported :
//  - geometry vertices
//  - normals
//  - face indexed by geometry vertices only
Model get_model_info(const char* dataStart, size_t dataSize = (size_t)INFINITY)
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
            while (not(*charPtr == '\n' || *charPtr == '\r' || *charPtr == '\0'))
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
        }
        charPtr++;
    }
    
    // std::cout << model.vertices.size() << " vertices have been found.\n";
    // std::cout << model.faces.size() << " triangles have been found.\n";

    model.projectedBuffer = new sf::Vector2f[model.vertices.size()];
    model.clipMask = new bool[model.vertices.size()];

    return model;
}

// Extracts model data in the wavefront obj format from a file path
//
// TODO : read_float and read_uint for fstreams
//
// Currently supported :
//  - geometry vertices
//  - normals
//  - face indexed by geometry vertices only
Model get_model_info_file(const char* fileName)
{
    Model model;

    File data;
    data.open(fileName);

    std::array<float, 3> tempF;
    std::array<uint, 3> tempUI;

    std::vector<coord>* target;

    while (true)
    {
        if (data() == '\0') break;
        else if (data() == '#')
        {
            while (not(data() == '\n' || data() == '\r' || data() == '\0'))
            {
                data++;
            }
        }
        else if (data() == 'v')
        {
            data++;
            if (data() == ' ') target = &(model.vertices);
            else if (data() == 'n') target = &(model.normals);
            data++;
            for (int i = 0; i < 3; i++)
            {
                tempF[i] = read_float(data);
                data++;
            }
            data--;
            target->push_back(tempF);
        }
        else if (data() == 'f')
        {
            data++;
            data++;
            for (int i = 0; i < 3; i++)
            {
                tempUI[i] = read_uint(data)-1;
                data++;
            }
            data--;
            model.faces.push_back(tempUI);
        }
        data++;
    }
    
    // std::cout << model.vertices.size() << " vertices have been found.\n";
    // std::cout << model.faces.size() << " triangles have been found.\n";

    model.projectedBuffer = new sf::Vector2f[model.vertices.size()];
    model.clipMask = new bool[model.vertices.size()];

    data.close();

    return model;
}

#endif