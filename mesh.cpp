#include <fstream>
#include <sstream>
#include <cassert>

#include "mesh.hpp"

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
				s >> c;
				std::string tokens[6];
				int token_n = -1;

				while (!s.eof())
				{
					char c = s.get();
					if (c == ' ' || c == '/') token_n++;
					else tokens[token_n].append(1, c);
				}

				tokens[token_n].pop_back();

				triangle t = {
					verts[std::stoi(tokens[0]) - 1],
					verts[std::stoi(tokens[2]) - 1],
					verts[std::stoi(tokens[4]) - 1],
					textures[std::stoi(tokens[1]) - 1],
					textures[std::stoi(tokens[3]) - 1],
					textures[std::stoi(tokens[5]) - 1]
				};

				ts.push_back(t);
			}
		}
	}
	return true;
}
