//------------------------------------------------------------------------------
// This code provides additional routine for KinectFusionExplorer-D2D.
// It supports loav/save of TSDF volume generated by KinectFusion.
// Author: Seung-Tak Noh (seungtak.noh@gmail.com)
//------------------------------------------------------------------------------
#include "KinectFusionHelperEx.h"

// save/load volume data in XML document
#include <pugixml.hpp>
#include "voxelData.pb.h"

/// <summary>
/// import volumetric data in XML formatted file
/// </summary>
bool LoadXML(const char* xmlFile, KinfuData& kinfuData)
{
	printf("read: %s ... \n", xmlFile);

	pugi::xml_document doc;

	// read and parse in advance
	if (!doc.load_file(xmlFile))
	{
		std::cerr << "ERROR: cannot read this file: " << xmlFile << std::endl;
		return false;
	}

	// all data is under <kinfu_savedata>
	pugi::xml_node root = doc.child("kinfu_savedata");

	////////////////////////////////////////////////////////////
	// 1) camera transform
	// 2) volume transform
	////////////////////////////////////////////////////////////
	pugi::xml_node xn_camera = root.child("camera_transform");
	pugi::xml_node xn_volume = root.child("volume_transform");

	auto ReadMatrix4 = [](pugi::xml_node xn_node)->Matrix4
	{
		pugi::xml_node node = xn_node.child("Matrix4");

		Matrix4 mat;

		mat.M11 = atof(node.child("M11").child_value());
		mat.M12 = atof(node.child("M12").child_value());
		mat.M13 = atof(node.child("M13").child_value());
		mat.M14 = atof(node.child("M14").child_value());

		mat.M21 = atof(node.child("M21").child_value());
		mat.M22 = atof(node.child("M22").child_value());
		mat.M23 = atof(node.child("M23").child_value());
		mat.M24 = atof(node.child("M24").child_value());

		mat.M31 = atof(node.child("M31").child_value());
		mat.M32 = atof(node.child("M32").child_value());
		mat.M33 = atof(node.child("M33").child_value());
		mat.M34 = atof(node.child("M34").child_value());

		mat.M41 = atof(node.child("M41").child_value());
		mat.M42 = atof(node.child("M42").child_value());
		mat.M43 = atof(node.child("M43").child_value());
		mat.M44 = atof(node.child("M44").child_value());

		return mat;
	};
	kinfuData.worldToCameraTransform = ReadMatrix4(xn_camera);
	kinfuData.worldToVolumeTransform = ReadMatrix4(xn_volume);

	////////////////////////////////////////////////////////////
	// 3) TSDF volume array dimension
	////////////////////////////////////////////////////////////
	pugi::xml_node xn_setting = root.child("setting");

	int voX, voY, voZ;
	kinfuData.reconsParams.voxelsPerMeter = atof(xn_setting.child("voxelsPerMeter").child_value());
	kinfuData.reconsParams.voxelCountX    = voX = atoi(xn_setting.child("voxelsX").child_value());
	kinfuData.reconsParams.voxelCountY    = voY = atoi(xn_setting.child("voxelsY").child_value());
	kinfuData.reconsParams.voxelCountZ    = voZ = atoi(xn_setting.child("voxelsZ").child_value());

	////////////////////////////////////////////////////////////
	// 4) voxels 
	////////////////////////////////////////////////////////////
	pugi::xml_node xn_voxels = root.child("volume_data");

	// allocate voxel area in advance
	int voxelArea = voX * voY * voZ;
	kinfuData.volumeBlock      = std::vector<short>(voxelArea, (short)-32768); // (NULL) distance
	kinfuData.colorVolumeBlock = std::vector<int>  (voxelArea);

	// serialize TSDF array by using protobuf
	for (int k = 0; k < voZ; k++)
	{
		char buf[128];
		sprintf_s(buf, "layer_%03d", k);

		printf("%s", buf);

		pugi::xml_node xn_layer = xn_voxels.child(buf);

		// read voxel data from deserialized data
		if (nullptr == xn_layer) {
			fprintf(stderr, " - cannot find this block in XML.\n");
			continue;
		}

		const char* rawData = xn_layer.child_value();

		if (nullptr != rawData) {

			// decode and deserialize base64 string *************************** [TEMPORARY] compatible to protobuf-NET
			std::string base64(rawData);
			std::string byteArray;
			google::protobuf::Base64Unescape(base64, &byteArray);

			// deserialization
			List_Voxel voxelData;
			voxelData.ParseFromString(byteArray);

			for (int i = 0; i < voxelData.items_size(); i++) {
				const Voxel& voxel = voxelData.items(i);

				// get partial data
				int   index = voxel.index();
				short tsdf  = (short)voxel.tsdf();
				int   color = voxel.color();

				// insert data based on index
				kinfuData.volumeBlock[index]      = tsdf;
				kinfuData.colorVolumeBlock[index] = color;
			}
		}

		printf("\r");
	}

	printf("read: %s ... [DONE]\n", xmlFile);
	return true;
}

