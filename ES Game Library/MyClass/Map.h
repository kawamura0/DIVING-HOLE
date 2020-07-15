/**
* @file MapCourse.cpp
* @brief �}�b�v�N���X�w�b�_�[�t�@�C��
* @author �쑺����(�q���[�}���A�J�f�~�[���Z)
* @date 2019/9/9
*/
#pragma once
#include "../ESGLib.h"
class Map
{
public:
	bool Initialize(LPCTSTR filename, float scale);
	void Update();
	void Draw(Vector3 pos);

	MODEL GetTerrain() const { return map_stage_; }

private:
	MODEL   map_stage_;
	Vector3 pos_;
};
