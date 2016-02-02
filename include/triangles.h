#include <string>
#include <vector>

// Basic type definitions
#if defined(FP64)
 typedef double real_t;                            //!< Floating point type is double precision
#else
 typedef float real_t;                             //!< Floating point type is single precision
#endif

struct Triangle;
inline void printTriangle(std::vector<struct Triangle> vec);

struct Point
{
    Point() { for(int i=0; i<=2; i++) data[i] = (real_t)0.0; }
    Point(real_t x, real_t y , real_t z)
    {
        data[0] = x;
        data[1] = y;
        data[2] = z;
    }
    
     const real_t &at(int i) const { return data[i]; }
     bool operator==(const Point &other) const { return (other.at(0) == this->at(0) && other.at(1) == this->at(1) && other.at(2) == this->at(2)); }

    real_t data[3];
};


struct Triangle
{
    Triangle(){};
    Point node[3];

   bool operator==(const Triangle &other) const {
        return(other.node[0] == node[0] && other.node[1] == node[1] && other.node[2] == node[2]); }

};
