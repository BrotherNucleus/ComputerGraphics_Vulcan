#version 460
#extension GL_EXT_ray_tracing : enable

struct stHitValue {
	vec3 color;
	bool miss;
	int depth;
	vec3 origin;
	vec3 direction;
	float contribution;
	bool reflection;
	bool refraction;
	bool refracted;
};

layout(location=0) rayPayloadInEXT stHitValue hitValue;

void main()
{
	hitValue.color = vec3(0.0, 0.0, 0.2);
	hitValue.miss = true;
	hitValue.reflection = false;
	hitValue.refraction = false;
	hitValue.refracted = false;
}