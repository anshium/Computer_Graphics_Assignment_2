#include "shade.h"

// Vector3f shade(Light light, Vector3f color){
// 	return (color / 3.14) * light.radiance[0];
// }
Vector3f shade(Light light, Vector3f color){
	return (Vector3f({light.radiance.x * color.x, light.radiance.y * color.y, light.radiance.z * color.z}) / 3.14);
}