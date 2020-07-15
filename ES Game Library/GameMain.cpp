#include "StdAfx.h"
#include "GameMain.h"

bool GameMain::Initialize()
{
	WindowTitle(_T("DAIVING　HOLE"));

	GameTimer.SetFPS(60);
	InputDevice.CreateGamePad(1);

	Light light_;
	light_.Type      = Light_Directional;
	light_.Diffuse   = Color(1.0f, 1.0f, 1.0f);
	light_.Specular  = Color(1.0f, 1.0f, 1.0f);
	light_.Ambient   = Color(1.0f, 1.0f, 1.0f);
	light_.Direction = Vector3(-2.0f, -5.0f, 0.0f);	
	GraphicsDevice.SetLight(light_);

	PlayerInitialize();

	fog_color_red_   = 225;
	fog_color_green_ = 225;
	fog_color_blue_  = 255;

	dividing   = 0;
	blur_depth = 0.0f;

	title = GraphicsDevice.CreateSpriteFromFile(_T("UI/Title.png"));

	title_flag     = true;
	main_flag      = false;
	gameover_flag_ = false;

	map[0].Initialize(_T("3D/Map/hasira.X"), (1));
	map[1].Initialize(_T("3D/Map/hasira_itanaga.X"), (1));
	map[2].Initialize(_T("3D/Map/hasira_migi_naname.X"), (1));
	map[3].Initialize(_T("3D/Map/hasira_hidari_naname.X"), (1));

	items_[0] = GraphicsDevice.CreateModelFromFile(_T("3D/item/tubu.X"));
	items_[1] = GraphicsDevice.CreateModelFromFile(_T("3D/item/hurisuku.X"));

	item_scale[0] = 3;//フリスク
	item_scale[1] = 3;//フリスクの箱

	Effect_Initialize();

	Collsion_Initialize();

	Map_Read();

	//ゲージ
	gauge    = 450;
	hp_gauge = 600.0f;

	//スコア読み込み
	reader_rank.Open(_T("txt/rank.txt"));
	for (int i = 0; i < 3; i++) {
		const int score = reader_rank.ReadInt32();
		if (reader_rank.IsEOF())
			break;
		rank_score[i] = score;
	}
	reader_rank.Close();

	rank_score_list1.clear();

	//フォグ
	GraphicsDevice.SetRenderState(Fog_Enable);				//フォグを有効にする
	GraphicsDevice.SetRenderState(FogVertexMode_Linear);	//線形フォグ
	GraphicsDevice.SetRenderState(FogColor(fog_color_red_, fog_color_green_, fog_color_blue_)); //フォグの色
	GraphicsDevice.SetRenderStateF(FogStart, 200.0f);		//フォグの開始点
	GraphicsDevice.SetRenderStateF(FogEnd,  5000.0f);		//フォグの終了点

	wind_sound_ = SoundDevice.CreateBufferFromFile(_T("SE/DownFoll_mono.wav"), true);
	wind_3D_    = wind_sound_->GetSound3D();
	wind_x_ = 0.0f;
	wind_z_ = 1.5f;	

	slow_sound_ = SoundDevice.CreateSoundFromFile(_T("SE/heartbeat.wav"));
	slow_sound_->Stop();

	sen = GraphicsDevice.CreateModelFromFile(_T("3D/sen.X"));
	sen->SetScale(2.0f, 10.0f, 1.0f);
	sen_state = 0;

	//パーティクル
	s_point = GraphicsDevice.CreateModelFromFile(_T("3D/sen.X"));
	s_point->SetScale(0.015f, 0.003f, 0.015f);
	camera_pos = camera_->GetPosition();
	for (int i = 0; i < SPEEDPOINT_MAX; i++)
	{
		mat[i].Emissive = Color(175, 255, 255);
		s_point_pos[i] = Vector3(camera_pos.x, camera_pos.y - 10000.0f, camera_pos.z);
		s_point_direction[i] = Vector3(MathHelper_Random(-80, 80) / 1000.0f, 3, MathHelper_Random(-80, 80) / 1000.0f);
		s_point_direction[i] = Vector3_Normalize(s_point_direction[i]) * MathHelper_Random(10, 100);
	}

	//ぼかし
	Viewport   view_port = GraphicsDevice.GetViewport();
	blur = GraphicsDevice.CreateEffectFromFile(_T("FX/blur.fx"));
	blur->SetParameter("AddU", (float)(1.0f / view_port.Width ) * blur_depth);
	blur->SetParameter("AddV", (float)(1.0f / view_port.Height) * blur_depth);

	offscreen_ = GraphicsDevice.CreateRenderTarget(view_port.Width, view_port.Height, PixelFormat_RGBA8888, DepthFormat_Unknown);

	return true;
}

void GameMain::Finalize()
{
	// TODO: Add your finalization logic here

}

