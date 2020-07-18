#include "Mesh.h"
#include <time.h>

Mesh::Mesh(std::vector<Texture> textures, GLuint baseVertex, GLuint baseIndex)
{
	this->textures = textures;
}

// Render the mesh
void Mesh::draw(GLuint count)
{
	// Draw mesh
	glBindVertexArray(this->VAO);
	// glDrawElementsInstanced(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0, count);
	glBindVertexArray(0);

	// Always good practice to set everything back to defaults once configured.
	/*for (GLuint i = 0; i < this->textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}*/
}