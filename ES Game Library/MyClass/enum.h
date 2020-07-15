/**
* @file enum.h
* @brief enum�p�w�b�_�[�t�@�C��
* @author ��� �O�M(�q���[�}���A�J�f�~�[���Z)
* @date 2019/3/5
*/
#pragma once

/**
* enum��p�w�b�_�[�t�@�C��
*/
enum SPEED_MODE_
{
	FORWARD,
	RETREAT,
};

enum CAMERA_MODE_
{
	THIRD_PERSON,
	SIDEVIEW,
};

enum ITEM_
{
	DELETE_,
	EXIST,
};

enum SLOW_GAUGE_MADE_
{
	USE  = 0,
	HIAL = 1,
};

enum SLOW_GAUGE_
{
	MAX = 300,
	LOW = 200,
};

enum CONDITION_STATE_
{
	NORMAL,
	FALL,
	JUMP,
};

enum
{ 
	DIST_MAX = 1000000,
	EFFCT_MAX = 11,
	VERTEX_MAX = 6,
	SPEEDLINE_MAX = 100,
	SPEEDPOINT_MAX = 10000,
	item_type = 2,
	map_type = 4,
	SOUND_TYPE = 3
};