int GameMain::Update()
{
	key_buffer_ = Keyboard->GetBuffer();
	pad_state_  = GamePad(0)->GetState();
	pad_buffer_ = GamePad(0)->GetBuffer();

	if (key_buffer_.IsPressed(Keys_Enter) 
	 || pad_buffer_.IsPressed(GamePad_Button8 )
     || pad_buffer_.IsPressed(GamePad_Button10)
	 || pad_buffer_.IsPressed(GamePad_Button2))
	{
		title_flag = false;
		main_flag  = true;

		wind_sound_->PlayLooping();
	}

	if(main_flag && slow_flag_)
	{
		wind_sound_->Stop();
	}

	//セレクト
	if (bar_state)
	{
		if (key_buffer_.IsPressed(Keys_Up) || pad_state_.Y < Axis_Center)
		{
			bar_pos_y = 390;
		}

		if (key_buffer_.IsPressed(Keys_Down) || pad_state_.Y > Axis_Center)
		{
			bar_pos_y = 600;
		}

		// エンターキーを押したか？
		auto isPressedEnterKey = [&]()
		{ return pad_buffer_.IsPressed(GamePad_Button1) ||
				 pad_buffer_.IsPressed(GamePad_Button2) ||
				 pad_buffer_.IsPressed(GamePad_Button3) ||
				 pad_buffer_.IsPressed(GamePad_Button4) ||
				 key_buffer_.IsPressed(Keys_Enter); 
		};

		if (isPressedEnterKey()) {
			if (bar_pos_y == 600)
			{
				Initialize();
				bar_state = false;
				title_flag = true;
			}
			else if (bar_pos_y == 390)
			{
				Initialize();
				bar_state = false;
				gameover_flag_ = false;
				main_flag = true;
				title_flag = false;
			}
		}
	}

	Map_Copy();
	Collsion_Update();

	if (key_buffer_.IsReleased(Keys_LeftShift)
	 || pad_buffer_.IsReleased(GamePad_Button1)
	 || pad_buffer_.IsReleased(GamePad_Button2)
	 || pad_buffer_.IsReleased(GamePad_Button3)
	 || pad_buffer_.IsReleased(GamePad_Button4)
	 || pad_buffer_.IsReleased(GamePad_Button5)
	 || pad_buffer_.IsReleased(GamePad_Button6)
	 || pad_buffer_.IsReleased(GamePad_Button7))
	{
		wind_sound_->PlayLooping();
	}

	if (main_flag)
	{
		const int  FALL_DIST = fall_dist_ / 50;
		const int  OLD_FALL = old_fall / 50;

		//進んだ距離に応じてフォグの色を変える
		if (FALL_DIST >= 3000 + dividing && FALL_DIST < 5000 + dividing)
		{
			fog_color_red_ -= 0.0001f;
			fog_color_blue_ -= 0.0001f;
			if (fog_color_red_ <= 200)
			{
				fog_color_red_ = 200;
			}
			if (fog_color_blue_ <= 225)
			{
				fog_color_blue_ = 225;
			}
		}

		if (FALL_DIST >= 5000 + dividing && FALL_DIST < 7000 + dividing)
		{
			fog_color_blue_ -= 0.0001f;
			if (fog_color_blue_ <= 150)
			{
				fog_color_blue_ = 150;
			}
		}

		if (FALL_DIST >= 7000 + dividing && FALL_DIST < 9000 + dividing)
		{
			fog_color_green_ -= 5.0f;
			fog_color_blue_ -= 1.0f;
			fog_color_red_ += 0.0001f;
			if (fog_color_green_ <= 120)
			{
				fog_color_green_ = 120;
			}
			if (fog_color_blue_ <= 120)
			{
				fog_color_blue_ = 120;
			}
			if (fog_color_red_ >= 255)
			{
				fog_color_red_ = 255;
			}
		}

		if (FALL_DIST >= 9000 + dividing && FALL_DIST < 10000)
		{
			fog_color_red_ -= 1.0f;
			fog_color_blue_ += 1.0f;
			fog_color_green_ += 1.0f;
			if (fog_color_red_ <= 225)
			{
				fog_color_red_ = 225;
			}
			if (fog_color_blue_ >= 255)
			{
				fog_color_blue_ = 255;
			}
			if (fog_color_green_ >= 225)
			{
				fog_color_green_ = 225;
			}
		}

		if (FALL_DIST / 10000 != OLD_FALL / 10000)
		{
			dividing += 10000;
		}

		
		//パーティクル
		camera_pos = camera_->GetPosition();
		for (int i = 0; i < SPEEDPOINT_MAX; i++)
		{
			s_point_pos[i] += s_point_direction[i];
			if (s_point_pos[i].y > camera_pos.y + 1.0f) {
				mat[i].Emissive      = Color(175, 255, 255);
				s_point_pos[i]       = Vector3(camera_pos.x, camera_pos.y - 10000.0f, camera_pos.z);
				s_point_direction[i] = Vector3(MathHelper_Random(-80, 80) / 1000.0f, 3, MathHelper_Random(-80, 80) / 1000.0f);
				s_point_direction[i] = Vector3_Normalize(s_point_direction[i]) * MathHelper_Random(10, 100);
			}
		}

		PlayerUpdate(map);
		
		if (sen_state == 1)
		{
			camera_pos = camera_->GetPosition();
			for (int i = 0; i < SPEEDLINE_MAX; i++)
			{
				sen_pos[i] += sen_direction[i];
				if (sen_pos[i].y > camera_pos.y + 100.0f) {
					mtrl[i].Emissive = Color(MathHelper_Random(128, 255), MathHelper_Random(128, 255), MathHelper_Random(128, 255));
					sen_pos[i]       = Vector3(camera_pos.x, camera_pos.y - 10000.0f, camera_pos.z);
					sen_direction[i] = Vector3(MathHelper_Random(-100, 100) / 100.0f, 3, MathHelper_Random(-100, 100) / 100.0f);
					sen_direction[i] = Vector3_Normalize(sen_direction[i]) * 200;
				}
			}

			//ぼかし
			blur_depth += 0.1f;
			if (blur_depth >= 3.0)
			{
				blur_depth = 3.0f;
			}
			blur->SetParameter("AddU", (float)(1.0 / 1280)* (3.0f - blur_depth));
			blur->SetParameter("AddV", (float)(1.0 / 720.0)* (3.0f - blur_depth));

			sen_time++;
			if (sen_time >= 25)
			{
				sen_state = 0;
			}
		}
	}
	
	ThirdPersonCamera();
	GraphicsDevice.SetCamera(camera_);

	return 0;
}

void GameMain::Draw()
{
	GraphicsDevice.Clear(Color_GhostWhite);

	GraphicsDevice.BeginScene();

	GraphicsDevice.SetRenderTarget(offscreen_);
	GraphicsDevice.Clear(Color_GhostWhite);

	if (main_flag)
	{
		HP_Draw();
		Gage_Draw();
	}

	GraphicsDevice.SetRenderState(Fog_Enable);
	GraphicsDevice.SetRenderState(FogColor(fog_color_red_, fog_color_green_, fog_color_blue_));

	auto DrawList = [&](int num, std::list<Vector3>& position_list)
	{
		for (auto& pos : position_list)
		{
			this->map[num].Draw(pos);
		}
	};

	DrawList(0, hasira_position_list_);
	DrawList(1, hasira2_position_list_);
	DrawList(2, ana_position_list_);
	DrawList(3, hasira_yoko_position_list_);


	auto DrawItemList = [&](int num,std::list<Vector3> & position_list, std::list<float> & rotation_x_list)
	{
		auto it_pos = position_list.begin();
		auto it_rot = rotation_x_list.begin();

		while (true)
		{
			if (it_pos == position_list.end() ||
				it_rot == rotation_x_list.end())
				break;

			items_[num]->SetPosition(*it_pos);
			items_[num]->SetRotation(*it_rot, 0.0f, 0.0f);
			items_[num]->SetScale(item_scale[num]);
			items_[num]->Draw();

			//回転
			*it_rot += 10.0f;
			if (*it_rot >= 360.0f)
				*it_rot -= 360.0f;

			it_pos++;
			it_rot++;
		}
	};

	DrawItemList(0, tubu_position_list_,     tubu_rotation_x_list);
	DrawItemList(1, furisuku_position_list_, furisuku_rotation_x_list);


	PlayerDraw();

	GraphicsDevice.SetRenderState(Fog_Disable);

	if (sen_state == 1)
	{
		//集中線描画
		//板ポリなので背面描画もする
		GraphicsDevice.SetRenderState(CullMode_None);
		for (int i = 0; i < SPEEDLINE_MAX; i++)
		{
			sen->SetPosition(sen_pos[i]);
			sen->SetMaterial(mtrl[i]);
			sen->Draw();
		}
		GraphicsDevice.SetRenderState(CullMode_CullCounterClockwiseFace);
	}

	//パーティクル
	GraphicsDevice.SetRenderState(CullMode_None);
	if (!slow_flag_)
	{
		for (int i = 0; i < SPEEDPOINT_MAX; i++)
		{
			s_point->SetPosition(s_point_pos[i]);
			s_point->SetMaterial(mat[i]);
			s_point->Draw();
		}
	}
	GraphicsDevice.SetRenderState(CullMode_CullCounterClockwiseFace);
	//////

	SpriteBatch.Begin();
	
	if (!gameover_flag_)
	{
		Effect_Draw();
	}
	
	PlayerUi();

	SpriteBatch.End();

	GraphicsDevice.RenderTargetToBackBuffer(offscreen_, blur);

	GraphicsDevice.EndScene();
}

/**
* @brief マップ読み込みの処理
*/
void GameMain::Map_Read()
{

	hasira_position_list_.clear();
	for (int i = 0; i < 2; i++) 
	{
		Map_Create();
	}
	for (auto& read_y : file_read_y)
	{
		read_y = 0;
	}
}

