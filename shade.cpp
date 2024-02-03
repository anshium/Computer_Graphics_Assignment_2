#include "shade.h"

Vector3f shade(Light light, Vector3f color){
	return (color / 3.14) * light.radiance[0];
}