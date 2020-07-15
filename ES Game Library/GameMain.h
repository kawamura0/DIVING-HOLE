#pragma once

#include "ESGLib.h"
#include "GameScene/GameScene.hpp"
//#include "MyClass/Player.h"
#include "MyClass/Map.h"
#include "MyClass/enum.h"

class GameMain : public CGameScene 
{
public:
	GameMain() : DefaultFont(GraphicsDevice.CreateDefaultFont())
	{
//		ContentRootDirectory(_T("Content"));
	}

	virtual ~GameMain()
	{
#ifdef _INC_SQUIRREL
		Squirrel.ReleaseAllScripts();
#endif
#ifdef _INC_NUI
		NUI.ReleaseAllKinects();
#endif
#ifdef _INC_LIVE2D
		Live2D.ReleaseAllModels();
#endif
#ifdef _INC_EFFEKSEER
		Effekseer.ReleaseAllEffects();
#endif
#ifdef _INC_DIRECT2D
		Direct2D.ReleaseAllResources();
#endif
		MediaManager.ReleaseAllMedia();

		SoundDevice.ReleaseAllMusics();
		SoundDevice.ReleaseAllSounds();

		GraphicsDevice.ReleaseAllRenderTargets();
		GraphicsDevice.ReleaseAllStateBlocks();
		GraphicsDevice.ReleaseAllFonts();
		GraphicsDevice.ReleaseAllSprites();
		GraphicsDevice.ReleaseAllAnimationModels();
		GraphicsDevice.ReleaseAllModels();
		GraphicsDevice.ReleaseAllVertexBuffers();
		GraphicsDevice.ReleaseAllEffects();

		Finalize();
	}

public:
	virtual bool Initialize();

	virtual int  Update();
	virtual void Draw();

	//プレイヤー//
	bool PlayerInitialize();
	void PlayerUpdate(Map* map);
	void PlayerDraw();
	void PlayerUi();

	void FreeFall();
	void PlayerJump();

	void PlayerMove();

	void ThirdPersonCamera();

	void JumpEnd();
	void FreeFallEnd();

	void SlowGaugeStatus();
	void HPGauge(int a);

	FONT medium_font_;
	FONT big_font_;
	FONT test_font;
	//----------//

private:
	void Finalize();
	FONT DefaultFont;

private:
	// 変数宣言

	//マップ//
	Map            map[map_type];
	MODEL          map_model[map_type];
	Vector3        map_pos;
	float          file_read_y[2];
	int            reader_randam;

	std::list<Vector3>hasira_position_list_;
	std::list<Vector3>hasira2_position_list_;
	std::list<Vector3>ana_position_list_;
	std::list<Vector3>hasira_yoko_position_list_;

	//アイテム
	MODEL          items_[item_type];
	int            count;
	float          distance[item_type];
	float          item_scale[item_type];

	std::list<Vector3> tubu_position_list_;
	std::list<float>   tubu_rotation_x_list;
	std::list<Vector3> furisuku_position_list_;
	std::list<float>   furisuku_rotation_x_list;

	StreamReader   reader_map;//ファイル読み込み

	//スローゲージ//
	ScreenVertex   gauge_vertex[VERTEX_MAX];//頂点の数
	ScreenVertex   gauge_vertex2[VERTEX_MAX];
	ScreenVertex   gauge_vertex3[VERTEX_MAX];
	float          gauge;

	//体力ゲージ//
	ScreenVertex   hp_vertex[VERTEX_MAX];
	ScreenVertex   hp_vertex2[VERTEX_MAX];
	ScreenVertex   hp_vertex3[VERTEX_MAX];
	float          hp_gauge;

	// 関数宣言
	void Map_Read();
	void Map_Copy();
	void Gage_Draw();
	void HP_Draw();
	void Map_Create();
	void GameOver();
	void Effect_Initialize();
	void Effect_Draw();
	void Collsion_Initialize();
	void Collsion_Update();
	
	//プレイヤー//-----//
	CAMERA         camera_;

	SPRITE         player_effect_[EFFCT_MAX];
	ANIMATIONMODEL player_;

	MODEL          starting_point_;

	ANIMATIONMODEL crash_;

	MODEL          next_meters_model_;
	int            next_meters_;
	void           SetNextMetersModel(const int next_meters);

	GamePadState   pad_state_;
	GamePadBuffer  pad_buffer_;
	KeyboardState  key_state_;
	KeyboardBuffer key_buffer_;

	SPRITE         result_;

	Vector3        player_position_;
	Vector3        player_rotation_;
	Vector3        jump_position_;
	Vector3        point_pos_;

	int            fall_time_;
	int            move_ju_fa_gr_state_;
	int            jump_time_;
	int            speed_limit_;
	int            slow_gauge_;
	int            slow_count_;
	//TODO:ハイスコア

	float          gravity_y_;
	float          height_;
	float          height_down_;
	float          jump_speed_;
	float          jump_a_time_;
	float          move_speed_x_;
	float          move_speed_z_;
	float          cam_pos_;
	float          fall_min_dist_;
	float          anim_time_;
	float          player_f_;
	float          player_r_;
	float          player_l_;
	float          fall_dist_;
	float          fall_meters_;

	bool           slow_mode_;
	bool           slow_flag_;
	bool           gameover_flag_;

	//サウンド//
	SOUND          wind_sound_;
	SOUND3D        wind_3D_;

	SOUNDLISTENER  listener;

	float          wind_x_;
	float          wind_z_;

	SOUND          slow_sound_;

	//-------------------------------------------------------

	void           HpReset();
	int            heal_count_;
	int            heal_mode_;

	void           HpHeal();
	float          hp_heal_time;
	
	//TODO:仮版(デバッグ用)
	float          aa_jump; //デバッグ用(距離を表示する為の変数)
	float          aa_fall;

	//---------//-----//

	MODEL          sen;
	Material       mtrl[SPEEDLINE_MAX];
	Vector3        sen_pos[SPEEDLINE_MAX];
	Vector3        sen_direction[SPEEDLINE_MAX];
	Vector3        camera_pos;
	int            sen_time;
	int            sen_state;

	////パーティクル
	MODEL          s_point;
	Material       mat[SPEEDPOINT_MAX];
	Vector3        s_point_pos[SPEEDPOINT_MAX];
	Vector3        s_point_direction[SPEEDPOINT_MAX];

	//タイトル
	SPRITE         title;
	bool           title_flag;
	
	//メイン
	bool           main_flag;
	
	//リザルト
	SPRITE         select_bar;
	float          bar_pos_y;
	int            bar_state;


	FONT           start_font;
	
	//ランキング
	int            rank_score[4];	//スコア

	StreamWriter   write_rank;		//ランキング書き込み
	StreamReader   reader_rank;		//ランキング読み込み

	std::list<int> rank_score_list1;

	//フォグカラー//
	int            fog_color_red_, fog_color_green_, fog_color_blue_;
	float          old_fall;
	int            dividing;

	//ぼかしシェーダー
	EFFECT         blur;
	float          blur_depth;
	RENDERTARGET   offscreen_;

};
