/**
* @file MapCourse.cpp
* @brief �}�b�v�N���X�\�[�X�t�@�C��
* @author �쑺����(�q���[�}���A�J�f�~�[���Z)
* @date 2019/9/9
*/
#include "Map.h"

bool Map::Initialize(LPCTSTR mapname, float scale)
{
	auto map_model_ = mapname;
	map_stage_ = GraphicsDevice.CreateModelFromFile(map_model_);
	map_stage_->SetScale(scale);
	
	return true;
}
void Map::Update()
{
	
}
void Map::Draw(Vector3 pos)
{
	pos_ = pos;
	map_stage_->SetPosition(pos_);
	map_stage_->Draw();
}