/**
* @brief マップ生成の処理
*/
void GameMain::Map_Create()
{
	float   most_y = FLT_MAX;

	auto reader_position = [&](std::list<Vector3> & position_list)
	{
		map_pos.x = reader_map.ReadFloat();
		map_pos.y = reader_map.ReadFloat() + file_read_y[1];
		map_pos.z = reader_map.ReadFloat();

		if (most_y > map_pos.y)
			most_y = map_pos.y;
	};

	reader_randam = MathHelper_Random(5, 9);

	TCHAR   filename[MAX_PATH];
	::wsprintf(filename, _T("txt/Stage%d.txt"), reader_randam);
	reader_map.Open(filename);

	while (true)
	{
		//一文字目を読み取る
		int reader_type = reader_map.ReadChar();
		if (reader_map.IsEOF())
			break;
		if (reader_type == 't')
		{
			reader_position(tubu_position_list_);
			tubu_position_list_.push_back(map_pos);
			tubu_rotation_x_list.push_back(0.0f);
		}
		else if (reader_type == 'f')
		{
			reader_position(furisuku_position_list_);
			furisuku_position_list_.push_back(map_pos);
			furisuku_rotation_x_list.push_back(0.0f);
		}
		else if (reader_type == 'h')
		{
			reader_position(hasira_position_list_);
			hasira_position_list_.push_back(map_pos);
		}
		else if (reader_type == 'i')
		{
			reader_position(hasira2_position_list_);
			hasira2_position_list_.push_back(map_pos);
		}
		else if (reader_type == 'l')
		{
			reader_position(ana_position_list_);
			ana_position_list_.push_back(map_pos);
		}
		else if (reader_type == 'r')
		{
			reader_position(hasira_yoko_position_list_);
			hasira_yoko_position_list_.push_back(map_pos);
		}

	}
	reader_map.Close();

	file_read_y[0] = file_read_y[1];
	file_read_y[1] = most_y;
}

/**
* @brief マップ再生成の処理
*/
void GameMain::Map_Copy()
{
	auto player_pos = player_->GetPosition();

	auto Regeneration = [&](std::list<Vector3> & position_list)
	{
		auto& it = position_list.begin();
		while (it != position_list.end())
		{
			if (it->y > player_pos.y + 2000)
			{
				it = position_list.erase(it);
			}
			else {
				it++;
			}
		}
	};

	Regeneration(tubu_position_list_);
	Regeneration(furisuku_position_list_);
	Regeneration(hasira_position_list_);
	Regeneration(hasira2_position_list_);
	Regeneration(ana_position_list_);
	Regeneration(hasira_yoko_position_list_);

	if (file_read_y[0] >= player_pos.y)
	{
		Map_Create();
	}

}
/**
* @brief ポリゴンの頂点の座標の計算をして並べる
*/
void GameMain::Gage_Draw()
{
	// ゲージの頂点に合わせて色をつける
	for (auto& gage : gauge_vertex) {
		gage.z = 0.0f;
		gage.color = Color(255, 40, 0);
	}

	for (auto& gage2 : gauge_vertex2) {
		gage2.z = 0.0f;
		gage2.color = Color(255, 80, 0);
	}

	for (auto& gage3 : gauge_vertex3) {
		gage3.z = 0.0f;
		gage3.color = Color(255, 120, 0);
	}

	Vector3   diagonal = Vector3_Transform(Vector3_Down, Matrix_CreateRotationZ(60.0f));//角度
	//ゲージの長さに合わせて三個の四角形を並べて描画をする
	if (gauge > 0)
	{
		int  gauge_len = gauge;
		if (gauge_len > 100)
		{
			gauge_len = 100;
		}

		gauge_vertex[0].x = -32.0f;
		gauge_vertex[0].y = 100.0f;

		gauge_vertex[1] = gauge_vertex[0];
		gauge_vertex[1].x += diagonal.x * 32.0f;
		gauge_vertex[1].y += diagonal.y * 32.0f;

		gauge_vertex[2].x = gauge_vertex[0].x + 100 - (100 - gauge_len);
		gauge_vertex[2].y = gauge_vertex[0].y;

		gauge_vertex[3] = gauge_vertex[1];

		gauge_vertex[4].x = gauge_vertex[3].x + 100 - (100 - gauge_len);
		gauge_vertex[4].y = gauge_vertex[3].y;

		gauge_vertex[5] = gauge_vertex[2];
		GraphicsDevice.DrawUserPrimitives(PrimitiveType_TriangleList, gauge_vertex, 2, ScreenVertex::FVF());
	}

	if (gauge > 100)
	{
		int  gage_len2 = gauge;
		if (gage_len2 > 250)
		{
			gage_len2 = 250;
		}

		gauge_vertex2[0] = gauge_vertex[5];

		gauge_vertex2[1] = gauge_vertex2[0];
		gauge_vertex2[1].x += diagonal.x * 48.0f;
		gauge_vertex2[1].y += diagonal.y * 48.0f;

		gauge_vertex2[2].x = gauge_vertex2[0].x + 150 - (250 - gage_len2);
		gauge_vertex2[2].y = gauge_vertex[0].y;

		gauge_vertex2[3] = gauge_vertex2[1];

		gauge_vertex2[4].x = gauge_vertex2[3].x + 150 - (250 - gage_len2);
		gauge_vertex2[4].y = gauge_vertex2[3].y;

		gauge_vertex2[5] = gauge_vertex2[2];

		GraphicsDevice.DrawUserPrimitives(PrimitiveType_TriangleList, gauge_vertex2, 2, ScreenVertex::FVF());
	}

	if (gauge > 250)
	{
		int  gage_len3 = gauge;
		if (gage_len3 > 450)
		{
			gage_len3 = 450;
		}
		gauge_vertex3[0] = gauge_vertex2[5];

		gauge_vertex3[1] = gauge_vertex3[0];
		gauge_vertex3[1].x += diagonal.x * 64.0f;
		gauge_vertex3[1].y += diagonal.y * 64.0f;

		gauge_vertex3[2].x = gauge_vertex3[0].x + 200 - (450 - gage_len3);
		gauge_vertex3[2].y = gauge_vertex[0].y;

		gauge_vertex3[3] = gauge_vertex3[1];

		gauge_vertex3[4].x = gauge_vertex3[3].x + 200 - (450 - gage_len3);
		gauge_vertex3[4].y = gauge_vertex3[3].y;

		gauge_vertex3[5] = gauge_vertex3[2];

		GraphicsDevice.DrawUserPrimitives(PrimitiveType_TriangleList, gauge_vertex3, 2, ScreenVertex::FVF());
	}

}

