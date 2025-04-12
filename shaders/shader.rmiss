#version 460
#extension GL_EXT_ray_tracing : enable

struct stHitValue{
	vec3 color;
	bool miss;
};

layout(location=0) rayPayloadInEXT stHitValue hitValue;

void main()
{
	hitValue.color = vec3(0.0, 0.0, 0.2);
	hitValue.miss = true;
}