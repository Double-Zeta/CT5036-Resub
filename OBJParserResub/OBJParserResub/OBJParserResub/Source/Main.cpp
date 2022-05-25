#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <map>
#include <sstream>

std::string g_filePath;

struct vec4
{
	float x, y, z, w;
};

struct vec2
{
	float u, v;
};

struct OBJVertex
{
	vec4 vertex;
	vec4 normal;
	vec2 uvCoord;
};


bool ProcessLine(const std::string& a_inLine, std::string& a_outKey, std::string& a_outValue)
{
	if (!a_inLine.empty())
	{
		size_t keyStart = a_inLine.find_first_not_of(" \t\r\n");
		if (keyStart == std::string::npos)
			return false;
		size_t keyEnd = a_inLine.find_first_of("\t\r\n", keyStart);
		size_t valueStart = a_inLine.find_first_not_of(" \t\r\n", keyEnd);
		size_t valueEnd = a_inLine.find_last_not_of(" \t\n\r") + 1;
		a_outKey = a_inLine.substr(keyStart, keyEnd - keyStart);
		if (valueStart == std::string::npos)
		{
			a_outValue = "";
			return true;
		}
		a_outValue = a_inLine.substr(valueStart, valueEnd - valueStart);
		return true;
	}
	return false;
}

vec4 processVectorString(const std::string a_data)
{
	//split line data at each space character and store as a float value within glm::vec4
	std::stringstream iss(a_data);
	//create zero vec4
	vec4 vecData = { 0.0f,0.0f,0.0f,0.0f };
	int i = 0;
	for (std::string val; iss >> val; ++i)
	{
		//use std::string to float function
		float fVal = std::stof(val);
		//cast vec4 to float* to allow iteration through vec4
		((float*)(&vecData))[i] = fVal;
	}
	return vecData;
}

std::vector<std::string> splitStringAtCharacter(std::string data, char a_character)
{
	std::vector<std::string> lineData;
	std::stringstream iss(data);
	std::string lineSegment;
	while (std::getline(iss, lineSegment, a_character))
	{
		//push each line segment into a vector
		lineData.push_back(lineSegment);
	}
	return lineData;
}

OBJVertex processFaceData(std::string a_faceData, std::vector<vec4>& a_vertexArray,
							std::vector<vec4>& a_normalArray, std::vector<vec2>& a_uvArray)
{
	std::vector<std::string> vertexIndices = splitStringAtCharacter(a_faceData, '/');
	typedef struct objFaceTriplet { int32_t v, vn, vt; }objFaceTriplet;
	objFaceTriplet ft = { 0,0,0 };
	ft.v = std::stoi(vertexIndices[0]);
	if (vertexIndices.size() >= 2) {
		if (vertexIndices[1].size() > 0)
		{
			ft.vt = std::stoi(vertexIndices[1]);
		}
		if (vertexIndices.size() >= 3)
		{
			ft.vn = std::stoi(vertexIndices[2]);
		}
	}
	//face index values have been processed get actual data from vertex arrays
	OBJVertex currentVertex;
	currentVertex.vertex = a_vertexArray[size_t(ft.v) - 1];
	if (ft.vn != 0)
	{
		currentVertex.normal = a_normalArray[size_t(ft.vn) - 1];
	}
	if (ft.vt != 0)
	{
		currentVertex.uvCoord = a_uvArray[size_t(ft.vt) - 1];
	}
	return currentVertex;

}



int main(int argc, char* argv[])
{
	std::string filename = "obj_models/basic_box.obj";
	std::cout << "Attempting to open file: " << filename << std::endl;
	//use fstream to read file data
	std::fstream file;
	file.open(filename, std::ios_base::in | std::ios_base::binary);

	
	if (file.is_open())
	{
		std::cout << "Successfully Opened!" << std::endl;
		file.ignore(std::numeric_limits<std::streamsize>::max());
		std::streamsize fileSize = file.gcount();
		file.clear();
		file.seekg(0, std::ios_base::beg);
		if (fileSize != 0)
		{
			std::cout << std::fixed;
			std::cout << std::setprecision(2);
			std::cout << "File Size: " << fileSize / (float)1024 << "KB" << std::endl;
			std::string fileLine;

			//while the end of file token (EOF) has not been read
			std::map< std::string, int32_t > faceIndexMap;
			std::vector<vec4> vertexData;
			std::vector<vec4> normalData;
			std::vector<OBJVertex> meshData;
			std::vector<uint32_t> meshIndices;
			std::vector<uint32_t> faceIndices;
			//std::vector<OBJMaterial> materials;
			std::vector<vec2> textureData;

			while (!file.eof())
			{
				if (std::getline(file, fileLine))
				{
					//file has been read
					std::string key;
					std::string value;
					if (ProcessLine(fileLine, key, value))
					{
						if (key == "#")
						{
							std::cout << value << std::endl;
						}
						if (key == "v")
						{
							vec4 vertex = processVectorString(value);
							vertex.w = 1.f;
							vertexData.push_back(vertex);
						}
						if (key == "vn")
						{
							vec4 normal = processVectorString(value);
							normal.w = 0.f;
							normalData.push_back(normal);
						}
						if (key == "vt")
						{
							vec4 vec = processVectorString(value);
							vec2 uvCoord = { vec.x,vec.y };
							textureData.push_back(uvCoord);
						}
						if (key == "f")
						{
							std::vector<std::string> faceComponents = splitStringAtCharacter(value, ' ');
							for (auto iter = faceComponents.begin(); iter != faceComponents.end(); ++iter)
							{
								auto searchKey = faceIndexMap.find(*iter);
								if (searchKey != faceIndexMap.end())
								{
									std::cout << "Processing repeat face data: " << (*iter) << std::endl;
								}
								else
								{
									OBJVertex vertex = processFaceData(*iter, vertexData, normalData, textureData);
									meshData.push_back(vertex);
									uint32_t index = ((uint32_t)meshData.size() - 1);
									faceIndexMap[*iter] = index;
									faceIndices.push_back(index);
								}
								for (int i = 1; i < faceIndices.size() - 1; i++)
								{
									meshIndices.push_back(faceIndices[0]);
									meshIndices.push_back(faceIndices[0]);
									meshIndices.push_back(faceIndices[(size_t)i + 1]);
								}
								

							}
						}
						
					}
				}
			}
			vertexData.clear();
			normalData.clear();
			textureData.clear();
			std::cout << "Processed " << vertexData.size() << "vertices in OBJ File" << std::endl;
		}
		else
		{
			std::cout << "File contains no data, closing file" << std::endl;
		}
		file.close();
	}
	else
	{
		std::cout << "Unable to open file: " << filename << std::endl;
	}
	return EXIT_SUCCESS;
}


	