/**
* @brief ポリゴンの頂点の座標の計算をして並べる
*/
void GameMain::HP_Draw()
{
	// ゲージの頂点に合わせて色をつける
	for (auto& hp_gage : hp_vertex) {
		hp_gage.z = 0.0f;
		hp_gage.color = Color(0, 230, 140);
	}

	for (auto& hp_gage2 : hp_vertex2) {
		hp_gage2.z = 0.0f;
		hp_gage2.color = Color(0, 230, 140);
	}

	for (auto& hp_gage3 : hp_vertex3) {
		hp_gage3.z = 0.0f;
		hp_gage3.color = Color(0, 230, 140);
	}

	Vector3   hp_diagonal = Vector3_Transform(Vector3_Down, Matrix_CreateRotationZ(60.0f));//角度
	//ゲージの長さに合わせて三個の四角形を並べて描画をする
	if (hp_gauge > 0)
	{
		int  hp_len = hp_gauge;
		if (hp_len > 200)
		{
			hp_len = 200;
		}

		hp_vertex[0].x = 0.0f;
		hp_vertex[0].y = 85.0f;

		hp_vertex[1].x = hp_vertex[0].x;
		hp_vertex[1].y = 55.0f;

		hp_vertex[2].x = hp_vertex[1].x + 200 - (200 - hp_len);
		hp_vertex[2].y = hp_vertex[1].y;

		hp_vertex[3] = hp_vertex[0];

		hp_vertex[4] = hp_vertex[2];

		hp_vertex[5].x = hp_vertex[3].x + 200 - (200 - hp_len);
		hp_vertex[5].y = hp_vertex[3].y;

		GraphicsDevice.DrawUserPrimitives(PrimitiveType_TriangleList, hp_vertex, 2, ScreenVertex::FVF());
	}

	if (hp_gauge > 200)
	{
		int  hp_len2 = hp_gauge;
		if (hp_len2 > 400)
		{
			hp_len2 = 400;
		}

		hp_vertex2[0] = hp_vertex[5];

		hp_vertex2[1] = hp_vertex[4];

		hp_vertex2[2].x = hp_vertex2[1].x + 200 - (400 - hp_len2);
		hp_vertex2[2].y = hp_vertex2[1].y;

		hp_vertex2[3] = hp_vertex2[0];

		hp_vertex2[4] = hp_vertex2[2];

		hp_vertex2[5].x = hp_vertex2[3].x + 200 - (400 - hp_len2);
		hp_vertex2[5].y = hp_vertex2[3].y;

		GraphicsDevice.DrawUserPrimitives(PrimitiveType_TriangleList, hp_vertex2, 2, ScreenVertex::FVF());
	}

	if (hp_gauge > 400)
	{
		int  hp_len3 = hp_gauge;
		if (hp_len3 > 600)
		{
			hp_len3 = 600;
		}
		hp_vertex3[0] = hp_vertex2[5];

		hp_vertex3[1] = hp_vertex2[4];

		hp_vertex3[2].x = hp_vertex3[1].x + 200 - (600 - hp_len3);
		hp_vertex3[2].y = hp_vertex3[1].y;

		hp_vertex3[3] = hp_vertex3[0];

		hp_vertex3[4] = hp_vertex3[2];

		hp_vertex3[5].x = hp_vertex3[3].x + 200 - (600 - hp_len3);
		hp_vertex3[5].y = hp_vertex3[3].y;

		GraphicsDevice.DrawUserPrimitives(PrimitiveType_TriangleList, hp_vertex3, 2, ScreenVertex::FVF());
	}
}
/**
* @brief プレイヤーエフェクト初期化
*/
void GameMain::Effect_Initialize()
{
	TCHAR   file_name[MAX_PATH];
	for (int i = 0; i < EFFCT_MAX; i++)
	{
		::wsprintf(file_name, _T("sloweffect/effect_%05d.png"), i);
		player_effect_[i] = GraphicsDevice.CreateSpriteFromFile(file_name, PixelFormat_DXT5);
	}
}
/**
* @brief プレイヤーエフェクトの処理
*/
void GameMain::Effect_Draw()
{
	int pattern = (int)(gauge / 20.0f);//エフェクトスピード調整

	if(slow_flag_)
	{
		
		if (pattern >= 10)
		{
			pattern = 10;
		}
		SpriteBatch.Draw(*player_effect_[10 - pattern], Vector3(0.0f, 0.0f, -1000.0f));
	}
	else
	{
		pattern = 0;
	}
}

/**
* @brief プレイヤーとアイテムの当たり判定初期化
*/
void GameMain::Collsion_Initialize()
{
	count = 0;
	for (int i = 0; i < item_type; i++)
	{
		distance[i] = 0;
	}
}
/**
* @brief プレイヤーとアイテムの当たり判定処理
*/
void GameMain::Collsion_Update()
{
	auto it_pos2 = tubu_position_list_.begin();
	auto it_rot2 = tubu_rotation_x_list.begin();

	auto player_pos = player_->GetPosition();

	while (true)
	{
		if (it_pos2 == tubu_position_list_.end())
			break;

		distance[0] = Vector3_Distance(player_pos, *it_pos2);
		if (distance[0] >= 90)
		{
			it_pos2++;
		}
		else 
		{
			count++;
			heal_mode_ = 1;
			it_pos2 = tubu_position_list_.erase(it_pos2);
			it_rot2 = tubu_rotation_x_list.erase(it_rot2);
		}
	}

	auto it_pos = furisuku_position_list_.begin();
	auto it_rot = furisuku_rotation_x_list.begin();

	while (true)
	{
		if (it_pos == furisuku_position_list_.end())
			break;

		distance[1] = Vector3_Distance(player_pos, *it_pos);
		if (distance[1] >= 90)
		{
			it_pos++;
		}
		else
		{
			count++;
			heal_mode_ = 0;
			it_pos = furisuku_position_list_.erase(it_pos);
			it_rot = furisuku_rotation_x_list.erase(it_rot);
		}
	}
}
/**
* @brief プレイヤー初期化
*/
bool GameMain::PlayerInitialize()
{
	Viewport view = GraphicsDevice.GetViewport();

	camera_->SetView(Vector3(0.0f, 30.0f, -160.0f), Vector3(0.0f, 0.0f, 0.0f));
	camera_->SetPerspectiveFieldOfView(45.0f, (float)view.Width, (float)view.Height, 1.0f, 10000.0f);
	GraphicsDevice.SetCamera(camera_);
	player_ = GraphicsDevice.CreateAnimationModelFromFile(_T("3D/Player/player.X"));
	player_position_ = Vector3(0.0f, 0.0f, 0.0f);//プレイヤー座標
	player_->SetRotation(0.0f, -90.0f, 0.0f);
	player_->SetTrackLoopMode(0, AnimationLoopMode_Once);
	player_->SetPosition(player_position_);

	crash_ = GraphicsDevice.CreateAnimationModelFromFile(_T("3D/Player/crash.X"));
	crash_->SetRotation(0.0f, -90.0f, 0.0f);
	crash_->SetTrackLoopMode(0, AnimationLoopMode_Once);

	player_->SetTrackEnable(1,FALSE);

	starting_point_ = GraphicsDevice.CreateModelFromFile(_T("3D/仮用/battery/battery.X"));
	point_pos_ = Vector3(0.0f,0.0f,0.0f);

	result_ = GraphicsDevice.CreateSpriteFromFile(_T("UI/Result.png"));

	medium_font_ = GraphicsDevice.CreateSpriteFont(_T("メイリオ"), 25);
	big_font_    = GraphicsDevice.CreateSpriteFont(_T("メイリオ"), 50);
	start_font   = GraphicsDevice.CreateSpriteFont(_T("Metrophobic"), 100);
	
	move_ju_fa_gr_state_ = NORMAL;

	fall_time_ = 0;
	jump_time_ = 0;
	gravity_y_ = 0;
	jump_speed_ = 60;
	jump_a_time_ = 0;
	speed_limit_ = 1;
	anim_time_ = 0.0f;

	cam_pos_ = 0;

	slow_gauge_ = MAX;
	slow_mode_ = false;
	slow_flag_ = false;

	player_f_ = 0;
	player_r_ = 0;
	player_l_ = 0;

	move_speed_x_ = 0;
	move_speed_z_ = 0;

	slow_count_ = LOW;

	heal_mode_ = 99;
	heal_count_ = 0;
	hp_heal_time = 0;

	fall_meters_ = 5000 * 10;
	fall_dist_ = 0;
	old_fall = 0;

	next_meters_ = 100 * 10;
	next_meters_model_ = nullptr;
	SetNextMetersModel(next_meters_);
	next_meters_model_->SetPosition(-1500.0f,-5000.0f * 10, -700.0f);

	Material mate(next_meters_model_);
	mate.Emissive = Color(0.0f, 0.5f, 1.0f);
	next_meters_model_->SetMaterial(mate);
	next_meters_model_->SetRotation(90, -90, 0);

	select_bar = GraphicsDevice.CreateSpriteFromFile(_T("UI/bar.png"));
	bar_pos_y = 390;
	bar_state = false;

	listener = SoundDevice.GetListener();

	return true;
}

