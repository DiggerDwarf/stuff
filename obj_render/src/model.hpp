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
    typedef std::array<std::array<uint, 3>, 3> face;

    #define SFML_STATIC
    #include <SFML/Graphics.hpp>    // Graphics

#endif


struct Model    // Model data storage unit
{
    coord position;
    std::vector<coord> vertices;        // Geometric vertex data in x, y, z form
    std::vector<coord> normals;         // Vertex normal vectors
    std::vector<coord> textureCoords;   // Texture coordinates
    std::vector<face>  faces;           // Faces vertex indices `(vertex[3], normals[3], uvs[3])`
};

struct File
{
private:
    FILE* file;             // Actual file object
    char current;           // Current char in file at pos
    fpos_t pos;             // Position in file
    bool isOpen = false;    // Is a file open in this struct
public:
    // Advance in the file
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
    // Go back in the file
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
    // Fetch current character
    char  operator()()
    {
        return this->isOpen ? this->current : '\0';
    }
    // Open a file
    void open(const char* fileName)
    {
        this->isOpen = true;
        this->file = fopen(fileName, "rb");
        fgetpos(this->file, &this->pos);
        this->current = fgetc(this->file);
        fsetpos(this->file, &this->pos);
    }
    // Close the file
    void close()
    {
        this->isOpen = false;
        fclose(this->file);
    }

    void scan(const char* format, va_list __local_argv)
    {
        vfscanf(this->file, format, __local_argv);
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

    while (data() >= '0' && data() <= '9')
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

    std::array<float, 3> tempVertex;
    std::array<uint, 3> tempFace;

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
            else if (*charPtr == 't') target = &(model.textureCoords);
            charPtr++;
            for (int i = 0; i < 3; i++)
            {
                tempVertex[i] = read_float(charPtr);
                charPtr++;
            }
            charPtr--;
            target->push_back(tempVertex);
        }
        else if (*charPtr == 'f')
        {
            charPtr += 2;
            for (int i = 0; i < 3; i++)
            {
                tempFace[i] = read_uint(charPtr)-1;
                charPtr++;
            }
            charPtr--;
            model.faces.push_back({tempFace, {0, 0, 0}, {0, 0, 0}});
        }
        charPtr++;
    }
    
    // std::cout << model.vertices.size() << " vertices have been found.\n";
    // std::cout << model.faces.size() << " triangles have been found.\n";

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

    coord tempVertex;
    face tempFace;

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
            else if (data() == 't') target = &(model.textureCoords);
            data++;
            for (int i = 0; i < 3; i++)
            {
                tempVertex[i] = read_float(data);
                data++;
            }
            data--;
            target->push_back(tempVertex);
        }
        else if (data() == 'f')
        {
            data++;
            data++;
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    if (data() < '0' or data() > '9')
                    {
                        tempFace[j][i] = 0;
                        if (data() == '/') data++;
                        continue;
                    }
                    tempFace[j][i] = read_uint(data)-1;
                }
                
                data++;
            }
            data--;
            model.faces.push_back(tempFace);
        }
        data++;
    }
    
    // std::cout << model.vertices.size() << " vertices have been found.\n";
    // std::cout << model.faces.size() << " triangles have been found.\n";

    data.close();

    return model;
}

#endif