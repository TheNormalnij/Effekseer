﻿#include "Effekseer.EffectNodeSprite.h"

#include "Effekseer.Effect.h"
#include "Effekseer.EffectImplemented.h"
#include "Effekseer.EffectNode.h"
#include "Effekseer.Manager.h"
#include "Effekseer.Vector3D.h"
#include "SIMD/Utils.h"

#include "Effekseer.Instance.h"
#include "Effekseer.InstanceContainer.h"
#include "Effekseer.InstanceGlobal.h"

#include "Renderer/Effekseer.SpriteRenderer.h"

#include "Effekseer.Setting.h"

namespace Effekseer
{

void EffectNodeSprite::LoadRendererParameter(unsigned char*& pos, const SettingRef& setting)
{
	EffectNodeType type = EffectNodeType::NoneType;
	memcpy(&type, pos, sizeof(int));
	pos += sizeof(int);
	assert(type == GetType());
	EffekseerPrintDebug("Renderer : Sprite\n");

	auto ef = (EffectImplemented*)m_effect;

	memcpy(&RenderingOrder, pos, sizeof(int));
	pos += sizeof(int);

	if (m_effect->GetVersion() >= 3)
	{
		AlphaBlend = RendererCommon.AlphaBlend;
	}
	else
	{
		memcpy(&AlphaBlend, pos, sizeof(int));
		pos += sizeof(int);
		RendererCommon.AlphaBlend = AlphaBlend;
		RendererCommon.BasicParameter.AlphaBlend = AlphaBlend;
	}

	memcpy(&Billboard, pos, sizeof(int));
	pos += sizeof(int);

	SpriteAllColor.load(pos, m_effect->GetVersion());
	EffekseerPrintDebug("SpriteColorAllType : %d\n", SpriteAllColor.type);

	memcpy(&SpriteColor.type, pos, sizeof(int));
	pos += sizeof(int);
	EffekseerPrintDebug("SpriteColorType : %d\n", SpriteColor.type);

	if (SpriteColor.type == SpriteColor.Default)
	{
	}
	else if (SpriteColor.type == SpriteColor.Fixed)
	{
		memcpy(&SpriteColor.fixed, pos, sizeof(SpriteColor.fixed));
		pos += sizeof(SpriteColor.fixed);
	}

	memcpy(&SpritePosition.type, pos, sizeof(int));
	pos += sizeof(int);
	EffekseerPrintDebug("SpritePosition : %d\n", SpritePosition.type);

	if (SpritePosition.type == SpritePosition.Default)
	{
		if (m_effect->GetVersion() >= 8)
		{
			std::array<Vector2D, 4> fixed;
			memcpy(fixed.data(), pos, sizeof(Vector2D) * 4);

			// This code causes bugs on asmjs
			// const Vector2D* fixed = (const Vector2D*)pos;
			SpritePosition.fixed.ll = fixed[0];
			SpritePosition.fixed.lr = fixed[1];
			SpritePosition.fixed.ul = fixed[2];
			SpritePosition.fixed.ur = fixed[3];
			pos += sizeof(Vector2D) * 4;
			SpritePosition.type = SpritePosition.Fixed;
		}
		else
		{
			SpritePosition.fixed.ll = {-0.5f, -0.5f};
			SpritePosition.fixed.lr = {0.5f, -0.5f};
			SpritePosition.fixed.ul = {-0.5f, 0.5f};
			SpritePosition.fixed.ur = {0.5f, 0.5f};
			SpritePosition.type = SpritePosition.Fixed;
		}
	}
	else if (SpritePosition.type == SpritePosition.Fixed)
	{
		std::array<Vector2D, 4> fixed;
		memcpy(fixed.data(), pos, sizeof(Vector2D) * 4);

		// This code causes bugs on asmjs
		// const Vector2D* fixed = (const Vector2D*)pos;
		SpritePosition.fixed.ll = fixed[0];
		SpritePosition.fixed.lr = fixed[1];
		SpritePosition.fixed.ul = fixed[2];
		SpritePosition.fixed.ur = fixed[3];
		pos += sizeof(Vector2D) * 4;
	}

	if (m_effect->GetVersion() >= 3)
	{
	}
	else
	{
		int SpriteTexture = -1;
		memcpy(&SpriteTexture, pos, sizeof(int));
		pos += sizeof(int);
		RendererCommon.TextureIndexes[0] = SpriteTexture;
		RendererCommon.BasicParameter.TextureIndexes[0] = SpriteTexture;
	}

	if (setting->GetCoordinateSystem() == CoordinateSystem::LH)
	{
	}

	if (ef->IsDyanamicMagnificationValid())
	{
		if (SpritePosition.type == SpritePosition.Default)
		{
		}
		else if (SpritePosition.type == SpritePosition.Fixed)
		{
			SpritePosition.fixed.ll *= m_effect->GetMaginification();
			SpritePosition.fixed.lr *= m_effect->GetMaginification();
			SpritePosition.fixed.ul *= m_effect->GetMaginification();
			SpritePosition.fixed.ur *= m_effect->GetMaginification();
		}
	}
}

void EffectNodeSprite::BeginRendering(int32_t count, Manager* manager, const InstanceGlobal* global, void* userData)
{
	SpriteRendererRef renderer = manager->GetSpriteRenderer();
	if (renderer != nullptr)
	{
		nodeParam_ = GetNodeParameter(manager, global);
		renderer->BeginRendering(nodeParam_, count, userData);
	}
}

void EffectNodeSprite::Rendering(const Instance& instance, const Instance* next_instance, int index, Manager* manager, void* userData)
{
	const InstanceValues& instValues = instance.rendererValues.sprite;
	SpriteRendererRef renderer = manager->GetSpriteRenderer();
	if (renderer != nullptr)
	{
		SpriteRenderer::InstanceParameter instanceParameter;
		instanceParameter.AllColor = instValues._color;

		instanceParameter.SRTMatrix43 = instance.GetRenderedGlobalMatrix();

		// Inherit Color
		Color _color;
		if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
		{
			_color = Color::Mul(instValues._originalColor, instance.ColorParent);
		}
		else
		{
			_color = instValues._originalColor;
		}

		Color color_ll = _color;
		Color color_lr = _color;
		Color color_ul = _color;
		Color color_ur = _color;

		if (SpriteColor.type == SpriteColorParameter::Default)
		{
		}
		else if (SpriteColor.type == SpriteColorParameter::Fixed)
		{
			color_ll = Color::Mul(color_ll, SpriteColor.fixed.ll);
			color_lr = Color::Mul(color_lr, SpriteColor.fixed.lr);
			color_ul = Color::Mul(color_ul, SpriteColor.fixed.ul);
			color_ur = Color::Mul(color_ur, SpriteColor.fixed.ur);
		}

		instanceParameter.Colors[0] = color_ll;
		instanceParameter.Colors[1] = color_lr;
		instanceParameter.Colors[2] = color_ul;
		instanceParameter.Colors[3] = color_ur;

		// Apply global Color
		if (instance.m_pContainer->GetRootInstance()->IsGlobalColorSet)
		{
			instanceParameter.Colors[0] = Color::Mul(instanceParameter.Colors[0], instance.m_pContainer->GetRootInstance()->GlobalColor);
			instanceParameter.Colors[1] = Color::Mul(instanceParameter.Colors[1], instance.m_pContainer->GetRootInstance()->GlobalColor);
			instanceParameter.Colors[2] = Color::Mul(instanceParameter.Colors[2], instance.m_pContainer->GetRootInstance()->GlobalColor);
			instanceParameter.Colors[3] = Color::Mul(instanceParameter.Colors[3], instance.m_pContainer->GetRootInstance()->GlobalColor);
		}

		if (SpritePosition.type == SpritePosition.Default)
		{
			instanceParameter.Positions[0] = {-0.5f, -0.5f};
			instanceParameter.Positions[1] = {0.5f, -0.5f};
			instanceParameter.Positions[2] = {-0.5f, 0.5f};
			instanceParameter.Positions[3] = {0.5f, 0.5f};
		}
		else if (SpritePosition.type == SpritePosition.Fixed)
		{
			instanceParameter.Positions[0] = SpritePosition.fixed.ll;
			instanceParameter.Positions[1] = SpritePosition.fixed.lr;
			instanceParameter.Positions[2] = SpritePosition.fixed.ul;
			instanceParameter.Positions[3] = SpritePosition.fixed.ur;
		}

		instanceParameter.UV = instance.GetUV(0);
		instanceParameter.AlphaUV = instance.GetUV(1);
		instanceParameter.UVDistortionUV = instance.GetUV(2);
		instanceParameter.BlendUV = instance.GetUV(3);
		instanceParameter.BlendAlphaUV = instance.GetUV(4);
		instanceParameter.BlendUVDistortionUV = instance.GetUV(5);

		instanceParameter.FlipbookIndexAndNextRate = instance.GetFlipbookIndexAndNextRate();

		instanceParameter.AlphaThreshold = instance.m_AlphaThreshold;

		if (nodeParam_.EnableViewOffset)
		{
			instanceParameter.ViewOffsetDistance = instance.translation_state_.view_offset.distance;
		}

		CalcCustomData(&instance, instanceParameter.CustomData1, instanceParameter.CustomData2);

		renderer->Rendering(nodeParam_, instanceParameter, userData);
	}
}

void EffectNodeSprite::EndRendering(Manager* manager, void* userData)
{
	SpriteRendererRef renderer = manager->GetSpriteRenderer();
	if (renderer != nullptr)
	{
		renderer->EndRendering(nodeParam_, userData);
	}
}

void EffectNodeSprite::InitializeRenderedInstance(Instance& instance, InstanceGroup& instanceGroup, Manager* manager)
{
	InstanceValues& instValues = instance.rendererValues.sprite;
	IRandObject& rand = instance.GetRandObject();

	AllTypeColorFunctions::Init(instValues.allColorValues, rand, SpriteAllColor);
	instValues._originalColor = AllTypeColorFunctions::Calculate(instValues.allColorValues, SpriteAllColor, instance.m_LivingTime, instance.m_LivedTime);

	// TODO : Refactor
	if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
	{
		instValues._color = Color::Mul(instValues._originalColor, instance.ColorParent);
	}
	else
	{
		instValues._color = instValues._originalColor;
	}

	instance.ColorInheritance = instValues._color;
}

void EffectNodeSprite::UpdateRenderedInstance(Instance& instance, InstanceGroup& instanceGroup, Manager* manager)
{
	InstanceValues& instValues = instance.rendererValues.sprite;

	instValues._originalColor = AllTypeColorFunctions::Calculate(instValues.allColorValues, SpriteAllColor, instance.m_LivingTime, instance.m_LivedTime);

	float fadeAlpha = GetFadeAlpha(instance);
	if (fadeAlpha != 1.0f)
	{
		instValues._originalColor.A = (uint8_t)(instValues._originalColor.A * fadeAlpha);
	}

	if (RendererCommon.ColorBindType == BindType::Always || RendererCommon.ColorBindType == BindType::WhenCreating)
	{
		instValues._color = Color::Mul(instValues._originalColor, instance.ColorParent);
	}
	else
	{
		instValues._color = instValues._originalColor;
	}

	instance.ColorInheritance = instValues._color;
}

SpriteRenderer::NodeParameter EffectNodeSprite::GetNodeParameter(const Manager* manager, const InstanceGlobal* global)
{
	SpriteRenderer::NodeParameter nodeParameter;
	nodeParameter.ZTest = RendererCommon.ZTest;
	nodeParameter.ZWrite = RendererCommon.ZWrite;
	nodeParameter.Billboard = Billboard;
	nodeParameter.EffectPointer = GetEffect();
	nodeParameter.IsRightHand = manager->GetCoordinateSystem() == CoordinateSystem::RH;
	nodeParameter.LocalTime = global->GetUpdatedFrame() / 60.0f;

	nodeParameter.DepthParameterPtr = &DepthValues.DepthParameter;
	nodeParameter.BasicParameterPtr = &RendererCommon.BasicParameter;

	nodeParameter.ZSort = DepthValues.ZSort;

	nodeParameter.EnableViewOffset = (TranslationParam.TranslationType == ParameterTranslationType_ViewOffset);
	nodeParameter.Maginification = GetEffect()->GetMaginification();

	nodeParameter.UserData = GetRenderingUserData();

	nodeParameter.EnableViewOffset = (TranslationParam.TranslationType == ParameterTranslationType_ViewOffset);

	return nodeParameter;
}

} // namespace Effekseer
