#ifndef COLOR_H
#define COLOR_H

/**
 * RGBA color structure for graphics rendering
 * Used across multiple systems: Tank colors, Bullet colors, FX colors, etc.
 */
struct Color {
    float r, g, b, a;
    
    // Default constructor with full alpha
    Color(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f)
        : r(r), g(g), b(b), a(a) {}
    
    // Copy constructor
    Color(const Color& other) = default;
    
    // Assignment operator
    Color& operator=(const Color& other) = default;
};

#endif // COLOR_H