#include "pch.h"

//Project includes
#include "Renderer_Software.h"
#include "Math.h"
#include "Matrix.h"
#include "Utils.h"
#include "Timer.h"
#include <iostream>
#include <ppl.h> //parallel_for

using namespace dae;

Renderer_Software::Renderer_Software(SDL_Window* pWindow, dae::Camera* pCamera, std::vector<Mesh*>& pMeshes) :
	Renderer(pWindow, pCamera, pMeshes)
{
	m_RendererColor = ColorRGB{ 0.39f, 0.39f, 0.39f}*255.f;
	m_UniformColor = ColorRGB{ 0.1f, 0.1f, 0.1f} * 255.f;
	
	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Lights
	m_pLights.push_back(new Light({0, 0, 0}, { 0.577f, -0.577f, 0.557f }, colors::White, 7.0f, LightType::Directional));

	//Initialize Meshes
	//Only vehicle is needed, this is the first mesh
	Mesh_Software* pVehicleMesh = new Mesh_Software(pMeshes[0], PrimitiveTopology::TriangleList);
	m_pSoftwareMeshes.push_back(pVehicleMesh);
}

Renderer_Software::~Renderer_Software()
{
	delete[] m_pDepthBufferPixels;
	m_pDepthBufferPixels = nullptr;

	//delete meshes and lights
	for (auto pMesh : m_pSoftwareMeshes) {
		//Mesh textures are deleted in the mesh itself
		delete pMesh;
		pMesh = nullptr;
	}
	for (auto pLight : m_pLights) {
		delete pLight;
		pLight = nullptr;
	}
}

void Renderer_Software::Update(const dae::Timer* pTimer)
{
	m_pCamera->Update(pTimer);
	if (m_CanRotate) {
		for (const auto pMesh : m_pMeshes) {
			pMesh->RotateY(PI_DIV_4 * pTimer->GetElapsed());
		}
	}
}

void Renderer_Software::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	Render_Meshes();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer_Software::ToggleDepthBuffer()
{
	m_RenderDepthBuffer = !m_RenderDepthBuffer;
}

void Renderer_Software::ToggleRotation()
{
	m_CanRotate = !m_CanRotate;
}

void Renderer_Software::ToggleNormalMap()
{
	m_CanUseNormalMap = !m_CanUseNormalMap;
}

