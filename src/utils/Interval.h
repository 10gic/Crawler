#ifndef INTERVAL_H
#define INTERVAL_H

class Interval
{
private:
    /** Position in the interval */
    uint pos;
    /** Size of the interval */
    uint size;
public:
    /** Constructor */
    Interval(uint size) { this->size = size; pos = 0; }
    /** Destructor */
    ~Interval() {}
    /** How many urls can we put
    * answer 0 if no urls can be put
    */
    inline uint putAll() { int res = size - pos; pos = size; return res; }
    /** Warn an url has been retrieved */
    inline void getOne() { pos--; }
    /** only for debugging, handle with care */
    inline uint getPos() { return pos; }
};

#endif