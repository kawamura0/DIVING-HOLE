/**
* @file Player.cpp
* @brief �v���C���[�N���X�\�[�X�t�@�C��
* @author ��� �O�M(�q���[�}���A�J�f�~�[���Z)
* @date 2019/3/5
*/
#include "Player.h"

/**
* @brief �v���C���[�����̏�����
* @return ���� �c true / ���s �c false
*/

bool Player::Initialize()
{
	// TODO:�v���C���[������

	Viewport view = GraphicsDevice.GetViewport();

	camera_->SetView(Vector3(0.0f, 30.0f, -160.0f), Vector3(0.0f, 0.0f, 0.0f));
	camera_->SetPerspectiveFieldOfView(45.0f, (float)view.Width, (float)view.Height, 1.0f, 10000.0f);
	GraphicsDevice.SetCamera(camera_);

	player_          = GraphicsDevice.CreateAnimationModelFromFile(_T("3D/Player/Player_kari.X"));//�v���C���[
	player_position_ = Vector3(0.0f, 0.0f, 0.0f);//�v���C���[���W
	player_->SetRotation(0.0f, -90.0f, 0.0f);
	player_->SetTrackEnable(0, TRUE);
	player_->SetTrackEnable(1, FALSE);
	player_->SetTrackLoopMode(2, AnimationLoopMode_Once);
	player_->SetTrackEnable(3, FALSE);
	player_->SetPosition(player_position_);

	pad_paramater_ = 2.0f;

	medium_font_ = GraphicsDevice.CreateSpriteFont(_T("���C���I"), 25);
	big_font_    = GraphicsDevice.CreateSpriteFont(_T("���C���I"), 50);

	move_ju_fa_gr_state_ = 0;

	fall_time_   = 0;
	jump_time_   = 0;
	gravity_y_   = 0;
	jump_speed_  = 60;
    jump_a_time_ = 0;
	speed_limit_ = 1;
	move_speed_ = 1;

	cam_pos_ = 0;

	slow_mode_ = false;

	mae_usiro   = 0;

	return true;
}

/**
 * @brief �v���C���[�̃A�b�v�f�[�g����
 * @param [in] map �}�b�v(�n�`����Ɏg�p)
 * @return �Ȃ�
 * @detail 
 */
void Player::Update(Map &map)
{
	map_model_ = map.GetTerrain();

	player_position_      = player_->GetPosition();

	float outotu = FLT_MAX;
	             
	pad_state_   = GamePad(0)->GetState();
	pad_buffer_  = GamePad(0)->GetBuffer();
	key_state_   = Keyboard  ->GetState();
	key_buffer_  = Keyboard  ->GetBuffer();

	if (key_buffer_.IsPressed(Keys_NumPad0))
	{
		Initialize();
	}

	PlayerMove();

	//�����A�W�����v�A�ʏ�̏�Ԃ̏K���A�؂�ւ�------------------------------------------------------------
	map_model_->IntersectRay(player_->GetPosition() + Vector3(0.0f, 1.0f, 0.0f), Vector3_Down, &outotu);

	//TODO:�g���K�[���[�h(�X���[���[�V�������[�h)
	if (key_state_.IsKeyDown(Keys_LeftShift))
	{
		speed_limit_ = 4.5;

		if (cam_pos_ > 0.0f)
		{
			cam_pos_ -= 4;
		}
		else if (cam_pos_ <= 0.0f)
		{
			cam_pos_ += 0.01f;
		}
	}
	else
	{
		speed_limit_ = 1;
		cam_pos_ += 2;
	}
	
	if (cam_pos_ >= 80.0f)
	{
		cam_pos_ = 80.0f;
	}

	if (move_ju_fa_gr_state_ == 0)
	{
		move_speed_ = 4.0f;

		player_->SetTrackEnable(0, TRUE);
		player_->SetTrackEnable(2, FALSE);
		jump_a_time_ = 0;

		if (outotu >= 2)
		{
			move_ju_fa_gr_state_ = 1;
		}

		if (key_buffer_.IsPressed(Keys_Space))
		{
			jump_time_           = 0;
			move_ju_fa_gr_state_ = 2;
			jump_position_       = player_->GetPosition();
		}
		else if(pad_buffer_.IsPressed(GamePad_Button1))
		{
			jump_time_           = 0;
			move_ju_fa_gr_state_ = 2;
			jump_position_       = player_->GetPosition();
		}
	}
	else if(move_ju_fa_gr_state_ != 0)
	{
		move_speed_ = 2.5f;
	}

	if (move_ju_fa_gr_state_ == 1) 
	{
		FreeFall();
	}
	if (move_ju_fa_gr_state_ == 2) 
	{
		PlayerJump();
	}
	//--------------------------------------------------------------------------------------------------------
	
	ThirdPersonCamera();

	GraphicsDevice.SetCamera(camera_);
}

/**
 * @brief �v���C���[3D�p�`�揈��
 */
void Player::Draw()
{
	player_->Draw();
	player_->AdvanceTime(0.01);
}

/**
 * @brief �v���C���[2D�p�`�揈��
 */
void Player::Ui()
{
	//�����̃���
	SpriteBatch.DrawString(medium_font_, Vector2(0.0f, 25.0f), Color(255.0f, 0.0f, 0.0f), _T("jump/ %d"), (int)aa_jump);
	SpriteBatch.DrawString(medium_font_, Vector2(0.0f, 50.0f), Color(0.0f, 255.0f, 0.0f), _T("fall/ %d"), (int)aa_fall);
	SpriteBatch.DrawString(medium_font_, Vector2(0.0f, 75.0f), Color(255.0f, 255.0f, 0.0f),
		                   _T("sped/ %d"), (int)gravity_y_);
}

