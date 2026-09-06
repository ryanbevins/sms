#include <MoveBG/MapObjItem2.hpp>
#include <Map/Map.hpp>
#include <Map/MapData.hpp>
#include <Map/MapCollisionManager.hpp>
#include <Map/MapCollisionEntry.hpp>
#define MAP_COLLISION_ENTRY_DEFINE_SET_UP_TRANS
#include <Map/MapCollisionEntry.hpp>
#undef MAP_COLLISION_ENTRY_DEFINE_SET_UP_TRANS
#include <Strategic/HitActor.hpp>
#include <Player/MarioAccess.hpp>
#include <System/MarDirector.hpp>
#include <System/EmitterViewObj.hpp>
#include <System/Particles.hpp>
#include <System/Application.hpp>
#include <MSound/MSound.hpp>
#include <MSound/MSoundSE.hpp>
#include <MarioUtil/MathUtil.hpp>
#include <M3DUtil/MActor.hpp>
#include <JSystem/J3D/J3DGraphAnimator/J3DAnimation.hpp>
#include <JSystem/J3D/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/JMath.hpp>
#include <JSystem/JSupport/JSUInputStream.hpp>
#include <JSystem/JParticle/JPAEmitter.hpp>
#include <dolphin/mtx.h>
#include <stdlib.h>

#include <M3DUtil/InfectiousStrings.hpp>

static inline f32 callMsWrap(f32 t, f32 l, f32 r)
{
	return MsWrap<f32>(t, l, r);
}

TMushroom1up::TMushroom1up(int type, const char* name)
    : TMapObjBase(name)
{
	mState = 0;
	mType  = (s8)type;
	mTaken = 0;
	mTimer = 0;
}

void TMushroom1up::touchPlayer(THitActor* sender)
{
	if (mTaken == 1)
		return;
	if (!sender->receiveMessage(this, 0xe))
		return;
	mTimer = 0;
	mTaken = 1;
	if (gpMSound->gateCheck(0x4841)) {
		MSoundSESystem::MSoundSE::startSoundSystemSE(0x4841, 0, 0, 0);
	}
	mGroundPlane = TMap::getIllegalCheckData();
}

void TMushroom1up::makeObjAppeared()
{
	TMapObjBase::makeObjAppeared();
	mLifeTimer = 0x4b0;
	mState     = 0;
	mTaken     = 0;
	if (mType != 2) {
		if (gpMSound->gateCheck(0x4854)) {
			MSoundSESystem::MSoundSE::startSoundSystemSE(0x4854, 0, 0, 0);
		}
	}
	{
		JPABaseEmitter* emt
		    = gpMarioParticleManager->emit(0xe5, &mPosition, 0, 0);
		if (emt) {
			emt->unk154.x = mScaling.x;
			emt->unk154.y = mScaling.y;
			emt->unk154.z = mScaling.z;
			emt->unk174.x = mScaling.x;
			emt->unk174.y = mScaling.y;
			emt->unk174.z = mScaling.z;
		}
	}
	{
		JPABaseEmitter* emt
		    = gpMarioParticleManager->emit(0xe6, &mPosition, 0, 0);
		if (emt) {
			emt->unk154.x = mScaling.x;
			emt->unk154.y = mScaling.y;
			emt->unk154.z = mScaling.z;
			emt->unk174.x = mScaling.x;
			emt->unk174.y = mScaling.y;
			emt->unk174.z = mScaling.z;
		}
	}
}

void TMushroom1up::initMapObj()
{
	TMapObjBase::initMapObj();
	mGravity = 0.35f;
	mLiveFlag &= ~0x93;
	if (mType == 2) {
		mLiveFlag |= 0x10;
		makeObjAppeared();
	}
	mScaling.x = 1.5f;
	mScaling.y = 1.5f;
	mScaling.z = 1.5f;
}

void TMushroom1up::load(JSUMemoryInputStream& s)
{
	TMapObjBase::load(s);
	mLiveFlag &= ~0x90;
}