/**
 * @brief プレイヤーのアップデート処理
 * @param [in] map マップ(地形判定に使用)
 * @return なし
 */
void GameMain::PlayerUpdate(Map* map)
{
	for (int i = 0; i < map_type; i++)
	{
		map_model[i] = map[i].GetTerrain();
	}

	player_position_ = player_->GetPosition();

	float dist_ = FLT_MAX;

	key_state_  = Keyboard->GetState();

	for (int i = 1; i < DIST_MAX; i++)
	{
		if (player_position_.y <= (-5000.0f * i * 10) -250 && next_meters_ <= 100 * i * 10)
		{
			next_meters_ += 100 * 10;
			SetNextMetersModel(next_meters_);
			fall_meters_ += 5000 * 10;
			next_meters_model_->SetPosition(-1500.0f, -fall_meters_, -700.0f);

			Material mate(next_meters_model_);
			mate.Emissive = Color(0.0f, 0.5f, 1.0f);
			next_meters_model_->SetMaterial(mate);
			next_meters_model_->SetRotation(90, -90, 0);
		}
	}

	if (!gameover_flag_)
	{
		PlayerMove();
	}

	HPGauge(heal_mode_);

	if(key_buffer_.IsPressed(Keys_D1))
	{
		heal_mode_ = 0;
	}

	//落下、ジャンプ、通常の状態の習得、切り替え------------------------------------------------------------
	float min_dist = FLT_MAX;
	for (auto& hasira_pos : hasira_position_list_)
	{
		dist_ = FLT_MAX;
		map_model[0]->SetPosition(hasira_pos);
		map_model[0]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Down, &dist_);
		if (min_dist > dist_)
		{
			min_dist = dist_;
		}
	}

	for (auto& hasira2_pos : hasira2_position_list_)
	{
		dist_ = FLT_MAX;
		map_model[1]->SetPosition(hasira2_pos);
		map_model[1]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Down, &dist_);
		if (min_dist > dist_)
		{
			min_dist = dist_;
		}
	}

	for (auto& ana_pos : ana_position_list_)
	{
		dist_ = FLT_MAX;
		map_model[2]->SetPosition(ana_pos);
		map_model[2]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Down, &dist_);
		if (min_dist > dist_)
		{
			min_dist = dist_;
		}
	}

	for (auto& hasira_yoko_pos : hasira_yoko_position_list_)
	{
		dist_ = FLT_MAX;
		map_model[3]->SetPosition(hasira_yoko_pos);
		map_model[3]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Down, &dist_);
		if (min_dist > dist_)
		{
			min_dist = dist_;
		}
	}

	
	if (key_buffer_.IsPressed(Keys_LeftShift)
		|| pad_buffer_.IsPressed(GamePad_Button1)
		|| pad_buffer_.IsPressed(GamePad_Button2)
		|| pad_buffer_.IsPressed(GamePad_Button3)
		|| pad_buffer_.IsPressed(GamePad_Button4)
		|| pad_buffer_.IsPressed(GamePad_Button5)
		|| pad_buffer_.IsPressed(GamePad_Button6)
		|| pad_buffer_.IsPressed(GamePad_Button7))
	{
		slow_sound_->PlayLooping();
	}

	if (sen_state == 0 && cam_pos_ < 0.0)
	{
		if (key_buffer_.IsReleased(Keys_LeftShift)
			|| pad_buffer_.IsReleased(GamePad_Button1)
			|| pad_buffer_.IsReleased(GamePad_Button2)
			|| pad_buffer_.IsReleased(GamePad_Button3)
			|| pad_buffer_.IsReleased(GamePad_Button4)
			|| pad_buffer_.IsReleased(GamePad_Button5)
			|| pad_buffer_.IsReleased(GamePad_Button6)
			|| pad_buffer_.IsReleased(GamePad_Button7))
		{
			slow_sound_->Stop();
			sen_state = 1;
			sen_time = 0;
			camera_pos = camera_->GetPosition();

			blur_depth = 0.0f;

			blur->SetParameter("AddU", (float)(1.0 / 1280.0f) * 3.0f + blur_depth);
			blur->SetParameter("AddV", (float)(1.0 /  720.0f) * 3.0f + blur_depth);

			for (int i = 0; i < SPEEDLINE_MAX; i++)
			{
				mtrl[i].Emissive = Color(MathHelper_Random(128, 255), MathHelper_Random(128, 255), MathHelper_Random(128, 255));
				sen_pos[i] = Vector3(camera_pos.x, camera_pos.y - 10000.0f, camera_pos.z);
				sen_direction[i] = Vector3(MathHelper_Random(-100, 100) / 100.0f, 3, MathHelper_Random(-100, 100) / 100.0f);
				sen_direction[i] = Vector3_Normalize(sen_direction[i]) * 200;
			}
		}
	}

	if (key_state_.IsKeyDown(Keys_LeftShift) && slow_count_ >= LOW ||
		          pad_state_.Buttons[0] != 0 && slow_count_ >= LOW ||
		          pad_state_.Buttons[1] != 0 && slow_count_ >= LOW ||
	              pad_state_.Buttons[2] != 0 && slow_count_ >= LOW ||
	              pad_state_.Buttons[3] != 0 && slow_count_ >= LOW ||
		          pad_state_.Buttons[4] != 0 && slow_count_ >= LOW ||
		          pad_state_.Buttons[5] != 0 && slow_count_ >= LOW ||
		          pad_state_.Buttons[6] != 0 && slow_count_ >= LOW)
	{
		slow_flag_ = true;
	}
	else
	{
		slow_sound_->Stop();
		slow_flag_ = false;
	}

	if(slow_flag_)
	{
		speed_limit_ = 7;

		player_f_ -= 2;

		if (cam_pos_ > 0.0f)
		{
			cam_pos_ -= 6;
		}
		else if (cam_pos_ <= 0.0f)
		{
			cam_pos_ += 0.01f;
		}

		anim_time_ -= 0.06f;
		if (anim_time_ < 0)
		{
			anim_time_ = 0;
		}
		player_->SetTrackPosition(0, anim_time_);
	}
	else if(!slow_flag_)
	{
		speed_limit_ = 1;
		cam_pos_ += 2;

		player_f_++;

		anim_time_ += 0.1f;
		if (anim_time_ > 0.8f)
		{
			anim_time_ = 0.8f;
		}
		player_->SetTrackPosition(0, anim_time_);
	}

	SlowGaugeStatus();

	//プレイヤーの状態
	if (move_ju_fa_gr_state_ == NORMAL)
	{
	
		jump_a_time_ = 0;

		if (min_dist >= 2)
		{
			move_ju_fa_gr_state_ = FALL;
		}

		if (key_buffer_.IsPressed(Keys_Space))
		{
			jump_time_ = 0;
			move_ju_fa_gr_state_ = JUMP;
			jump_position_ = player_->GetPosition();
		}
	}
	else if (move_ju_fa_gr_state_ != NORMAL)
	{
		if (move_speed_x_ >= 7.0f)
		{
			move_speed_x_ = 7.0f;
		}
		if (move_speed_x_ <= -7.0f)
		{
			move_speed_x_ = -7.0f;
		}

		if (move_speed_z_ >= 7.0f)
		{
			move_speed_z_ = 7.0f;
		}
		if (move_speed_z_ <= -7.0f)
		{
			move_speed_z_ = -7.0f;
		}
	}

	if (move_ju_fa_gr_state_ == FALL)
	{
		if (!gameover_flag_)
		{
			FreeFall();
		}
		
	}
	
	if (move_ju_fa_gr_state_ == JUMP)
	{
		PlayerJump();
	}

	if(player_f_ <= 0.0f)
	{
		player_f_ = 0.0f;
	}
	else if(player_f_ >= 45.0f)
	{
		player_f_ = 45.0f;
	}

	if (player_r_ <= 0.0f)
	{
		player_r_ = 0.0f;
	}
	else if (player_r_ >= 45.0f)
	{
		player_r_ = 45.0f;
	}

	if (gameover_flag_ == false)
	{
		old_fall = fall_dist_;
		fall_dist_ = Vector3_Distance(player_->GetPosition(), point_pos_);
	}

	crash_->SetPosition(player_->GetPosition());
}