bool Renderer_Software::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void Renderer_Software::Render_Meshes() {
	//Clear depth buffer and back buffer
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

	ColorRGB clearColor = m_RendererColor;
	if (m_ShouldUseUniformColor) {
		clearColor = m_UniformColor;
	}
	//SDL_MapRGB
	Uint32 clearColorUint = 0xFF000000 | (Uint32)clearColor.r | (Uint32)clearColor.g << 8 | (Uint32)clearColor.b << 16;
	SDL_FillRect(m_pBackBuffer, NULL, clearColorUint);

	//Vertices in NDC space
	MeshVertexTransformationFunction(m_pSoftwareMeshes);
	//go over all the meshes
	for (const auto pSoftwareMesh : m_pSoftwareMeshes) {
		Mesh* pMesh = pSoftwareMesh->internalMesh;
		if (pSoftwareMesh->primitiveTopology == PrimitiveTopology::TriangleList) {
			int size = static_cast<int>(pMesh->indices.size());
			uint32_t amountOfTriangles = size / 3;

			//go over all triangles
			concurrency::parallel_for(0u, amountOfTriangles, [=, this](int i) 
				{
			Vertex_Out vertex1{ pSoftwareMesh->vertices_out[pMesh->indices[3 * i]] };
			Vertex_Out vertex2{ pSoftwareMesh->vertices_out[pMesh->indices[3 * i + 1]] };
			Vertex_Out vertex3{ pSoftwareMesh->vertices_out[pMesh->indices[3 * i + 2]] };

			//Frustrum culling for x and y
			if (vertex1.position.x < -1 || vertex1.position.x > 1 ||
				vertex1.position.y < -1 || vertex1.position.y > 1) {
				return;
			}
			if (vertex2.position.x < -1 || vertex2.position.x > 1 ||
				vertex2.position.y < -1 || vertex2.position.y > 1) {
				return;
			}
			if (vertex3.position.x < -1 || vertex3.position.x > 1 ||
				vertex3.position.y < -1 || vertex3.position.y > 1) {
				return;
			}

			//Vertices from NDC space to raster space
			vertex1.position.x = (vertex1.position.x + 1) / 2.f * static_cast<float>(m_Width);
			vertex1.position.y = (1 - vertex1.position.y) / 2.f * static_cast<float>(m_Height);

			vertex2.position.x = (vertex2.position.x + 1) / 2.f * static_cast<float>(m_Width);
			vertex2.position.y = (1 - vertex2.position.y) / 2.f * static_cast<float>(m_Height);

			vertex3.position.x = (vertex3.position.x + 1) / 2.f * static_cast<float>(m_Width);
			vertex3.position.y = (1 - vertex3.position.y) / 2.f * static_cast<float>(m_Height);

			RenderTriangle(vertex1, vertex2, vertex3, pSoftwareMesh);
				}
			);
		}
		if (pSoftwareMesh->primitiveTopology == PrimitiveTopology::TriangleStrip) {
			//go over all the indices one by one, every pair of 3 will be a triangle
			for (int i{}; i <= pMesh->indices.size() - 3; ++i) {
				Vertex_Out vertex1{ pSoftwareMesh->vertices_out[pMesh->indices[i]] };
				Vertex_Out vertex2{ pSoftwareMesh->vertices_out[pMesh->indices[i + 1]] };
				Vertex_Out vertex3{ pSoftwareMesh->vertices_out[pMesh->indices[i + 2]] };
				//if uneven, switch the last two vertices
				if (i % 2 == 1) {
					Vertex_Out tempVertex{ vertex2 };
					vertex2 = vertex3;
					vertex3 = tempVertex;
				}
				//if two of the vertices are the same continue, because those are not Triangles (no area)
				if (vertex1 == vertex2 || vertex1 == vertex3 || vertex2 == vertex3) {
					continue;
				}

				//Frustrum culling for x and y
				if (vertex1.position.x < -1 || vertex1.position.x > 1 ||
					vertex1.position.y < -1 || vertex1.position.y > 1) {
					continue;
				}
				if (vertex2.position.x < -1 || vertex2.position.x > 1 ||
					vertex2.position.y < -1 || vertex2.position.y > 1) {
					continue;
				}
				if (vertex3.position.x < -1 || vertex3.position.x > 1 ||
					vertex3.position.y < -1 || vertex3.position.y > 1) {
					continue;
				}

				//Vertices from NDC space to raster space
				vertex1.position.x = (vertex1.position.x + 1) / 2.f * static_cast<float>(m_Width);
				vertex1.position.y = (1 - vertex1.position.y) / 2.f * static_cast<float>(m_Height);

				vertex2.position.x = (vertex2.position.x + 1) / 2.f * static_cast<float>(m_Width);
				vertex2.position.y = (1 - vertex2.position.y) / 2.f * static_cast<float>(m_Height);

				vertex3.position.x = (vertex3.position.x + 1) / 2.f * static_cast<float>(m_Width);
				vertex3.position.y = (1 - vertex3.position.y) / 2.f * static_cast<float>(m_Height);

				RenderTriangle(vertex1, vertex2, vertex3, pSoftwareMesh);
			}
		}
	}
}

