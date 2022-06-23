#include <fstream>
#include <sstream>

#include "mesh.hpp"

bool mesh::load_from_file(const char *path)
{
	std::ifstream f(path);
	if (!f.is_open()) return false;

	std::vector<vec3> verts;
	while (!f.eof())
	{
		char buf[256];
		f.getline(buf, 256);

		std::stringstream s;
		s << buf;

		char c;
		if (buf[0] == 'v')
		{
			vec3 v;
			s >> c >> v.x >> v.y >> v.z;
			verts.push_back(v);
		}
		else if (buf[0] == 'f')
		{
			int face[3];
			s >> c >> face[0] >> face[1] >> face[2];
			ts.push_back({verts[face[0] - 1], verts[face[1] - 1], verts[face[2] - 1]});
		}
	}
	return true;
}