/**
 * @brief プレイヤー3D用描画処理
 */
void GameMain::PlayerDraw()
{
	if (main_flag)
	{
		if (!gameover_flag_)
		{
			player_->Draw();
		}
		player_->AdvanceTime(0.01);
		next_meters_model_->Draw();

		if (gameover_flag_)
		{
			crash_->Draw();
			crash_->AdvanceTime(0.1);
		}
	}
}

/**
 * @brief UIの描画処理
 */
void GameMain::PlayerUi()
{
	//到達深度ランキング
	auto DrawRankString = [&](int score_index)
	{
		const int STING_Y = 400.0f + score_index * 100;
		SpriteBatch.DrawString(start_font, Vector2(900.0f, STING_Y - 2), Color(  0.0f, 0.0f, 0.0f), _T("%dm"), rank_score[score_index]);
		SpriteBatch.DrawString(start_font, Vector2(900.0f, STING_Y + 2), Color(  0.0f, 0.0f, 0.0f), _T("%dm"), rank_score[score_index]);
		SpriteBatch.DrawString(start_font, Vector2(900.0f - 2, STING_Y), Color(  0.0f, 0.0f, 0.0f), _T("%dm"), rank_score[score_index]);
		SpriteBatch.DrawString(start_font, Vector2(900.0f + 2, STING_Y), Color(  0.0f, 0.0f, 0.0f), _T("%dm"), rank_score[score_index]);
		SpriteBatch.DrawString(start_font, Vector2(900.0f,     STING_Y), Color(255.0f, 0.0f, 0.0f), _T("%dm"), rank_score[score_index]);
	};

	if (gameover_flag_ == true)
	{
		SpriteBatch.Draw(*result_, Vector3(0.0f, 0.0f, -10000.0f));

		//自己スコア		
		SpriteBatch.DrawString(start_font, Vector2(900.0f, 300.0f - 2), Color(0.0f, 0.0f, 0.0f), _T("%dm"), (int)fall_dist_ / 50);
		SpriteBatch.DrawString(start_font, Vector2(900.0f, 300.0f + 2), Color(0.0f, 0.0f, 0.0f), _T("%dm"), (int)fall_dist_ / 50);
		SpriteBatch.DrawString(start_font, Vector2(900.0f - 2, 300.0f), Color(0.0f, 0.0f, 0.0f), _T("%dm"), (int)fall_dist_ / 50);
		SpriteBatch.DrawString(start_font, Vector2(900.0f + 2, 300.0f), Color(0.0f, 0.0f, 0.0f), _T("%dm"), (int)fall_dist_ / 50);
		SpriteBatch.DrawString(start_font, Vector2(900.0f, 300.0f), Color(255.0f, 0.0f, 0.0f), _T("%dm"), (int)fall_dist_ / 50);

		DrawRankString(0);
		DrawRankString(1);
		DrawRankString(2);

		if (bar_state == true)
		{
			SpriteBatch.Draw(*select_bar, Vector3(60, bar_pos_y, 0));
		}
	}
		
	if (title_flag == true)
	{
		SpriteBatch.Draw(*title, Vector3(0.0f, 0.0f, -10000.0f));
		DrawRankString(0);
		DrawRankString(1);
		DrawRankString(2);
	}
	else if (main_flag)
	{
	SpriteBatch.DrawString(big_font_, Vector2(620.0f, 0.0f - 2), Color(0.0f, 0.0f, 0.0f), _T("%dm"), (int)fall_dist_ / 50);
	SpriteBatch.DrawString(big_font_, Vector2(620.0f, 0.0f + 2), Color(0.0f, 0.0f, 0.0f), _T("%dm"), (int)fall_dist_ / 50);
	SpriteBatch.DrawString(big_font_, Vector2(620.0f - 2, 0.0f), Color(0.0f, 0.0f, 0.0f), _T("%dm"), (int)fall_dist_ / 50);
	SpriteBatch.DrawString(big_font_, Vector2(620.0f + 2, 0.0f), Color(0.0f, 0.0f, 0.0f), _T("%dm"), (int)fall_dist_ / 50);
	SpriteBatch.DrawString(big_font_, Vector2(620.0f, 0.0f), Color(0.0f, 255.0f, 0.0f), _T("%dm"), (int)fall_dist_ / 50);
	}

}

/**
 * @brief プレイヤーの移動処理
 */
