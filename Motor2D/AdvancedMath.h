#ifndef __ADVANCED_MATH_H__
#define __ADVANCED_MATH_H__

float lerp(float v0, float v1, float t) {
	return (1 - t)*v0 + t*v1;
}

#endif