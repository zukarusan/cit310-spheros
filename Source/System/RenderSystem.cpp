#include "RenderSystem.hpp"

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <iostream>
#include <glm/glm.hpp>
#include <cmath>

#include "../Component/TransformComponent.hpp"
#include "../Component/SphereComponent.hpp"
#include "../Core/Camera.hpp"
#include "../Util/Logger.hpp"

#define PI 3.14159265358979323846f
#define RAD 0.0174533f

RenderSystem::RenderSystem() :
	m_MatTransform(1.0f)
{
	ResetMatrix();
}

// need refactor
void RenderSystem::ResetMatrix(float vanishing_point_z)
{
    const float scaleX = 1.0f;
    const float scaleY = 1.0f;
    const float translateX = 300.0f;
    const float translateY = 300.0f;

    // Perspective Projection
    // St
    m_MatTransform = glm::mat4(
            glm::vec4(scaleX, 0.0f, 0.0f, 0.0f),
            glm::vec4(0.0f, -scaleY, 0.0f, 0.0f),
            glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
            glm::vec4(translateX, translateY, 0.0f, 1.0f)
    );

    // Vt
    m_MatTransform *= glm::mat4(
            glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
            glm::vec4(0.0f, 0.0f, 0.0f, 1.0f / vanishing_point_z),
            glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
}

void RenderSystem::ResetTransform(entt::registry& registry)
{
    ResetMatrix();
    registry.view<SphereComponent, TransformComponent>().each([&](auto entity, SphereComponent& sphere, TransformComponent& transform)
      {
            transform.ResetMatrix();
      });
}

bool once = true;

void RenderSystem::Render(entt::registry& registry, sf::RenderWindow& window)
{
	registry.view<SphereComponent, TransformComponent>().each([&](auto entity, SphereComponent& sphere, const TransformComponent& transform)
		{
			// Draw the vertices
			/*
			auto circle = sf::CircleShape(2.0f);
			circle.setFillColor(sf::Color::Red);
			for (glm::vec4& vertex : m_EntitySphere.m_Vertices)
			{
				auto s = Normalize3DToProjectionSFML(vertex, transform);
				circle.setPosition(s);
				window.draw(circle);
			}
			*/

			// TODO
			// Parallelize the code?
			// https://en.cppreference.com/w/cpp/algorithm/execution_policy_tag_t
			// https://docs.microsoft.com/en-us/cpp/parallel/concrt/how-to-write-a-parallel-for-loop?view=vs-2019

			// Toggle backface cull
			if (sphere.m_IsCullBackface)
			{
				// In case you want to use surface
				//sf::ConvexShape convex(3);
				//convex.setFillColor(sf::Color::Green);
				//convex.setOutlineColor(sf::Color::Green);
				//convex.setOutlineThickness(1.0f);

				// Bottom part
				for (size_t i = 0; i < sphere.m_NLongitude - 1; ++i)
				{
					const glm::vec3 v1 = Normalize3DToProjection3D(sphere.m_Vertices[i + 1], transform) - Normalize3DToProjection3D(sphere.m_Vertices[0], transform);
					const glm::vec3 v2 = Normalize3DToProjection3D(sphere.m_Vertices[i + 2], transform) - Normalize3DToProjection3D(sphere.m_Vertices[i + 1], transform);
					if (glm::dot(glm::cross(v1, v2), m_Camera.m_Direction) > 0.0f)
					{
						sf::Vertex line[] =
						{
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[0], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[i + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[i + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[i + 2], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[i + 2], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[0], transform))
						};
						window.draw(line, 6, sf::Lines);

						// In case you want to use surface
						//convex.setPoint(0, Normalize3DToProjectionSFML(m_EntitySphere.m_Vertices[0], transform));
						//convex.setPoint(1, Normalize3DToProjectionSFML(m_EntitySphere.m_Vertices[i + 1], transform));
						//convex.setPoint(2, Normalize3DToProjectionSFML(m_EntitySphere.m_Vertices[i + 2], transform));
						//window.draw(convex);
					}
				}


				const glm::vec3 v1 = Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude], transform) - Normalize3DToProjection3D(sphere.m_Vertices[0], transform);
				const glm::vec3 v2 = Normalize3DToProjection3D(sphere.m_Vertices[1], transform) - Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude], transform);
				if (glm::dot(glm::cross(v1, v2), m_Camera.m_Direction) > 0.0f)
				{
					sf::Vertex line[] =
					{
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[0], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[0], transform))
					};
					window.draw(line, 6, sf::Lines);
				}

				// Middle part
				for (size_t i = 1; i < sphere.m_NLatitude * 2 - 1; ++i)
				{
					for (size_t j = 0; j < sphere.m_NLongitude - 1; ++j)
					{
						const glm::vec3 v1 = Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * i + j + 2], transform) - Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * i + j + 1], transform);
						const glm::vec3 v2 = Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform) - Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * i + j + 2], transform);
						if (glm::dot(glm::cross(v1, v2), m_Camera.m_Direction) > 0.0f)
						{
							sf::Vertex line[] =
							{
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 1], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 2], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 2], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 1], transform)),
							};
							window.draw(line, 6, sf::Lines);
						}

						const glm::vec3 v3 = Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * i + j + 2], transform) - Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform);
						const glm::vec3 v4 = Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 2], transform) - Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * i + j + 2], transform);
						if (glm::dot(glm::cross(v3, v4), m_Camera.m_Direction) > 0.0f)
						{
							sf::Vertex line[] =
							{
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 2], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 2], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 2], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 2], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform))
							};
							window.draw(line, 6, sf::Lines);
						}
					}

					const size_t j = sphere.m_NLongitude - 1;
					const glm::vec3 v1 = Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * i + 1], transform) - Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * i + j + 1], transform);
					const glm::vec3 v2 = Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform) - Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * i + 1], transform);
					if (glm::dot(glm::cross(v1, v2), m_Camera.m_Direction) > 0.0f)
					{
						sf::Vertex line[] =
						{
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 1], transform))
						};
						window.draw(line, 6, sf::Lines);
					}

					const glm::vec3 v3 = Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * i + 1], transform) - Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform);
					const glm::vec3 v4 = Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + 1], transform) - Normalize3DToProjection3D(sphere.m_Vertices[sphere.m_NLongitude * i + 1], transform);
					if (glm::dot(glm::cross(v3, v4), m_Camera.m_Direction) > 0.0f)
					{
						sf::Vertex line[] =
						{
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + 1], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + 1], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + 1], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + 1], transform)),
								sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform))
						};
						window.draw(line, 6, sf::Lines);
					}
				}

				// Top part
				const size_t topMostVertex = sphere.m_NLongitude * (sphere.m_NLatitude * 2 - 1) + 2 - 1;
				for (size_t i = 0; i < sphere.m_NLongitude - 1; ++i)
				{
					const glm::vec3 v1 = Normalize3DToProjection3D(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude + i + 1], transform) - Normalize3DToProjection3D(sphere.m_Vertices[topMostVertex], transform);
					const glm::vec3 v2 = Normalize3DToProjection3D(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude + i], transform) - Normalize3DToProjection3D(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude + i + 1], transform);
					if (glm::dot(glm::cross(v1, v2), m_Camera.m_Direction) > 0.0f)
					{
						sf::Vertex line[] =
						{
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude + i + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude + i + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude + i], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude + i], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex], transform))
						};
						window.draw(line, 6, sf::Lines);
					}
				}

				const glm::vec3 v3 = Normalize3DToProjection3D(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude], transform) - Normalize3DToProjection3D(sphere.m_Vertices[topMostVertex], transform);
				const glm::vec3 v4 = Normalize3DToProjection3D(sphere.m_Vertices[topMostVertex - 1], transform) - Normalize3DToProjection3D(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude], transform);
				if (glm::dot(glm::cross(v3, v4), m_Camera.m_Direction) > 0.0f)
				{
					sf::Vertex line[] =
					{
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex], transform))
					};
					window.draw(line, 6, sf::Lines);
				}
			}
			else
			{
				// Bottom part
				for (size_t i = 0; i < sphere.m_NLongitude - 1; ++i)
				{
					sf::Vertex line[] =
					{
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[i + 2], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[i + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[i + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[0], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[0], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[i + 2], transform))
					};
					window.draw(line, 6, sf::Lines);

					// In case you want to use surface
					//convex.setPoint(0, Normalize3DToProjection(m_EntitySphere.m_Vertices[0], transform));
					//convex.setPoint(1, Normalize3DToProjection(m_EntitySphere.m_Vertices[i + 1], transform));
					//convex.setPoint(2, Normalize3DToProjection(m_EntitySphere.m_Vertices[i + 2], transform));
					//window.draw(convex);
				}

				sf::Vertex line[] =
				{
					sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[1], transform)),
					sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude], transform)),
					sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude], transform)),
					sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[0], transform)),
					sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[0], transform)),
					sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[1], transform))
				};
				window.draw(line, 6, sf::Lines);

				// Middle part
				for (size_t i = 1; i < sphere.m_NLatitude * 2 - 1; ++i)
				{
					for (size_t j = 0; j < sphere.m_NLongitude - 1; ++j)
					{
						sf::Vertex line[] =
						{
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 2], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 2], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),

							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 2], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 2], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 2], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),
							sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 2], transform))
						};
						window.draw(line, 12, sf::Lines);
					}

					const size_t j = sphere.m_NLongitude - 1;
					sf::Vertex line[] =
					{
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + j + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),

						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * i + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + j + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[sphere.m_NLongitude * (i - 1) + 1], transform))
					};
					window.draw(line, 12, sf::Lines);
				}

				// Top part
				const size_t topMostVertex = sphere.m_NLongitude * (sphere.m_NLatitude * 2 - 1) + 2 - 1;
				for (size_t i = 0; i < sphere.m_NLongitude - 1; ++i)
				{
					sf::Vertex line[] =
					{
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude + i], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude + i + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude + i + 1], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex], transform)),
						sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude + i], transform))
					};
					window.draw(line, 6, sf::Lines);
				}

				sf::Vertex line2[] =
				{
					sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex], transform)),
					sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude + 1], transform)),
					sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - sphere.m_NLongitude + 1], transform)),
					sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - 1], transform)),
					sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex - 1], transform)),
					sf::Vertex(Normalize3DToProjectionSFML(sphere.m_Vertices[topMostVertex], transform))
				};
				window.draw(line, 6, sf::Lines);


				/*auto circle = sf::CircleShape(2.0f);
				circle.setFillColor(sf::Color::Red);
				for (glm::vec4& vertex : sphere.m_Vertices)
				{
					auto s = Normalize3DToProjectionSFML(vertex, transform);
					circle.setPosition(s);
					window.draw(circle);
				}*/
			}
		});
}

inline sf::Vector2f RenderSystem::Normalize3DToProjectionSFML(const glm::vec4& v, const TransformComponent& transform)
{
	const glm::vec4& result = Normalize3DToProjection(v, transform);
	return sf::Vector2f(result.x / result.w, result.y / result.w);
}

// I know it is a bad name
inline glm::vec4 RenderSystem::Normalize3DToProjection3D(const glm::vec4& v, const TransformComponent& transform)
{
	const glm::vec4& result = Normalize3DToProjection(v, transform);
	return glm::vec4(result.x / result.w, result.y / result.w, result.z / result.w, 1.0f);
}

inline glm::vec4 RenderSystem::Normalize3DToProjection(const glm::vec4& v, const TransformComponent& transform)
{
	return m_MatTransform * transform.m_MatTransform * v;
}