void GameMain::PlayerMove()
{
float d, u, r, l;
float min_d, min_u, min_r, min_l;

	min_d = FLT_MAX;
	for (auto& map_pos : hasira_position_list_)
	{
		d = -1;
		map_model[0]->SetPosition(map_pos);
		map_model[0]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Right, &d);
		if (d >= 0)
		{
			if (min_d > d)
			{
				min_d = d;
			}
		}
	}
	for (auto& map_pos : hasira2_position_list_)
	{
		d = -1;
		map_model[1]->SetPosition(map_pos);
		map_model[1]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Right, &d);
		if (d >= 0)
		{
			if (min_d > d)
			{
				min_d = d;
			}
		}
	}
	for (auto& map_pos : ana_position_list_)
	{
		d = -1;
		map_model[2]->SetPosition(map_pos);
		map_model[2]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Right, &d);
		if (d >= 0)
		{
			if (min_d > d)
			{
				min_d = d;
			}
		}
	}
	for (auto& map_pos : hasira_yoko_position_list_)
	{
		d = -1;
		map_model[3]->SetPosition(map_pos);
		map_model[3]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Right, &d);
		if (d >= 0)
		{
			if (min_d > d)
			{
				min_d = d;
			}
		}
	}

	//------------------------------------------------------------------------------------

	min_u = FLT_MAX;
	for (auto& map_pos : hasira_position_list_)
	{
		u = -1;
		map_model[0]->SetPosition(map_pos);
		map_model[0]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Left, &u);
		if (u >= 0)
		{
			if (min_u > u)
			{
				min_u = u;
			}
		}
	}
	for (auto& map_pos : hasira2_position_list_)
	{
		u = -1;
		map_model[1]->SetPosition(map_pos);
		map_model[1]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Left, &u);
		if (u >= 0)
		{
			if (min_u > u)
			{
				min_u = u;
			}
		}
	}
	for (auto& map_pos : ana_position_list_)
	{
		u = -1;
		map_model[2]->SetPosition(map_pos);
		map_model[2]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Left, &u);
		if (u >= 0)
		{
			if (min_u > u)
			{
				min_u = u;
			}
		}
	}
	for (auto& map_pos : hasira_yoko_position_list_)
	{
		u = -1;
		map_model[3]->SetPosition(map_pos);
		map_model[3]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Left, &u);
		if (u >= 0)
		{
			if (min_u > u)
			{
				min_u = u;
			}
		}
	}

	//-----------------------------------------------------------------------------------

	min_r = FLT_MAX;
	for (auto& map_pos : hasira_position_list_)
	{
		r = -1;
		map_model[0]->SetPosition(map_pos);
		map_model[0]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 10.0f), Vector3_Forward, &r);
		if (r >= 0)
		{
			if (min_r > r)
			{
				min_r = r;
			}
		}
	}
	for (auto& map_pos : hasira2_position_list_)
	{
		r = -1;
		map_model[1]->SetPosition(map_pos);
		map_model[1]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Forward, &d);
		if (d >= 0)
		{
			if (min_d > d)
			{
				min_d = d;
			}
		}
	}
	for (auto& map_pos : ana_position_list_)
	{
		r = -1;
		map_model[2]->SetPosition(map_pos);
		map_model[2]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 10.0f), Vector3_Forward, &r);
		if (r >= 0)
		{
			if (min_r > r)
			{
				min_r = r;
			}
		}
	}
	for (auto& map_pos : hasira_yoko_position_list_)
	{
		r = -1;
		map_model[3]->SetPosition(map_pos);
		map_model[3]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 10.0f), Vector3_Forward, &r);
		if (r >= 0)
		{
			if (min_r > r)
			{
				min_r = r;
			}
		}
	}

	//-----------------------------------------------------------------------------------

	min_l = FLT_MAX;
	for (auto& map_pos : hasira_position_list_)
	{
		l = -1;
		map_model[0]->SetPosition(map_pos);
		map_model[0]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 10.0f), Vector3_Backward, &l);
		if (l >= 0)
		{
			if (min_l > l)
			{
				min_l = l;
			}
		}
	}
	for (auto& map_pos : hasira2_position_list_)
	{
		l = -1;
		map_model[1]->SetPosition(map_pos);
		map_model[1]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Backward, &d);
		if (d >= 0)
		{
			if (min_d > d)
			{
				min_d = d;
			}
		}
	}
	for (auto& map_pos : ana_position_list_)
	{
		l = -1;
		map_model[2]->SetPosition(map_pos);
		map_model[2]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Backward, &l);
		if (l >= 0)
		{
			if (min_l > l)
			{
				min_l = l;
			}
		}
	}
	for (auto& map_pos : hasira_yoko_position_list_)
	{
		l = -1;
		map_model[3]->SetPosition(map_pos);
		map_model[3]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Backward, &l);
		if (l >= 0)
		{
			if (min_l > l)
			{
				min_l = l;
			}
		}
	}

	//-----------------------------------------------------------------------------------

	Vector3 pos = player_->GetPosition();


	if (key_state_.IsKeyDown(Keys_Down) || pad_state_.Y > Axis_Center)
	{
		if (min_d <= 11)
		{
			GameOver();
		}
		else
		{
			move_speed_z_ -= 0.5;
			player_->Move(0.0f, 0.0f, move_speed_z_);
		}
	}
	else if (key_state_.IsKeyDown(Keys_Up) || pad_state_.Y < Axis_Center)
	{
		if (min_u <= 11)
		{
			GameOver();
		}
		else
		{
			move_speed_z_ += 0.5;
			player_->Move(0.0f, 0.0f, move_speed_z_);
		}
	}
	else if (key_state_.IsKeyDown(Keys_Right) || pad_state_.X > Axis_Center)
	{
		if (min_r <= 11)
		{
			GameOver();
		}
		else
		{
			move_speed_x_ += 0.5;
			player_->Move(move_speed_x_, 0.0f, 0.0f);

			wind_x_ += 0.01f;
			if (wind_x_ > 1.5f)
			{
				wind_x_ = 1.5f;
			}
		}
		player_r_++;
	}
	else if (key_state_.IsKeyDown(Keys_Left) || pad_state_.X < Axis_Center)
	{
		if (min_l <= 11)
		{
			GameOver();
		}
		else
		{
			move_speed_x_ -= 0.5;
			player_->Move(move_speed_x_, 0.0f, 0.0f);

			wind_x_ -= 0.01f;
			if (wind_x_ < -1.5f)
			{
				wind_x_ = -1.5f;
			}
		}
		player_l_++;
	}
	else
	{

		if (wind_x_ > 0.0f)
		{
			wind_x_ -= 0.05f;
			if (wind_x_ <= 0.0f)
			{
				wind_x_ = 0.0f;
			}
		}
		else if (wind_x_ < 0.0f)
		{
			wind_x_ += 0.1f;
			if (wind_x_ >= 0.0f)
			{
				wind_x_ = 0.0f;
			}
		}
	}

	if (key_state_.IsKeyUp(Keys_Right) && key_state_.IsKeyUp(Keys_Left) && pad_state_.X == Axis_Center) 
	{
		//TODO:x

		if (move_speed_x_ > 0)
		{
			move_speed_x_ -= 0.05;
			if (move_speed_x_ < 0)
			{
				move_speed_x_ = 0;
			}
		}

		else if (move_speed_x_ < 0)
		{
			move_speed_x_ += 0.05;
			if (move_speed_x_ > 0)
			{
				move_speed_x_ = 0;
			}
		}

		player_->Move(move_speed_x_, 0.0f, 0.0f);
	}
	if (key_state_.IsKeyUp(Keys_Up) && key_state_.IsKeyUp(Keys_Down) && pad_state_.Y == Axis_Center)
	{
		//TODO:z
		if (move_speed_z_ > 0)
		{
			move_speed_z_ -= 0.5;
			if (move_speed_z_ <= 0)
			{
				move_speed_z_ = 0;
			}
		}

		if (move_speed_z_ < 0)
		{
			move_speed_z_ += 0.5;
			if (move_speed_z_ >= 0)
			{
				move_speed_z_ = 0;
			}
		}

		player_->Move(0.0f, 0.0f, move_speed_z_);
	}

	//サウンドの座標
	wind_3D_->SetPosition(wind_x_, 0, 0);
}

/**
 * @brief 自由落下の処理
 */
