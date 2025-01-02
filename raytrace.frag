#version 460 core
precision highp float;

const float infinity = 0x7F800000;

struct Material
{
	vec3 color;
	float roughness;

	vec3 emissionColor;
	float emissionStrength;

	vec3 absorbColor;
	float absorbsionStrength;

	float emissionScatteringIndex;
	float refractiveIndex;
	float reflectiveIndex;
	float padding;
};

struct Sphere
{
	vec3 position;
	float radius;

	Material material;
};

struct Triangle
{
	vec4 p[3];
};

struct BoundingBox
{
	vec3 min;
	float padding;
	vec3 max;
	float padding2;

	int triangleIndex;
	int boundingBoxAIndex;
	int boundingBoxBIndex;
	float padding3;
};

struct Mesh
{
	mat4 localToWorldMatrix;
	mat4 modelWorldToLocalMatrix;

	int triangleIndex;
	int nTriangles;

	int boundingBoxIndex;
	int nBoundingBoxes;

	Material material;
};

struct HitInfo
{
	int didHit;
	float distance;
	vec3 point;
	vec3 normal;
	vec3 flippedNormal;

	Material material;
};

struct Ray
{
	vec3 origin;
	vec3 normal;

	// The dot product between the normal of the surface the ray is resting on and the ray normal 
	float surfaceNormalDot;
};