void TMushroom1up::control()
{
	TMapObjBase::control();
	if (mTaken == 1) {
		int remaining = 180 - mTimer;
		if (remaining < 0) {
			kill();
			return;
		}
		JGeometry::TVec3<f32> pos = *gpMarioPos;
		s16 angle = (s16)(remaining * 5.0f * (65536.0f / 360.0f));
		pos.y += 200.0f;
		pos.x += 1.5f * (50.0f * JMASCos(angle));
		pos.z += 1.5f * (50.0f * JMASSin(angle));
		mPosition = pos;
		mScaling.set(1.5f, 1.5f, 1.5f);
		mVelocity.zero();
		mLinearVelocity.zero();
		mTimer += 1;
		return;
	} else {
		mTimer += 1;
		if (mType == 2) {
			mVelocity.x       = 0.0f;
			mVelocity.y       = 0.0f;
			mVelocity.z       = 0.0f;
			mLinearVelocity.x = 0.0f;
			mLinearVelocity.y = 0.0f;
			mLinearVelocity.z = 0.0f;
			return;
		}
		if (mState == 0) {
			if (isAirborne())
				return;
			mState = 1;
		}
	}
	JGeometry::TVec3<f32> diff = *gpMarioPos;
	diff -= mPosition;
	diff.y = 0.0f;
	f32 distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	if (distSq <= 0.0000038146973f) {
		diff.x = 1.0f;
	}
	if (mType == 1) {
		diff.x = -diff.x;
		diff.y = -diff.y;
		diff.z = -diff.z;
	}
	f32 angle = MsGetRotFromZaxisY(diff);
	f32 wrapped = callMsWrap(mRotation.y, angle - 180.0f, angle + 180.0f);
	f32 delta   = angle - wrapped;
	f32 clamped;
	if (delta > 0.0f) {
		clamped = (delta > 1.0f) ? 1.0f : delta;
	} else {
		clamped = (delta < -1.0f) ? -1.0f : delta;
	}
	f32 newRotY = mRotation.y + clamped;
	while (newRotY >= 360.0f)
		newRotY -= 360.0f;
	while (newRotY < 0.0f)
		newRotY += 360.0f;
	mRotation.y = newRotY;
	PSVECNormalize((Vec*)&diff, (Vec*)&diff);
	diff.x *= 3.8f;
	diff.y *= 3.8f;
	diff.z *= 3.8f;
	mLinearVelocity.x += diff.x;
	mLinearVelocity.y += diff.y;
	mLinearVelocity.z += diff.z;
}

void TMushroom1up::perform(u32 param_1, JDrama::TGraphics* graphics)
{
	if (mType != 2 && mLifeTimer < 0xf0 && (param_1 & 0x200)) {
		if (gpMarDirector->unk58 % 6 > 2)
			param_1 &= ~0x200;
	}
	if ((param_1 & 1) && mTaken == 0 && mType != 2 && mLifeTimer <= 0) {
		kill();
	}
	TMapObjBase::perform(param_1, graphics);
}

TJumpBase::TJumpBase(const char* name)
    : TMapObjBase(name)
{
	mState = 2;
}

void TJumpBase::initMapObj()
{
	TMapObjBase::initMapObj();
	if (mMapCollisionManager) {
		TMapCollisionBase* col = mMapCollisionManager->unk8;
		col->setAllBGType(7);
		col->setAllActor(this);
		col->setAllData(0x2710);
	}
	unkE8 = 0;
}

void TJumpBase::ensureTakeSituation()
{
	if (mHeldObject && mHeldObject->mHolder != this)
		mHeldObject = 0;
	if (!mHolder)
		return;
	if (mHolder->mHeldObject == this)
		return;
	mHolder = 0;
}

BOOL TJumpBase::receiveMessage(THitActor* sender, u32 message)
{
	if (sender->isActorTypeOf(0x80000000)) {
		if (message == 4) {
			if (mState == 0) {
				mHolder = (TTakeActor*)sender;
				unk64 |= 1;
				if (mMapCollisionManager
				    && mMapCollisionManager->unk8) {
					mMapCollisionManager->unk8->remove();
				}
				return TRUE;
			}
		} else {
			if (message == 8) {
				mHolder = 0;
				mTimer  = 0;
				mState  = 2;
				return TRUE;
			}
			if (message == 6) {
				mHolder = 0;
				mTimer  = 0;
				mState  = 2;
				return TRUE;
			}
			if (message == 7) {
				mHolder = 0;
				mTimer  = 0;
				mState  = 5;
				return TRUE;
			}
			if (message == 0) {
				mTimer = 0;
				mState = 4;
				return TRUE;
			}
		}
	}
	if (sender->isActorTypeOf(0x01000000)) {
		if (mState == 3) {
			mTimer = 0;
			mState = 1;
			return TRUE;
		}
	}
	return FALSE;
}

Mtx* TJumpBase::getRootJointMtx() const
{
	return mMActor->unk4->mNodeMatrices;
}