void GameMain::FreeFall()
{
	fall_time_ = 100;
	fall_time_++;

	float time = fall_time_ / 300.0f;

	gravity_y_ -= 4.9 * (time * time);

	if (gravity_y_ <= -200.0f)
	{
		gravity_y_ = -200.0f;
	}

	player_->Move(0.0f, gravity_y_ / speed_limit_, 0.0f);

	fall_min_dist_ = FLT_MAX;
	for (auto& map_pos : hasira_position_list_)
	{
		height_down_ = -1;
		map_model[0]->SetPosition(map_pos);
		map_model[0]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Down, &height_down_);
		if (height_down_ >= 0)
		{
			if (fall_min_dist_ > height_down_)
			{
				fall_min_dist_ = height_down_;
			}
		}
	}
	for (auto& map_pos : hasira2_position_list_)
	{
		height_down_ = -1;
		map_model[1]->SetPosition(map_pos);
		map_model[1]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Down, &height_down_);
		if (height_down_ >= 0)
		{
			if (fall_min_dist_ > height_down_)
			{
				fall_min_dist_ = height_down_;
			}
		}
	}
	for (auto& map_pos : ana_position_list_)
	{
		height_down_ = -1;
		map_model[2]->SetPosition(map_pos);
		map_model[2]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Down, &height_down_);
		if (height_down_ >= 0)
		{
			if (fall_min_dist_ > height_down_)
			{
				fall_min_dist_ = height_down_;
			}
		}
	}
	for (auto& map_pos : hasira_yoko_position_list_)
	{
		height_down_ = -1;
		map_model[3]->SetPosition(map_pos);
		map_model[3]->IntersectRay(player_->GetPosition() + Vector3(0.0f, 0.0f, 0.0f), Vector3_Down, &height_down_);
		if (height_down_ >= 0)
		{
			if (fall_min_dist_ > height_down_)
			{
				fall_min_dist_ = height_down_;
			}
		}
	}

	aa_fall = height_down_;

	if(fall_min_dist_ <= 150)
	{
		FreeFallEnd();
		GameOver();
	}

	if (fall_time_ > 200.0f)
	{
		if (fall_min_dist_ > 0.0f && fall_min_dist_ < 400.0f)
		{
			FreeFallEnd();
			GameOver();
		}
	}
	else if (fall_time_ > 150.0f)
	{
		if (fall_min_dist_ > 0.0f && fall_min_dist_ < 350.0f)
		{
			FreeFallEnd();
			GameOver();
		}
	}
	else if (fall_time_ >= 100.0f)
	{
		if (fall_min_dist_ > 0.0f && fall_min_dist_ < 300.0f)
		{
			FreeFallEnd();
			GameOver();
		}
	}
	else if (fall_time_ >= 50.0f)
	{
		if (fall_min_dist_ > 0.0f && fall_min_dist_ < 200.0f)
		{
			FreeFallEnd();
			GameOver();
		}
	}
}

/**
 * @brief ジャンプの処理
 */
void GameMain::PlayerJump()
{
	jump_time_++;
	float time_j = jump_time_ / 8.0f;

	Vector3   position = player_->GetPosition();

	player_->SetPosition(position.x, jump_position_.y + jump_speed_ * MathHelper_Sin(60) * time_j - 4.9 * (time_j * time_j),
		position.z);

	jump_a_time_ += 0.01f;

	if (jump_a_time_ >= 1.0f)
	{
		jump_a_time_ = 1.0f;
	}

	height_ = -1;

	for (auto& map_pos : hasira2_position_list_)
	{
		map_model[0]->SetPosition(map_pos);
		map_model[0]->IntersectRay(player_->GetPosition(), Vector3_Up, &height_);
	}
	for (auto& map_pos : ana_position_list_)
	{
		map_model[1]->SetPosition(map_pos);
		map_model[1]->IntersectRay(player_->GetPosition(), Vector3_Up, &height_);
	}

	aa_jump = height_;

	if (jump_time_ > 400.0f)
	{
		if (height_ > 0.0f)
		{
			JumpEnd();
		}
	}
	else if (jump_time_ > 100.0f)
	{
		if (height_ > 0.0f && height_ < 30.0f)
		{
			JumpEnd();
		}
	}
	else if (jump_time_ >= 50.0f)
	{
		if (height_ > 0.0f && height_ < 30.0f)
		{
			JumpEnd();
		}
	}
}

void GameMain::SlowGaugeStatus()
{
	//TODO:スローゲージの数値管理(消費、回復、使用可(不可)の判定(？)、増減(？)etc.)
	if (gauge > 450)
	{
		gauge = 450;
	}
	else if (gauge <= 0)
	{
		gauge = 0;
		slow_count_ = 0;
	}
	slow_count_++;

	if(slow_flag_)
	{
		gauge--;
	}
	else if(!slow_flag_)
	{
		gauge += 0.5;
	}
}
void GameMain::HPGauge(int a)
{
	//TODO:HPの数値管理(消費、回復etc.)

	if (hp_gauge < 0)
	{
		hp_gauge = 0;
		GameOver();
	}

	if (hp_gauge >= 600)
	{
		HpReset();
	}
	hp_gauge -= 1;

	if (a == 99)
	{
		;
	}
	else if (a == 0)
	{
		HpHeal();
		if (heal_count_ >= 150 / 5)
		{
			HpReset();
		}
	}
	else if (a == 1)
	{
		HpHeal();
		if (heal_count_ >= 20 / 5)
		{
			HpReset();
		}
	}
}

/**
 * @brief 三人称カメラ
 */
void GameMain::ThirdPersonCamera()
{
	if (cam_pos_ >= 50.0f)
	{
		cam_pos_ = 50.0f;
	}

	if (main_flag)
	{
		if (player_position_.y <= 10.0)
		{
			camera_->SetLookAt(player_position_ + Vector3(0.001f, 30.0f + cam_pos_, 0.0f), player_position_, Vector3_Up);
		}

	}
	else if (title_flag)
	{
		camera_->SetLookAt(Vector3_Zero + Vector3(1.0f, 30.0f, 0.0f), Vector3_Zero, Vector3_Up);
	}
}

/**
 * @brief ジャンプが終わった時の処理
 */
void GameMain::JumpEnd()
{
	player_->Move(0.0f, height_, 0.0f);
	jump_time_ = 0;
	move_ju_fa_gr_state_ = NORMAL;
}

/**
 * @brief 自由落下が終わった時の処理
 */
void GameMain::FreeFallEnd()
{
	/*player_->Move(0.0f, fall_min_dist_, 0.0f);*/
	player_->Move(0.0f, 0.0f, 0.0f);
	fall_time_ = 0;
	gravity_y_ = 0;
	move_ju_fa_gr_state_ = NORMAL;
	speed_limit_ = 0;
}


/**
* @brief HP回復の処理
*/
void GameMain::HpHeal() {
	hp_heal_time++;
	if (hp_heal_time >= 0.5) //回復速度
	{
		heal_count_++;      //カウント(回復量)
		hp_heal_time = 0;
		hp_gauge += 5;        //回復(HP)
	}
}

/**
* @brief HP回復初期化
*/
void  GameMain::HpReset() 
{
	heal_count_ = 0;
	heal_mode_ = 99;
}

void GameMain::SetNextMetersModel(const int next_meters)
{
	GraphicsDevice.ReleaseModel(next_meters_model_);

	TCHAR   nextmeters_text[32];
	::wsprintf(nextmeters_text, _T("%dm"), next_meters);

	next_meters_model_ = GraphicsDevice.CreateModelFromText(nextmeters_text, _T("メイリオ"), 300, 2.5f);
	
	next_meters_model_->SetScale(50 * 5);
}

/**
* @brief ゲームオーバーの処理
*/
void GameMain::GameOver()
{

	blur->SetParameter("AddU", (float)(1.0 / 1280) * 0.0f);
	blur->SetParameter("AddV", (float)(1.0 / 720.0) * 0.0f);

	if(gameover_flag_)
		return;

	crash_->SetTrackEnable(0, TRUE);

	gameover_flag_ = true;
	//死んだらスコアを書き込む
	rank_score[3] = fall_dist_ / 50;
	for (int i = 2; i >= 0; i--)
	{
		if (rank_score[i] < rank_score[i + 1]) {
			int w = rank_score[i];
			rank_score[i] = rank_score[i + 1];
			rank_score[i + 1] = w;
		}
		else
		{
			break;
		}
	}
	write_rank.Open(_T("txt/rank.txt"));
	for (int i = 0; i < 3; i++) {
		write_rank.WriteInt32(rank_score[i]);
		write_rank.WriteChar(_T('\n'));
	}
	write_rank.Close();
	
	//セレクトバー表示
	bar_state = true;

	wind_sound_->Stop();
}

