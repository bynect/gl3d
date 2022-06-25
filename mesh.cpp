#include <cctype>
#include <fstream>
#include <sstream>
#include <cassert>

#include "mesh.hpp"
#include "triangle.hpp"

bool mesh::load_from_file(const char *path, bool with_texture)
{
	std::ifstream f(path);
	if (!f.is_open()) return false;

	std::vector<vec3> verts;
	std::vector<vec2> textures;

	while (!f.eof())
	{
		char buf[256];
		f.getline(buf, 256);

		std::stringstream s;
		s << buf;

		char c;
		if (buf[0] == 'v')
		{
			if (buf[1] == 't')
			{
				assert(with_texture);
				vec2 t;
				s >> c >> c >> t.u >> t.v;
				textures.push_back(t);
			}
			else
			{
				vec3 v;
				s >> c >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}
		}
		else if (buf[0] == 'f')
		{
			if (!with_texture)
			{
				int face[3];
				s >> c >> face[0] >> face[1] >> face[2];
				ts.push_back({verts[face[0] - 1], verts[face[1] - 1], verts[face[2] - 1]});
			}
			else
			{
				int face_v[3] = {};
				int face_vn = 0;

				int face_t[3] = {};
				int face_tn = 0;

				// Currently ignored
				int normal;

				enum {
					PARSE_START,
					PARSE_V,
					PARSE_T,
					PARSE_N,
				} state = PARSE_START;

				bool done = false;
				while (!s.eof() && !done)
				{
					char c = s.get();

					switch (state)
					{
						case PARSE_START:
							assert(c == 'f');
							c = s.get();
							assert(c == ' ');
							state = PARSE_V;
							break;

						case PARSE_V:
							assert(std::isdigit(c));
							s.seekg((int)s.tellg() - 1);
							s >> face_v[face_vn++];

							c = s.get();
							assert(c == '/');
							state = PARSE_T;
							break;

						case PARSE_T:
							if (c == '/') state = PARSE_N;
							else
							{
								assert(std::isdigit(c));
								s.seekg((int)s.tellg() - 1);
								s >> face_t[face_tn++];

								if (face_tn == 3)
								{
									done = true;
									break;
								}

								c = s.get();
								assert(c == '/' || c == ' ');
								state = c == ' ' ? PARSE_V : PARSE_N;
							}
							break;

						case PARSE_N:
							assert(std::isdigit(c));
							s.seekg((int)s.tellg() - 1);
							s >> normal;

							if (face_vn == 3)
							{
								done = true;
								break;
							}

							c = s.get();
							assert(c == ' ');
							state = PARSE_V;
							break;

						default:
							assert(false && "Unreachable");
					}
				}

				assert(face_tn == 3 && face_tn == 3);
				ts.push_back({
					verts[face_v[0] - 1], verts[face_v[1] - 1], verts[face_v[2] - 1],
					textures[face_t[0] - 1], textures[face_t[1] - 1], textures[face_t[2] - 1],
				});
			}
		}
	}
	return true;
}