void TJumpBase::calcRootMatrix()
{
	if (mHolder) {
		J3DModel* model    = getModel();
		TTakeActor* holder = mHolder;
		MtxPtr holderMtx   = holder->getTakingMtx();
		PSMTXCopy(holderMtx, model->unk20);
		model->unk14 = (Vec&)mScaling;
		mPosition.set(holderMtx[0][3], holderMtx[1][3], holderMtx[2][3]);
	} else {
		TMapObjBase::calcRootMatrix();
	}
}

void TJumpBase::control()
{
	int prevState = mState;
	if (!isAirborne())
		onLiveFlag(LIVE_FLAG_UNK10);

	switch (mState) {
	case 0:
		if (mTimer == 0) {
			getMActor()->setBck("jumpbase_shrink");
			J3DFrameCtrl* ctrl = getMActor()->getFrameCtrl(0);
			if (ctrl) {
				ctrl->setFrame((f32)ctrl->getEnd());
				ctrl->setRate(0.0f);
			}
			mScaledBodyRadius = 50.0f;
		}
		break;

	case 3:
		if (mTimer == 0) {
			offHitFlag(HIT_FLAG_NO_COLLISION);
			if (mMapCollisionManager)
				mMapCollisionManager->getUnk8()->setUp();

			getMActor()->setBck("jumpbase_set");
			J3DFrameCtrl* ctrl = getMActor()->getFrameCtrl(0);
			if (ctrl) {
				ctrl->setFrame((f32)ctrl->getEnd());
				ctrl->setRate(0.0f);
			}
		}
		break;

	case 2:
		if (mTimer == 0) {
			getMActor()->setBck("jumpbase_set");
			J3DFrameCtrl* ctrl = getMActor()->getFrameCtrl(0);
			if (ctrl) {
				ctrl->setFrame(0.0f);
				ctrl->setRate(SMSGetAnmFrameRate());
			}
			offLiveFlag(LIVE_FLAG_UNK10);
			mScaledBodyRadius = 100.0f;
		}
		if (getMActor()->curAnmEndsNext(0, nullptr)) {
			mTimer = 0;
			mState = 3;
		}
		break;

	case 1:
		if (mTimer == 0) {
			if (mMapCollisionManager && mMapCollisionManager->getUnk8())
				mMapCollisionManager->getUnk8()->remove();

			getMActor()->setBck("jumpbase_shrink");
			J3DFrameCtrl* ctrl = getMActor()->getFrameCtrl(0);
			if (ctrl) {
				ctrl->setFrame(0.0f);
				ctrl->setRate(SMSGetAnmFrameRate());
			}
		}
		if (getMActor()->curAnmEndsNext(0, nullptr)) {
			mTimer = 0;
			mState = 0;
		}
		break;

	case 4:
		if (mTimer == 0) {
			getMActor()->setBck("jumpbase_jump");
			J3DFrameCtrl* ctrl = getMActor()->getFrameCtrl(0);
			if (ctrl) {
				ctrl->setFrame(0.0f);
				ctrl->setRate(SMSGetAnmFrameRate());
			}
		}
		if (getMActor()->curAnmEndsNext(0, nullptr)) {
			mTimer = 0;
			mState = 3;
		}
		break;

	case 5:
		if (mTimer == 0) {
			onLiveFlag(LIVE_FLAG_AIRBORNE);
			int angle = *gpMarioAngleY;
			mVelocity
			    = JGeometry::TVec3<f32>(JMASSin(angle), 0.0f, JMASCos(angle));
			JGeometry::TVec3<f32> velocity = mVelocity;
			mPosition += velocity;
			offLiveFlag(LIVE_FLAG_UNK10);
		}
		if (!isAirborne()) {
			mTimer = 0;
			mState = 2;
		}
		break;
	}

	if (mState == prevState) {
		mTimer++;
		if (mTimer == 0)
			mTimer = 1;
	}

	TMapObjBase::control();

	if (mGroundPlane) {
		if (mGroundPlane->isIllegalData() || mGroundPlane->isWaterSurface()) {
			makeObjDead();
			makeObjDefault();
			makeObjAppeared();
		}
	}

	if (mHolder) {
		mGroundPlane  = SMS_GetMarioGroundPlane();
		mGroundHeight = SMS_GetMarioPos().y;
	}
}

// rogue includes for static init (JALList<*> instantiations)
#include <MSound/MSoundBGM.hpp>
#include <MSound/MSSetSound.hpp>
