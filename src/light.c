#include "light.h"

Light light_create(Vector3 position, Vector3 color, double intensity) {
    Light l = {position, color, intensity};
    return l;
}