/**
 * @brief �v���C���[�̈ړ�����
 */
void Player::PlayerMove()
{
	if (key_state_.IsKeyDown(Keys_Down))
	{
		mae_usiro = 1;
		player_->SetRotation(0.0f, 90.0f, 0.0f);
		player_->Move(0.0f, 0.0f, move_speed_);
		player_->SetTrackEnable(0, FALSE);
		player_->SetTrackEnable(1, TRUE);
	}
	else if (key_state_.IsKeyDown(Keys_Up))
	{
		mae_usiro = 0;
		player_->SetRotation(0.0f, -90.0f, 0.0f);
		player_->Move(0.0f, 0.0f, move_speed_);
		player_->SetTrackEnable(0, FALSE);
		player_->SetTrackEnable(1, TRUE);
	}
	else
	{
		player_->SetTrackEnable(0, TRUE);
		player_->SetTrackEnable(1, FALSE);
	}

	if (mae_usiro == 0)
	{
		if (key_state_.IsKeyDown(Keys_Right))
		{
			player_->Move(move_speed_, 0.0f, 0.0f);

			player_->SetTrackEnable(0, FALSE);
			player_->SetTrackEnable(1, TRUE);
		}
		else if (key_state_.IsKeyDown(Keys_Left))
		{
			player_->Move(-move_speed_, 0.0f, 0.0f);

			player_->SetTrackEnable(0, FALSE);
			player_->SetTrackEnable(1, TRUE);
		}
	}
	else if (mae_usiro == 1)
	{
		if (key_state_.IsKeyDown(Keys_Right))
		{
			player_->Move(-move_speed_, 0.0f, 0.0f);

			player_->SetTrackEnable(0, FALSE);
			player_->SetTrackEnable(1, TRUE);
		}
		else if (key_state_.IsKeyDown(Keys_Left))
		{
			player_->Move(move_speed_, 0.0f, 0.0f);

			player_->SetTrackEnable(0, FALSE);
			player_->SetTrackEnable(1, TRUE);
		}
	}
	else
	{
		player_->SetTrackEnable(0, TRUE);
		player_->SetTrackEnable(1, FALSE);
	}
}
/**
 * @brief ���R�����̏���
 */
void Player::FreeFall()
{
	fall_time_ = 50;
	fall_time_++;

	float time = fall_time_ / 300.0f;

	gravity_y_ -= 4.9 * (time * time);

	if(gravity_y_ <= -30.0f)
	{
		gravity_y_ = -30.0f;
	}

	player_->Move(0.0f, gravity_y_ / speed_limit_, 0.0f);
	
	player_->SetTrackEnable(0, FALSE);
	player_->SetTrackEnable(1, FALSE);
	player_->SetTrackEnable(2, TRUE);

	height_down_ = -1;

	map_model_->IntersectRay(player_->GetPosition(), Vector3_Up, &height_down_); //����

	aa_fall = height_down_;
	
	if (fall_time_ > 400.0f)
	{
        if (height_down_ > 0.0f)
		{
		    FreeFallEnd();
		}
	}
	else if (fall_time_ > 100.0f)
	{
		if (height_down_ > 0.0f && height_down_ < 100.0f)
		{
			FreeFallEnd();
		}
	}
	else if (fall_time_ >= 50.0f)
	{
		if (height_down_ > 0.0f && height_down_ < 100.0f)
		{
			FreeFallEnd();
		}
	}
}

/**
 * @brief �W�����v�̏���
 */
void Player::PlayerJump()
{
	jump_time_++;
	float time_j = jump_time_ / 8.0f;

	Vector3   position = player_->GetPosition();

	player_->SetPosition(position.x,jump_position_.y + jump_speed_ * MathHelper_Sin(60) * time_j - 4.9 * (time_j * time_j),
		                 position.z);

	jump_a_time_ += 0.01f;

    if (jump_a_time_ >= 1.0f)
	{
		jump_a_time_ = 1.0f;
	}
	player_->SetTrackEnable(0, FALSE);
	player_->SetTrackEnable(1, FALSE);
	player_->SetTrackEnable(2, TRUE);
    player_->SetTrackPosition(2, jump_a_time_);

	height_ = -1;
	
	map_model_->IntersectRay(player_->GetPosition(), Vector3_Up, &height_);//����
	
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
/**
 * @brief �O�l�̃J����
 */
void Player::ThirdPersonCamera()
{
	Vector3   player_foward = player_->GetFrontVector();
	camera_->SetLookAt(player_position_ + Vector3(1.0f, 150.0f + cam_pos_, 0.0f),player_position_,Vector3_Up);
}

/**
 * @brief �W�����v���I��������̏���
 */
void Player::JumpEnd()
{
	player_->Move(0.0f, height_, 0.0f);
	jump_time_           = 0;
	move_ju_fa_gr_state_ = 0;
}

/**
 * @brief ���R�������I��������̏���
 */
void Player::FreeFallEnd()
{
	player_->Move(0.0f, height_down_, 0.0f);
	fall_time_           = 0;
	gravity_y_           = 0;
	move_ju_fa_gr_state_ = 0;
}