void Renderer_Software::RenderTriangle(const Vertex_Out& vertex1, const Vertex_Out& vertex2, const Vertex_Out& vertex3, Mesh_Software* pMesh) {
	Vector2 v0{ vertex1.position.x, vertex1.position.y };
	Vector2 v1{ vertex2.position.x, vertex2.position.y };
	Vector2 v2{ vertex3.position.x, vertex3.position.y };

	//Bounding box
	Int2 min{}, max{};

	min.x = Clamp(int(std::min(vertex1.position.x, std::min(vertex2.position.x, vertex3.position.x))), 0, m_Width);
	min.y = Clamp(int(std::min(vertex1.position.y, std::min(vertex2.position.y, vertex3.position.y))), 0, m_Height);

	max.x = Clamp(int(std::max(vertex1.position.x, std::max(vertex2.position.x, vertex3.position.x))), 0, m_Width);
	max.y = Clamp(int(std::max(vertex1.position.y, std::max(vertex2.position.y, vertex3.position.y))), 0, m_Height);

	//RENDER LOGIC
	for (int px{ min.x }; px <= max.x; ++px)
	{
		for (int py{ min.y }; py <= max.y; ++py)
		{
			if (m_CanRenderBoundingBox)
			{
				//White bounding box
				ColorRGB finalColor{ 1,1,1 };

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));

				continue;
			}
			
			const Vector2 pixelPoint{ static_cast<float>(px), static_cast<float>(py) };

			Vector2 pointToSide = pixelPoint - v0;
			Vector2 a = v1 - v0;
			float w2 = Vector2::Cross(a, pointToSide);

			pointToSide = pixelPoint - v1;
			Vector2 b = v2 - v1;
			float w0 = Vector2::Cross(b, pointToSide);

			pointToSide = pixelPoint - v2;
			Vector2 c = v0 - v2;
			float w1 = Vector2::Cross(c, pointToSide);

			bool pointInTriangle{};

			switch (m_CurrentCullmode) {
			case Cullmode::backFace:
				pointInTriangle = (w0 >= 0) && (w1 >= 0) && (w2 >= 0); //backface
				break;
			case Cullmode::frontFace:
				pointInTriangle = (w0 <= 0) && (w1 <= 0) && (w2 <= 0); //frontface
				break;
			case Cullmode::none:
				pointInTriangle = ((w0 >= 0) && (w1 >= 0) && (w2 >= 0) || (w0 <= 0) && (w1 <= 0) && (w2 <= 0)); //none
				break;
			}

			if (pointInTriangle) {
				//Barycentric coordinates
				const float totalArea = w0 + w1 + w2;
				w0 /= totalArea;
				w1 /= totalArea;
				w2 /= totalArea;
				//Check depth buffer
				//	Interpolate depth
				float interpolatedDepth{ 1.f / ((w0 / vertex1.position.z) + (w1 / vertex2.position.z) + (w2 / vertex3.position.z)) };
				//	Frustrum culling on z
				if (interpolatedDepth < 0 || interpolatedDepth > 1) {
					continue;
				}

				if (interpolatedDepth < m_pDepthBufferPixels[px + (py * m_Width)]) {
					m_pDepthBufferPixels[px + (py * m_Width)] = interpolatedDepth;

					//Need the interpolated depth with the actual depth, stored in w
					const float interpolatedDepthW{ 1.f / ((w0 / vertex1.position.w) + (w1 / vertex2.position.w) + (w2 / vertex3.position.w)) };
					//Interpolate Vertex
					//	Interpolate Position
					Vector4 interpolatedPos = (vertex1.position * (w0 / vertex1.position.w) + vertex2.position * (w1 / vertex2.position.w) + vertex3.position * (w2 / vertex3.position.w));
					interpolatedPos = interpolatedPos * interpolatedDepthW;
					//	Interpolate UV
					Vector2 interpolatedUV = (vertex1.uv * w0 / vertex1.position.w + vertex2.uv * w1 / vertex2.position.w + vertex3.uv * w2 / vertex3.position.w);
					interpolatedUV *= interpolatedDepthW;
					//	Interpolate Normal
					Vector3 interpolatedNormal = (vertex1.normal * w0 / vertex1.position.w + vertex2.normal * w1 / vertex2.position.w + vertex3.normal * w2 / vertex3.position.w);
					interpolatedNormal *= interpolatedDepthW;
					interpolatedNormal.Normalize();
					//	Interpolate Tangent
					Vector3 interpolatedTangent = (vertex1.tangent * w0 / vertex1.position.w + vertex2.tangent * w1 / vertex2.position.w + vertex3.tangent * w2 / vertex3.position.w);
					interpolatedTangent *= interpolatedDepthW;
					interpolatedTangent.Normalize();
					//	Interpolate ViewDirection
					Vector3 interpolatedView = (vertex1.viewDirection * w0 / vertex1.position.w + vertex2.viewDirection * w1 / vertex2.position.w + vertex3.viewDirection * w2 / vertex3.position.w);
					interpolatedView *= interpolatedDepthW;
					interpolatedView.Normalize();

					Vertex_Out currentVertex{ interpolatedPos, interpolatedUV, interpolatedNormal, interpolatedTangent, interpolatedView };
					ColorRGB finalColor{ PixelShading(currentVertex, pMesh) };

					//Update Color in Buffer
					finalColor.MaxToOne();

					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}
}

ColorRGB Renderer_Software::PixelShading(const Vertex_Out& vertex, Mesh_Software* pSoftwareMesh)
{
	if (m_RenderDepthBuffer) {
		float interpolatedDepth = Utils::Remap(vertex.position.z, 0.985f, 1.f);
		return ColorRGB{ interpolatedDepth, interpolatedDepth, interpolatedDepth };
	}
	Texture* pDiffuse{};
	Texture* pNormal{};
	Texture* pSpecular{};
	Texture* pGloss{};

	ColorRGB ambientColor{ 0.025f, 0.025f, 0.025f };
	ColorRGB finalColor{};
	float specularShininess{ 25.f };

	//Get the textures

	Mesh* pMesh = pSoftwareMesh->internalMesh;
	if (pMesh->pTextures.size() == 4) {
		pDiffuse = pMesh->pTextures[0];
		pNormal = pMesh->pTextures[1];
		pSpecular = pMesh->pTextures[2];
		pGloss = pMesh->pTextures[3];
	}
	else {
		//You did not get all the needed textures!
		return finalColor;
	}
	
	Vector3 binormal = Vector3::Cross(vertex.normal, vertex.tangent);
	binormal.Normalize();
	Matrix tangentSpaceAxis = Matrix{ vertex.tangent, binormal, vertex.normal, Vector3::Zero };

	for (auto pLight : m_pLights) {
		Vector3 lightDirection{ (pLight->direction) * -1 };
		//Clamp UV values
		if (vertex.uv.x < 0 || vertex.uv.x > 1
			|| vertex.uv.y < 0 || vertex.uv.y > 1) {
			continue;
		}

		Vector3 normal = vertex.normal;

		if (m_CanUseNormalMap) {
			//Sample normal from the normal map
			if (!pNormal) {
				//normal map is not set
				return finalColor;
			}
			ColorRGB normalColor = pNormal->Sample(vertex.uv);

			//Make it into a vector and bring it in a correct range
			Vector3 sampledNormal = { normalColor.r, normalColor.g, normalColor.b };
			sampledNormal = 2.f * sampledNormal - Vector3{ 1.f, 1.f, 1.f };
			sampledNormal = tangentSpaceAxis.TransformVector(sampledNormal);
			sampledNormal.Normalize();

			normal = sampledNormal;
		}

		const float observedArea = Vector3::Dot(normal, lightDirection);

		if (observedArea <= 0) {
			continue;
		}

		if (!pDiffuse || !pSpecular || !pGloss) {
			//textures not correctly set!
			return finalColor;
		}

		//Sample diffuse color from the texture
		ColorRGB diffuseColor = pDiffuse->Sample(vertex.uv);
		float kd{ 1.f };
		ColorRGB lambertDiffuse = diffuseColor * (kd / float(M_PI));

		//Sample specular color from the specular texture
		ColorRGB specularColor = pSpecular->Sample(vertex.uv);
		//Sample glossiness from the glossiness map
		ColorRGB glossColor = pGloss->Sample(vertex.uv);
		float glossExponent = glossColor.r;
		glossExponent *= specularShininess;

		//light direction towards the point
		const Vector3 r = -lightDirection - (2 * Vector3::Dot(normal, -lightDirection) * normal);
		float cosA = Vector3::Dot(r, vertex.viewDirection);
		cosA = std::max(cosA, 0.f);
		ColorRGB phongSpecular{ specularColor * powf(cosA, glossExponent) };

		ColorRGB radiance = LightUtils::GetRadiance(pLight, pLight->origin);

		switch (m_CurrentShadingMode)
		{
		case Renderer_Software::ShadingMode::ObservedArea:
			finalColor += {observedArea, observedArea, observedArea};
			break;
		case Renderer_Software::ShadingMode::Diffuse:
			finalColor += radiance * lambertDiffuse * observedArea;
			break;
		case Renderer_Software::ShadingMode::Specular:
			finalColor += phongSpecular * observedArea;
			break;
		case Renderer_Software::ShadingMode::Combined:
			finalColor += ColorRGB{
				(ambientColor + (radiance * lambertDiffuse) + phongSpecular) *
				observedArea };
			break;
		}
	}
	return finalColor;
}


void Renderer_Software::MeshVertexTransformationFunction(std::vector<Mesh_Software*>& pMeshes_in) const {
	for (auto pSoftwareMesh : pMeshes_in) {
		Mesh* pMesh = pSoftwareMesh->internalMesh;
		Matrix worldViewProjectionMatrix = pMesh->worldMatrix * m_pCamera->viewMatrix * m_pCamera->projectionMatrix;
		//clear the current vertices_out
		pSoftwareMesh->vertices_out.clear();
		for (const Vertex_In& vertex : pMesh->vertices) {
			//make sure w is initialised as 1
			Vector4 position{ vertex.position, 1 };

			Vector4 transformedVertexPos = worldViewProjectionMatrix.TransformPoint(position);

			//perspective divide
			transformedVertexPos.x /= transformedVertexPos.w;
			transformedVertexPos.y /= transformedVertexPos.w;
			transformedVertexPos.z /= transformedVertexPos.w;

			//transform the normals in world space and normalize again
			Vector3 transformedNormal = pMesh->worldMatrix.TransformVector(vertex.normal);
			transformedNormal.Normalize();
			Vector3 transformedTangent = pMesh->worldMatrix.TransformVector(vertex.tangent);
			transformedTangent.Normalize();

			//calculate the view direction
			Vector3 vertexWorldPosition = pMesh->worldMatrix.TransformPoint(vertex.position);
			Vector3 viewDirection{ m_pCamera->origin - vertexWorldPosition };

			Vertex_Out transformedVertex{ transformedVertexPos, vertex.uv, transformedNormal, transformedTangent, viewDirection };
			pSoftwareMesh->vertices_out.emplace_back(transformedVertex);
		}
	}
}

void Renderer_Software::CycleLightingMode() {
	switch (m_CurrentShadingMode)
	{
	case ShadingMode::Combined:
		m_CurrentShadingMode = ShadingMode::ObservedArea;
		std::cout << "Lighting mode set to ObservedArea" << std::endl;
		break;
	case ShadingMode::ObservedArea:
		m_CurrentShadingMode = ShadingMode::Diffuse;
		std::cout << "Lighting mode set to Diffuse" << std::endl;
		break;
	case ShadingMode::Diffuse:
		m_CurrentShadingMode = ShadingMode::Specular;
		std::cout << "Lighting mode set to Specular" << std::endl;
		break;
	case ShadingMode::Specular:
		m_CurrentShadingMode = ShadingMode::Combined;
		std::cout << "Lighting mode set to Combined" << std::endl;
		break;
	}
}

void Renderer_Software::ToggleBoundingBox()
{
	m_CanRenderBoundingBox = !m_CanRenderBoundingBox;
}

bool Renderer_Software::CanRotate()
{
	return m_CanRotate;
}
