/**
* @file Player.h
* @brief プレイヤークラスヘッダーファイル
* @author 狩野 弘貴(ヒューマンアカデミー仙台校)
* @date 2019/3/5
*/
#pragma once

#include "../ESGLib.h"
#include "enum.h"
#include "Map.h"

/**
* プレイヤーの処理
*/
class Player
{
public:
	bool Initialize();
	void Update(Map &map);
	void Draw();
	void Ui();

	ANIMATIONMODEL GetPlayer(){ return player_; }

	void FreeFall();
	void PlayerJump();

	void PlayerMove();
	
	void ThirdPersonCamera();

	void JumpEnd();
	void FreeFallEnd();

	FONT medium_font_;
	FONT big_font_;

private:
	CAMERA camera_;

	ANIMATIONMODEL player_;
	
	GamePadState   pad_state_;
	GamePadBuffer  pad_buffer_;
	KeyboardState  key_state_;
    KeyboardBuffer key_buffer_;

	Vector3 player_position_;
	Vector3 player_rotation_;
	Vector3 jump_position_;

	int fall_time_;
	int move_ju_fa_gr_state_;
	int jump_time_;
	int speed_limit_;

	float pad_paramater_;
	float gravity_y_;
    float height_;
	float height_down_;
	float jump_speed_;
	float jump_a_time_;
	float move_speed_;

	bool slow_mode_;

	float cam_pos_;

	//-------------------------------------------------------
	//マップ//
	
	MODEL map_model_;

	//TODO:仮版(デバッグ用)
	int mae_usiro; //前、もしくは後ろを向いているのかを判定するステート
	float aa_jump; //デバッグ用(距離を表示する為の変数)
	float aa_fall;
};

