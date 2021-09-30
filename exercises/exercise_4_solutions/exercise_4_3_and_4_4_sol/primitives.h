//
// Created by Henrique on 9/17/2019.
//

#ifndef GRAPHICSPROGRAMMINGEXERCISES_PRIMITIVES_H
#define GRAPHICSPROGRAMMINGEXERCISES_PRIMITIVES_H

struct Primitives {
private:
    Primitives() = default;

public:
    // this class is only meant to hold information about the 3D model of the airplane
    // the getInstance and deleted functions below makes this a singleton
    static Primitives& getInstance()
    {
        static Primitives instance;
        return instance;
    }
    Primitives(Primitives const&)      = delete;
    void operator=(Primitives const&)  = delete;


    std::vector<float> cubeVertices {-1.0f, -1.0f, 1.0f,
                                     1.0f, -1.0f, 1.0f,
                                     1.0f, 1.0f, 1.0f,
                                     -1.0f, 1.0f, 1.0f,
                                     -1.0f, -1.0f, -1.0f,
                                     1.0f, -1.0f, -1.0f,
                                     1.0f, 1.0f, -1.0f,
                                     -1.0f, 1.0f, -1.0f};
    std::vector<unsigned int> cubeIndices {0, 1, 2,
                                           0, 2, 3,
                                           1, 5, 6,
                                           1, 6, 2,
                                           5, 4, 7,
                                           5, 7, 6,
                                           4, 0, 3,
                                           4, 3, 7,
                                           3, 2, 6,
                                           3, 6, 7,
                                           1, 0, 4,
                                           1, 4, 5};
    std::vector<float> cubeColors {.8f, .4f, .4f, 1.f,
                                   .7f, .7f, .4f, 1.f,
                                   .4f, .7f, .7f, 1.f,
                                   .7f, .4f, .7f, 1.f,
                                   .8f, .4f, .4f, 1.f,
                                   .7f, .7f, .4f, 1.f,
                                   .4f, .7f, .7f, 1.f,
                                   .7f, .4f, .7f, 1.f};


    std::vector<float> arrowVertices {-0.1f, 0.0f, 0.0f,
                                      0.1f, 0.0f, 0.0f,
                                      0.0f, 1.0f, 0.0f};
    std::vector<unsigned int> arrowIndices {0, 1, 2};
    std::vector<float> arrowColors {1.f, 1.f, 1.f, 1.f,
                                    1.f, 1.f, 1.f, 1.f,
                                    1.f, 1.f, 1.f, 1.f};

    void invertModelZ() {
        for(int i = 2; i < cubeVertices.size(); i+=3)
            cubeVertices[i] *= -1;
        for(int i = 2; i < arrowVertices.size(); i+=3)
            arrowVertices[i] *= -1;
    }
};


#endif //GRAPHICSPROGRAMMINGEXERCISES_PRIMITIVES_H
