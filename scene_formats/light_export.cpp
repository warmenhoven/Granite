/* Copyright (c) 2017 Hans-Kristian Arntzen
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define RAPIDJSON_ASSERT(x) do { if (!(x)) throw "JSON error"; } while(0)

#include "light_export.hpp"
#include "scene.hpp"
#include "lights/lights.hpp"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
using namespace rapidjson;

namespace Granite
{
std::string export_lights_to_json(const DirectionalParameters &dir, Scene &scene)
{
	Document doc;
	doc.SetObject();
	auto &allocator = doc.GetAllocator();

	Value directional(kObjectType);

	Value direction(kArrayType);
	direction.PushBack(dir.direction.x, allocator);
	direction.PushBack(dir.direction.y, allocator);
	direction.PushBack(dir.direction.z, allocator);
	directional.AddMember("direction", direction, allocator);

	Value color(kArrayType);
	color.PushBack(dir.color.r, allocator);
	color.PushBack(dir.color.g, allocator);
	color.PushBack(dir.color.b, allocator);
	directional.AddMember("color", color, allocator);
	doc.AddMember("directional", directional, allocator);

	Value spots(kArrayType);
	Value points(kArrayType);

	scene.update_cached_transforms();
	auto &pos = scene.get_entity_pool().get_component_group<PositionalLightComponent, CachedSpatialTransformComponent>();

	for (auto &light : pos)
	{
		auto *l = std::get<0>(light)->light;
		auto *t = std::get<1>(light)->transform;

		Value pos(kArrayType);
		pos.PushBack(t->world_transform[3].x, allocator);
		pos.PushBack(t->world_transform[3].y, allocator);
		pos.PushBack(t->world_transform[3].z, allocator);

		Value dir(kArrayType);
		dir.PushBack(t->world_transform[2].x, allocator);
		dir.PushBack(t->world_transform[2].y, allocator);
		dir.PushBack(t->world_transform[2].z, allocator);

		if (l->get_type() == PositionalLight::Type::Spot)
		{
			Value spot(kObjectType);
			auto &s = *static_cast<SpotLight *>(l);
			spot.AddMember("innerCone", s.get_inner_cone(), allocator);
			spot.AddMember("outerCone", s.get_outer_cone(), allocator);
			Value color(kArrayType);
			color.PushBack(s.get_color().r, allocator);
			color.PushBack(s.get_color().g, allocator);
			color.PushBack(s.get_color().b, allocator);
			spot.AddMember("color", color, allocator);
			spot.AddMember("constantFalloff", s.get_constant_falloff(), allocator);
			spot.AddMember("linearFalloff", s.get_linear_falloff(), allocator);
			spot.AddMember("quadraticFalloff", s.get_quadratic_falloff(), allocator);
			spot.AddMember("position", pos, allocator);
			spot.AddMember("direction", dir, allocator);
			spots.PushBack(spot, allocator);
		}
		else
		{
			Value point(kObjectType);
			auto &p = *static_cast<PointLight *>(l);
			Value color(kArrayType);
			color.PushBack(p.get_color().r, allocator);
			color.PushBack(p.get_color().g, allocator);
			color.PushBack(p.get_color().b, allocator);
			point.AddMember("color", color, allocator);
			point.AddMember("constantFalloff", p.get_constant_falloff(), allocator);
			point.AddMember("linearFalloff", p.get_linear_falloff(), allocator);
			point.AddMember("quadraticFalloff", p.get_quadratic_falloff(), allocator);
			point.AddMember("position", pos, allocator);
			points.PushBack(point, allocator);
		}
	}

	doc.AddMember("spot", spots, allocator);
	doc.AddMember("point", points, allocator);

	StringBuffer buffer;
	PrettyWriter<StringBuffer> writer(buffer);
	//Writer<StringBuffer> writer(buffer);
	doc.Accept(writer);
	return buffer.GetString();
}
}