/// <summary>
/// export volumetric data in XML formatted file
/// </summary>
bool SaveXML(const char* xmlFile, const KinfuData& kinfuData)
{
	printf("write: %s ... \n", xmlFile);

	pugi::xml_document doc;

	// default declaration: <?xml version="1.0" encoding="UTF-8"?>
	auto desclearationNode = doc.append_child(pugi::node_declaration);
	desclearationNode.append_attribute("version") = "1.0";
	desclearationNode.append_attribute("encoding") = "UTF-8";

	// all data is under <kinfu_savedata>
	auto root = doc.append_child("kinfu_savedata");

	////////////////////////////////////////////////////////////
	// 1) camera transform
	// 2) volume transform
	////////////////////////////////////////////////////////////
	pugi::xml_node xn_camera = root.append_child("camera_transform");
	pugi::xml_node xn_volume = root.append_child("volume_transform");

	auto WriteMatrix4 = [](pugi::xml_node xn_node, const Matrix4 mat)
	{
		pugi::xml_node node = xn_node.append_child("Matrix4");
		node.append_attribute("xmlns:xsd") = "http://www.w3.org/2001/XMLSchema";
		node.append_attribute("xmlns:xsi") = "http://www.w3.org/2001/XMLSchema-instance";

		char buf[128];
		auto to_cstr = [&](float value)->const char* {
			sprintf_s(buf, "%f", value);
			return buf;
		};

		node.append_child("M11").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M11));
		node.append_child("M12").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M12));
		node.append_child("M13").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M13));
		node.append_child("M14").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M14));

		node.append_child("M21").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M21));
		node.append_child("M22").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M22));
		node.append_child("M23").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M23));
		node.append_child("M24").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M24));

		node.append_child("M31").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M31));
		node.append_child("M32").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M32));
		node.append_child("M33").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M33));
		node.append_child("M34").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M34));

		node.append_child("M41").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M41));
		node.append_child("M42").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M42));
		node.append_child("M43").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M43));
		node.append_child("M44").append_child(pugi::node_pcdata).set_value(to_cstr(mat.M44));
	};
	WriteMatrix4(xn_camera, kinfuData.worldToCameraTransform);
	WriteMatrix4(xn_volume, kinfuData.worldToVolumeTransform);

	////////////////////////////////////////////////////////////
	// 3) TSDF volume array dimension
	////////////////////////////////////////////////////////////
	float vpm = kinfuData.reconsParams.voxelsPerMeter;
	int   voX = kinfuData.reconsParams.voxelCountX;
	int   voY = kinfuData.reconsParams.voxelCountY;
	int   voZ = kinfuData.reconsParams.voxelCountZ;

	pugi::xml_node xn_setting = root.append_child("setting");
	{
		char buf[128];
		sprintf_s(buf, "%f", vpm);

		auto to_cstr = [&](int value)->const char* {
			sprintf_s(buf, "%d", value);
			return buf;
		};
		xn_setting.append_child("voxelsPerMeter").append_child(pugi::node_pcdata).set_value(buf);
		xn_setting.append_child("voxelsX").append_child(pugi::node_pcdata).set_value(to_cstr(voX));
		xn_setting.append_child("voxelsY").append_child(pugi::node_pcdata).set_value(to_cstr(voY));
		xn_setting.append_child("voxelsZ").append_child(pugi::node_pcdata).set_value(to_cstr(voZ));
	}

	////////////////////////////////////////////////////////////
	// 4) voxels
	////////////////////////////////////////////////////////////
	pugi::xml_node xn_voxels = root.append_child("volume_data");
	int voxelArea = voX * voY * voZ;

	// serialize TSDF array by using protobuf
	for (int k = 0; k < voZ; k++)
	{
		char buf[128];
		sprintf_s(buf, "layer_%03d", k);

		printf("%s", buf);

		List_Voxel voxelData;

		for (int j = 0; j < voY; j++)
		for (int i = 0; i < voX; i++) {
			int index = i + j * voX + k * voX * voY;

			short tsdf = kinfuData.volumeBlock[index];
			int color  = kinfuData.colorVolumeBlock[index];

			if (tsdf <= -32767) continue;

			Voxel* voxel = voxelData.add_items();
			voxel->set_index(index);
			voxel->set_tsdf((int)tsdf);
			voxel->set_color(color);
		}

		// serialize and encode to base64 string ****************************** [TEMPORARY] compatible to protobuf-NET
		std::string ser    = voxelData.SerializeAsString();
		std::string base64;
		google::protobuf::Base64Escape(ser, &base64);

		// add to XML
		pugi::xml_node node_layer = xn_voxels.append_child(buf);
		node_layer.append_child(pugi::node_pcdata).set_value(base64.c_str());

		printf("\r");
	}

	// write XML file
	doc.save_file(xmlFile);

	printf("write: %s ... [DONE]\n", xmlFile);
	return true;
}