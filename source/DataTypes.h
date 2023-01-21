#pragma once

using namespace dae;

#include "Math.h"
#include "vector"
#include "Texture.h"


struct Vertex_In
{
	Vector3 position{};
	ColorRGB color{colors::White};
	Vector2 uv{};
	Vector3 normal{};
	Vector3 tangent{};
	Vector3 viewDirection{};

	bool operator==(Vertex_In other) {
		return other.position == position && other.color == color;
	}
};

struct Vertex_Out
{
	Vector4 position{};
	ColorRGB color{ colors::White };
	Vector2 uv{};
	Vector3 normal{};
	Vector3 tangent{};
	Vector3 viewDirection{};

	bool operator==(Vertex_Out other) {
		return other.position == position && other.color == color;
	}
};

enum class PrimitiveTopology
{
	TriangleList,
	TriangleStrip
};

struct Mesh {
	Mesh(std::vector<Vertex_In>& verticesIn, std::vector<uint32_t>& indicesIn, Vector3& translation, Vector3& scale,
		float yawRotation, std::vector<Texture*> pTexturesIn) {
		vertices = verticesIn;
		indices = indicesIn;
		Translate(translation);
		Scale(scale);
		RotateY(yawRotation);
		pTextures = pTexturesIn;
	}

	~Mesh() {
		//Delete all the textures
		for (auto pTexture : pTextures) {
			delete pTexture;
			pTexture = nullptr;
		}
	}

	std::vector<Vertex_In> vertices{};
	std::vector<uint32_t> indices{};

	dae::Matrix worldMatrix{};

	dae::Matrix translationTransform{};
	dae::Matrix scaleTransform{};
	dae::Matrix rotationTransform{};
	float totalYaw{};

	//Diffuse, Normal, Specular, Glossiness
	std::vector<dae::Texture*> pTextures{};

	void Translate(const dae::Vector3& translation) {
		translationTransform = Matrix::CreateTranslation(translation);
		UpdateWorldMatrix();
	}
	void RotateY(float yaw) {
		totalYaw += yaw;
		rotationTransform = Matrix::CreateRotationY(totalYaw);
		UpdateWorldMatrix();
	}
	void Scale(const dae::Vector3& scale) {
		scaleTransform = Matrix::CreateScale(scale);
		UpdateWorldMatrix();
	}

	void UpdateWorldMatrix() {
		worldMatrix = scaleTransform * rotationTransform * translationTransform;
	}
};

struct Mesh_Software 
{
	Mesh_Software(Mesh* pMesh, PrimitiveTopology topology)
	{
		primitiveTopology = topology;
		internalMesh = pMesh;
	}
	
	Mesh* internalMesh;
	PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleList };
	std::vector<Vertex_Out> vertices_out{};
};

enum class LightType
{
	Point,
	Directional
};

struct Light
{
	Light(Vector3 originIn, Vector3 directionIn, ColorRGB colorIn, float intensityIn, LightType typeIn) {
		origin = originIn;
		direction = directionIn;
		color = colorIn;
		intensity = intensityIn;
		type = typeIn;
	}

	Vector3 origin{};
	Vector3 direction{};
	ColorRGB color{};
	float intensity{};

	LightType type{};
};