Material EmptyMaterial()
{
	return Material(vec3(0.0f), 0.0f, vec3(0.0f), 0.0f, vec3(0.0f), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

HitInfo EmptyHitInfo()
{
	return HitInfo(0, 0.0f, vec3(0.0f), vec3(0.0f), vec3(0.0f), Material(vec3(0.0f), 0.0f, vec3(0.0f), 0.0f, vec3(0.0f), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
}













// Scene uniforms
uniform uint nSpheres;
layout(std430, binding = 0) buffer sphereBuffer {
    Sphere spheres[];
};
uniform uint nMeshes;
layout(std430, binding = 1) buffer meshBuffer {
    Mesh meshes[];
};
uniform uint nTriangles;
layout(std430, binding = 2) buffer triangleBuffer {
    Triangle triangles[];
};
uniform uint nBoundingBoxes;
layout(std430, binding = 3) buffer boundingBoxBuffer {
    BoundingBox boundingBoxes[];
};

uniform mat4 cameraRotation;
uniform vec3 cameraPosition;

uniform sampler2D skybox;

// Runtime dependent uniforms
uniform uint frame;
uniform float aspectRatio;

// Raytracing settings
uniform int maxBounces;
uniform int samplesPerPixel;
uniform float perspectiveSlope;
uniform float focalDistance;
uniform float focalBlur;
uniform float blur;













float Random(inout uint seed)
{
	seed = seed * 747796405u + 2891336453u;
	uint result = ((seed >> ((seed >> 28) + 4)) ^ seed) * 277803737u;
	result = (result >> 22) ^ result;
	return float(result) / 4294967295.0f;
}

float NormalDistribution(inout uint seed)
{
	float theta = 2.0f * 3.14159265359f * Random(seed);
	float rho = sqrt(-2.0f * log(Random(seed)));
	return rho * cos(theta);
}

vec3 RandomHemisphereNormal(inout uint seed, vec3 normal)
{
	vec3 randomNormal = normalize(vec3(NormalDistribution(seed), NormalDistribution(seed), NormalDistribution(seed)));

	if (dot(randomNormal, normal) < 0.0f)
	{
		randomNormal = -randomNormal;
	}

	return randomNormal;
}

vec2 RandomPointInCircle(inout uint seed)
{
	float angle = Random(seed) * 2 * 3.1415926f;
	vec2 pointInCircle = vec2(cos(angle), sin(angle));
	return pointInCircle * sqrt(Random(seed));
}













vec4 PerspectiveDivide(vec4 v)
{
	return vec4(v.xyz / v.w, 1.0f);
}

vec3 Reflect(vec3 v, vec3 normal)
{
	return (v - (normal * dot(v, normal) * 2.0f));
}

vec3 Refract(vec3 I, vec3 N, float ior)
{
	float cosi = clamp(-1, 1, dot(I, N));
    float etai = 1, etat = ior;
    vec3 n = N;
    if (cosi < 0) 
	{ 
		cosi = -cosi;
	} 
	else 
	{ 
		float temp = etai;
		etai = etat;
		etat = temp;
		n = -N; 
	}
    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi);

	if (k < 0)
	{
		return vec3(0.0f);
	}

    return normalize(eta * I + (eta * cosi - sqrt(k)) * n);
}

float FresnelReflectAmount(vec3 normal, vec3 incident, float n1, float n2, float objReflect)
{
    // Schlick aproximation
    float r0 = (n1-n2) / (n1+n2);
    r0 *= r0;
    float cosX = -dot(normal, incident);
    if (n1 > n2)
    {
        float n = n1/n2;
        float sinT2 = n*n*(1.0-cosX*cosX);

        // Total internal reflection
        if (sinT2 > 1.0)
		{
            return 1.0;
		}

		cosX = sqrt(1.0-sinT2);
    }
    float x = 1.0 - cosX;
    float ret = r0 + (1.0 - r0) * x * x * x * x * x;

    // Adjust reflect multiplier for object reflectivity
    return objReflect + (1.0-objReflect) * ret;
}

void ReflectRay(inout Ray ray, HitInfo hitInfo, inout uint seed)
{
	// Reflect ray
	vec3 reflectedNormal = Reflect(ray.normal, hitInfo.normal);
	vec3 randomNormal = RandomHemisphereNormal(seed, hitInfo.normal);

	// Combine the two
	ray.normal = reflectedNormal * (1.0f - hitInfo.material.roughness) + randomNormal * hitInfo.material.roughness;
	ray.normal = normalize(ray.normal);
	ray.origin = hitInfo.point;
	ray.surfaceNormalDot = dot(hitInfo.normal, reflectedNormal);
}

void RefractRay(inout Ray ray, HitInfo hitInfo, inout uint seed)
{
	// Refract ray
	vec3 refractedNormal = Refract(ray.normal, hitInfo.normal, hitInfo.material.refractiveIndex);
	vec3 randomNormal = RandomHemisphereNormal(seed, hitInfo.flippedNormal);

	// Combine the two
	ray.normal = refractedNormal * (1.0f - hitInfo.material.roughness) + randomNormal * hitInfo.material.roughness;
	ray.normal = normalize(ray.normal);
	ray.origin = hitInfo.point;
	ray.surfaceNormalDot = dot(hitInfo.normal, refractedNormal);
}













HitInfo HitSphere(Ray ray, Sphere sphere)
{
	vec3 offsetRayOrigin = ray.origin - sphere.position;
	float b = 2.0f * dot(offsetRayOrigin, ray.normal);
	
	float discriminant = b * b - 4.0f * (dot(offsetRayOrigin, offsetRayOrigin) - sphere.radius * sphere.radius);
	if (discriminant <= 0.0f)
	{
		return EmptyHitInfo();
	}
	
	float distance = 0.5f * (-b - sqrt(discriminant));
	if (distance < 0.0f || (ray.surfaceNormalDot < 0.0f && ray.surfaceNormalDot < 2))
	{
		distance = 0.5f * (-b + sqrt(discriminant));
		if (distance < 0.0f || (ray.surfaceNormalDot > 0.0f && ray.surfaceNormalDot < 2))
		{
			return EmptyHitInfo();
		}
	}

	vec3 point = ray.origin + ray.normal * distance;
	vec3 normal = normalize(point - sphere.position);

	vec3 flippedNormal = normal;
	if (dot(ray.normal, normal) > 0.0f) flippedNormal = -flippedNormal;

	return HitInfo(1, distance, point, normal, flippedNormal, sphere.material);
}

HitInfo HitTriangle(Ray ray, Triangle triangle)
{
	vec3 edgeAB = triangle.p[1].xyz - triangle.p[0].xyz;
	vec3 edgeAC = triangle.p[2].xyz - triangle.p[0].xyz;
	vec3 normal = cross(edgeAB, edgeAC);

	float determinant = -dot(ray.normal, normal);
	
	if (determinant == 0.0f || (determinant < 0.0f && ray.surfaceNormalDot > 0.0f) || (determinant > 0.0f && ray.surfaceNormalDot < 0.0f))
	{
		return EmptyHitInfo();
	}

	vec3 ao = ray.origin - triangle.p[0].xyz;

	float invDet = 1.0f / determinant;

	float distance = dot(ao, normal) * invDet;
	if (distance <= 0.0f)
	{
		return EmptyHitInfo();
	}

	vec3 dao = cross(ao, ray.normal);

	float u = dot(edgeAC, dao) * invDet;
	float v = -dot(edgeAB, dao) * invDet;

	if (u < 0 || v < 0 || 1.0f - u - v < 0) return EmptyHitInfo();

	vec3 point = ray.origin + ray.normal * distance;

	normal = normalize(normal);
	vec3 flippedNormal = normal;
	if (determinant < 0.0f) flippedNormal = -flippedNormal;

	return HitInfo(1, distance, point, normal, flippedNormal, EmptyMaterial());
}

float HitBoundingBox(Ray ray, BoundingBox boundingBox)
{
	vec3 invDirection = vec3(1.0f, 1.0f, 1.0f) / ray.normal;
	vec3 tMin = (boundingBox.min - ray.origin) * invDirection;
	vec3 tMax = (boundingBox.max - ray.origin) * invDirection;

	vec3 t1 = min(tMin, tMax);
	vec3 t2 = max(tMin, tMax);

	float dstFar = min(min(t2.x, t2.y), t2.z);
	float dstNear = max(max(t1.x, t1.y), t1.z);

	if (dstFar >= dstNear && dstFar > 0.0f)
	{
		return dstNear;
	}
	else
	{
		return infinity;
	}
}











void CheckSphereCollitions(Ray ray, inout HitInfo closestHit)
{
    for (int i = 0; i < nSpheres; i++)
	{
		Sphere sphere = spheres[i];
		HitInfo hit = HitSphere(ray, sphere);

		if (hit.didHit == 0) continue;
		if (closestHit.distance >= hit.distance || closestHit.didHit == 0)
		{
			closestHit = hit;
		}
    }
}

void CheckTriangleCollitions(Ray ray, inout HitInfo closestHit)
{
	if (nTriangles == 0) return;

	for (int meshIndex = 0; meshIndex < nMeshes; meshIndex++)
	{
		Mesh mesh = meshes[meshIndex];

		// Transform the ray instead of the object so we are able to dynamically transform the object without recalculating the bounding boxes.
		Ray transformedRay = ray;
		transformedRay.origin = (mesh.modelWorldToLocalMatrix * vec4(transformedRay.origin, 1.0f)).xyz;
		transformedRay.normal = (mesh.modelWorldToLocalMatrix * vec4(transformedRay.normal, 0.0f)).xyz;

		int currentBoxIndex = mesh.boundingBoxIndex;

		// The boxes to check stack
		int boxesToCheck[32];
		float closestIntersection[32];
		int nBoxesToCheck = 0;
		int needsNewBox = 0;

		// Check if the ray hits the root bounding box
		closestIntersection[0] = HitBoundingBox(transformedRay, boundingBoxes[currentBoxIndex]);
		if (closestIntersection[0] == infinity)
		{
			continue;
		}

		while (needsNewBox != 1 || nBoxesToCheck != 0)
		{
			if (needsNewBox == 1)
			{
				// Get the next box from stack
				nBoxesToCheck = nBoxesToCheck - 1;
				currentBoxIndex = boxesToCheck[nBoxesToCheck];

				// Check if the box is even worth checking
				if (closestIntersection[nBoxesToCheck] >= closestHit.distance && closestHit.didHit == 1)
				{
					continue;
				}

				// Successfully grabbed a new box to check
				needsNewBox = 0;
			}

			BoundingBox currentBox = boundingBoxes[currentBoxIndex];

			int boxIndexA = currentBox.boundingBoxAIndex;
			int boxIndexB = currentBox.boundingBoxBIndex;

			// Check if node is a leaf node
			if (boxIndexA == -1 || boxIndexB == -1)
			{
				if (currentBox.triangleIndex != -1)
				{
					Triangle triangle = triangles[mesh.triangleIndex + currentBox.triangleIndex];
					HitInfo hit = HitTriangle(transformedRay, triangle);
					hit.material = mesh.material;

					hit.point = (mesh.localToWorldMatrix * vec4(hit.point, 1.0f)).xyz;

					if ((hit.didHit == 1 && closestHit.distance >= hit.distance) || closestHit.didHit == 0)
					{
						closestHit = hit;
					}
				}

				needsNewBox = 1;
				continue;
			}

			boxIndexA += mesh.boundingBoxIndex;
			boxIndexB += mesh.boundingBoxIndex;

			float distanceA = HitBoundingBox(transformedRay, boundingBoxes[boxIndexA]);
			float distanceB = HitBoundingBox(transformedRay, boundingBoxes[boxIndexB]);

			// If the distance is more than the closest hit, there is no need to check any further
			if (distanceA >= closestHit.distance && closestHit.didHit == 1) distanceA = infinity;
			if (distanceB >= closestHit.distance && closestHit.didHit == 1) distanceB = infinity;

			if (distanceA != infinity && distanceB != infinity)
			{
				if (distanceA < distanceB)
				{
					// Box A is closer, push box B to stack
					boxesToCheck[nBoxesToCheck] = boxIndexB;
					closestIntersection[nBoxesToCheck] = distanceB;
					nBoxesToCheck = nBoxesToCheck + 1;

					// Set box A as the current box and recursivley check it
					currentBoxIndex = boxIndexA;
				}
				else
				{
					// Box B is closer, push box A to stack
					boxesToCheck[nBoxesToCheck] = boxIndexA;
					closestIntersection[nBoxesToCheck] = distanceA;
					nBoxesToCheck = nBoxesToCheck + 1;

					// Set box B as the current box and recursivley check it
					currentBoxIndex = boxIndexB;
				}
			}
			else if (distanceA != infinity)
			{
				currentBoxIndex = boxIndexA;
			}
			else if (distanceB != infinity)
			{
				currentBoxIndex = boxIndexB;
			}
			else
			{
				needsNewBox = 1;
			}
		}
	}
}

HitInfo RayCollition(Ray ray)
{
	HitInfo closestHit = EmptyHitInfo();

	CheckSphereCollitions(ray, closestHit);
	CheckTriangleCollitions(ray, closestHit);

	return closestHit;
}













vec2 calculatePitchYaw(vec3 direction)
{
    // Normalize the input vector to ensure it has unit length
    vec3 dir = normalize(direction);

    // Calculate the pitch (rotation around the X-axis)
    float pitch = degrees(asin(dir.y)); // asin returns radians, convert to degrees

    // Calculate the yaw (rotation around the Y-axis) using atan and manual quadrant adjustment
    float yaw = 0.0f;
    if (dir.x > 0) 
	{
        yaw = degrees(atan(dir.z / dir.x));
    }
	else if (dir.x < 0) 
	{
        yaw = degrees(atan(dir.z / dir.x)) + 180.0f;
    } 
	else 
	{
        yaw = (dir.z >= 0) ? 90.0f : -90.0f;
    }

    return vec2(pitch, yaw);
}

vec3 SkyColor(vec3 normal)
{
	//return vec3(0.0f, 0.01f, 0.06f);

	vec2 angles = calculatePitchYaw(normal);

	return texture(skybox, vec2(angles.y / 360.0f, 1.0f - (angles.x + 90.0f) / 180.0f)).xyz;

	//return vec3(normal.y / 10.0f + 0.2f, normal.y / 5.0f + 0.2f, normal.y / 3.0f + 0.5f) * 2.0f + 
	//vec3(max(dot(normal, normalize(vec3(0.1f, 1.0f, 0.4f))) - 0.99f, 0.0f) * 100.0f) * 10.0f;
}













vec3 Trace(Ray ray, inout uint seed)
{
	vec3 incomingLight = vec3(0.0f);
	vec3 rayColor      = vec3(1.0f);
	float currentRefractiveIndex = 1.0f;
	int isInsideObject = 0;

	for (int i = 0; i <= maxBounces; i++)
	{
		HitInfo hitInfo = RayCollition(ray);

		float surfaceNormalDot = dot(hitInfo.normal, ray.normal);

		if (i == 0)
		{
			// Check what starting refractive index the ray has
			if (surfaceNormalDot > 0.0f)
			{
				currentRefractiveIndex = hitInfo.material.refractiveIndex;
			}
			else
			{
				currentRefractiveIndex = 1.0f;
			}
		}

		if (hitInfo.didHit == 1)
		{
			// Check if the ray is inside of an object and adjust the current IOR accordingly
			float nextRefractiveIndex = 0.0f;
			if (surfaceNormalDot > 0.0f)
			{
				isInsideObject = 1;
				nextRefractiveIndex = 1.0f;
			}
			else
			{
				isInsideObject = 0;
				nextRefractiveIndex = hitInfo.material.refractiveIndex;
			}

			float fresnelReflectIndex = FresnelReflectAmount(hitInfo.flippedNormal, ray.normal, currentRefractiveIndex, nextRefractiveIndex, hitInfo.material.reflectiveIndex);
			
			// NaN
			if (fresnelReflectIndex == 0x7fbfffff)
			{
				return vec3(1.0f, 0.0f, 1.0f);
			}

			if (Random(seed) <= fresnelReflectIndex)
			{
				// Reflect ray
				ReflectRay(ray, hitInfo, seed);
				// Multiply the ray color with the material color
				rayColor = rayColor * hitInfo.material.color;
			}
			else
			{
				// Refract the ray
				RefractRay(ray, hitInfo, seed);
				// If we refract and we are not inside of an object we know that we just passed through an object
				if (isInsideObject == 1)
				{
					float distance = hitInfo.distance;
					vec3 absorb = exp(-hitInfo.material.absorbColor * hitInfo.material.absorbsionStrength * distance);
					rayColor *= absorb;
				}

				currentRefractiveIndex = nextRefractiveIndex;
			}

			// Only add emission if the ray is as parallell as specified if the material
			if (surfaceNormalDot < -1.0f + hitInfo.material.emissionScatteringIndex)
			{
				incomingLight += hitInfo.material.emissionColor * hitInfo.material.emissionStrength * rayColor;
			}
		}
		else
		{
			// Ray shooting off to sky, so we add the sky color
			incomingLight += SkyColor(ray.normal) * rayColor;
			break;
		}
	}

	return incomingLight;
}

in vec2 coordinate;
out vec4 FragColor;

void main()
{
	uint seed = uint((coordinate.x + 1.0f) * 728816.0f + (coordinate.y + 1.0f) * 1927962376.0f);
	seed = seed + seed * frame * 8701;

	vec3 averageColor = vec3(0.0f);

	for (int s = 0; s < samplesPerPixel; s++)
	{
		vec2 randomBlurPoint = RandomPointInCircle(seed) * blur;
		vec2 randomFocalBlurPoint = RandomPointInCircle(seed) * focalBlur;

    	vec3 gridPoint = vec3(coordinate.x * focalDistance * perspectiveSlope, coordinate.y * focalDistance * perspectiveSlope * aspectRatio, focalDistance);
    	// Add random jitter to get a uniform blur, also usefull for anti-aliasing
		gridPoint += vec3(randomBlurPoint, 0.0f) * focalDistance;
		// Make a ray origin with random jitter to simulate focal blur
		vec3 rayOrigin = vec3(randomFocalBlurPoint, 0.0f);
		vec3 rayNormal = normalize(gridPoint - vec3(randomFocalBlurPoint, 0.0f));

		// Transform the ray from its local transform into world space
		rayNormal = (cameraRotation * vec4(rayNormal, 0.0f)).xyz;
		rayOrigin = (cameraRotation * vec4(rayOrigin, 0.0f)).xyz + cameraPosition;

		Ray ray;
		ray.origin = rayOrigin;
		ray.normal = rayNormal;
		ray.surfaceNormalDot = 0.0f;

		averageColor += Trace(ray, seed);
	}

    FragColor = vec4(averageColor / float(samplesPerPixel), 1.0f);
}