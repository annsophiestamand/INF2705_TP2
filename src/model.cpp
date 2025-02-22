#include "model.h"
#include "obj_loader.h"

Model::Model(const char* path):
m_vbo(),
m_ebo(),
m_vao(),
m_drawcall(m_vao, 0, GL_UNSIGNED_INT) 
{
	// TODO
	// Cette fois-ci, la méthode BufferObject::allocate est publique (et devrait être utilisé ici)
	std::vector<GLfloat> vertexData;
	std::vector<GLuint> indices;
	loadObj(path,vertexData,indices);
	m_drawcall.setCount(indices.size());
	m_vbo.allocate(GL_ARRAY_BUFFER, sizeof(GLfloat)*vertexData.size(), vertexData.data(), GL_STATIC_DRAW);
	m_ebo.allocate(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*indices.size(), indices.data(), GL_STATIC_DRAW);
	m_vao.bind();
	m_ebo.bind(); 
	m_vao.specifyAttribute(m_vbo, 0, 3, 5, 0);
	m_vao.specifyAttribute(m_vbo, 1, 2, 5, 3);
	m_vao.unbind();
}	

void Model::loadObj(const char* path, std::vector<GLfloat>& vertexData, std::vector<GLuint>& indices)
{
	objl::Loader loader;
	bool loadout = loader.LoadFile(path);
	if (!loadout)
	{
		std::cout << "Unable to load model " << path << std::endl;
		return;
	}

	for (size_t i = 0; i < loader.LoadedVertices.size(); i++)
	{
		objl::Vector3 p = loader.LoadedVertices[i].Position;
		vertexData.push_back(p.X);
		vertexData.push_back(p.Y);
		vertexData.push_back(p.Z);
		objl::Vector2 t = loader.LoadedVertices[i].TextureCoordinate;
		vertexData.push_back(t.X);
		vertexData.push_back(t.Y);
	}
	indices = loader.LoadedIndices;
}

void Model::draw()
{
	// TODO 
	m_drawcall.draw();
	

}

