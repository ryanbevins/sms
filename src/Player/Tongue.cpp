#define J3DMTXCALC_BASIC_INIT_OUT_OF_LINE
#define J3DMTXCALC_MAYA_INIT_OUT_OF_LINE
#define JGEOMETRY_TONGUE_TVEC3_MINUS_ASSIGN_OUT_OF_LINE
#define JGEOMETRY_TONGUE_TVEC3_MUL_RET_REF
#define JGEOMETRY_MARIOMOVE_TVEC3_SUB_OUT_OF_LINE
#include <Player/Tongue.hpp>
#include <Player/Yoshi.hpp>
#include <Player/MarioAccess.hpp>
#include <Strategic/MirrorActor.hpp>
#include <Strategic/Strategy.hpp>
#include <Map/Map.hpp>
#include <MarioUtil/MathUtil.hpp>
#include <MarioUtil/MtxUtil.hpp>
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundSE.hpp>
#include <MSound/MSound.hpp>
#include <MSound/MSoundBGM.hpp>
#include <JSystem/J3D/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/J3D/J3DGraphAnimator/J3DJoint.hpp>
#include <JSystem/J3D/J3DGraphBase/J3DShape.hpp>
#include <JSystem/J3D/J3DGraphLoader/J3DModelLoader.hpp>
#include <JSystem/JDrama/JDRNameRefGen.hpp>
#include <JSystem/JKernel/JKRFileLoader.hpp>
#include <dolphin/mtx.h>

#undef J3DMTXCALC_MAYA_INIT_OUT_OF_LINE
#undef J3DMTXCALC_BASIC_INIT_OUT_OF_LINE
#undef JGEOMETRY_TONGUE_TVEC3_MINUS_ASSIGN_OUT_OF_LINE
#undef JGEOMETRY_TONGUE_TVEC3_MUL_RET_REF
#undef JGEOMETRY_MARIOMOVE_TVEC3_SUB_OUT_OF_LINE

static const char* dummyMactorStringValue1 = "\0\0\0\0\0\0\0\0\0\0\0";
static const char* SMS_NO_MEMORY_MESSAGE   = "メモリが足りません\n";
static const char cDirtyFileName[] = "/scene/map/pollution/H_ma_rak.bti";
static const char cDirtyTexName[]  = "H_ma_rak_dummy";

void TYoshiTongue::entry()
{
	if ((int)mState != 0) {
		mModel->entry();
		mTipModel->entry();
	}
}

void TYoshiTongue::viewCalc()
{
	if ((int)mState != 0) {
		mModel->viewCalc();
		mTipModel->viewCalc();
	}
}

void TYoshiTongue::calcAnim(MtxPtr p)
{
	mHeadPos.x = p[0][3];
	mHeadPos.y = p[1][3];
	mHeadPos.z = p[2][3];
	mHeadDir.x = p[0][0];
	mHeadDir.y = p[1][0];
	mHeadDir.z = p[2][0];

	switch ((int)mState) {
	case 0: {
		J3DModelData* mdMain = mModel->getModelData();
		for (u16 i = 0; i < mdMain->getShapeNum(); ++i) {
			J3DShape* s = mdMain->getShapeNodePointer(i);
			s->unk8 |= 1;
		}
		J3DModelData* mdTip = mTipModel->getModelData();
		for (u16 i = 0; i < mdTip->getShapeNum(); ++i) {
			J3DShape* s = mdTip->getShapeNodePointer(i);
			s->unk8 |= 1;
		}
		break;
	}
	default: {
		J3DModelData* mdMain = mModel->getModelData();
		for (u16 i = 0; i < mdMain->getShapeNum(); ++i) {
			J3DShape* s = mdMain->getShapeNodePointer(i);
			s->unk8 &= ~1U;
		}
		J3DModelData* mdTip = mTipModel->getModelData();
		for (u16 i = 0; i < mdTip->getShapeNum(); ++i) {
			J3DShape* s = mdTip->getShapeNodePointer(i);
			s->unk8 &= ~1U;
		}

		JGeometry::TVec3<f32> tipPos = mTipPos;
		tipPos.y += 50.0f;
		SMS_MakeJointsToArc(mModel, mHeadPos, mHeadDir, tipPos);

	u16 n = mModel->getModelData()->getJointNum();
	MtxPtr mtxPenult = mModel->getAnmMtx(n - 2);
	MtxPtr mtxLast   = mModel->getAnmMtx(n - 1);

	JGeometry::TVec3<f32> dir;
	dir.x = mtxLast[0][3] - mtxPenult[0][3];
	dir.y = 0.0f;
	dir.z = mtxLast[2][3] - mtxPenult[2][3];
	MsVECNormalize((Vec*)&dir, (Vec*)&dir);

	JGeometry::TVec3<f32> up(0.0f, 1.0f, 0.0f);
	JGeometry::TVec3<f32> side;
	side.cross(up, dir);

	Mtx tipMtx;
	tipMtx[0][0] = side.x;
	tipMtx[0][1] = up.x;
	tipMtx[0][2] = dir.x;
	tipMtx[0][3] = tipPos.x;
	tipMtx[1][0] = side.y;
	tipMtx[1][1] = up.y;
	tipMtx[1][2] = dir.y;
	tipMtx[1][3] = tipPos.y;
	tipMtx[2][0] = side.z;
	tipMtx[2][1] = up.z;
	tipMtx[2][2] = dir.z;
	tipMtx[2][3] = tipPos.z;
	PSMTXCopy(tipMtx, mTipModel->unk20);

		mTipModel->calc();
		break;
	}
	}
}

void TYoshiTongue::movement()
{
	if (unkD4 != 0) {
		mColCount = mSavedColCount;
		unkD4--;
	} else if (mColCount != 0) {
		mSavedColCount = mColCount;
		unkD4          = 3;
	}

	switch ((int)mState) {
	case 0:
	case 2:
	case 4:
		mAttackRadius = 300.0f;
		calcEntryRadius();
		unk64 &= ~HIT_FLAG_UNK2;
		break;
	case 1:
		mAttackRadius = 1000.0f;
		calcEntryRadius();
		mProgress++;
		if (mProgress >= mMaxProgress) {
			mState = STATE_RETRACTING;
			break;
		}
		{
			BOOL goRes = canGo();
			if (!goRes) {
				mState = STATE_RETRACTING;
			}
		}
		break;
	case 3: {
		mAttackRadius = 10.0f;
		calcEntryRadius();
		JGeometry::TVec3<f32> diff(mTipPos);
		diff.sub(mHeadPos);
		JGeometry::TVec3<f32> diffSave = diff;
		if (mHeldObject != nullptr) {
			f32 len = diff.length();

			if (len < mMaxReach) {
				JGeometry::TVec3<f32>* heldVelPtr
				    = (JGeometry::TVec3<f32>*)((u8*)mHeldObject + 0x24);
				JGeometry::TVec3<f32> heldVel = *heldVelPtr;
				heldVel.scale(mElasticity);
				if (heldVel.x < 0.01f) {
					heldVel.x = 0.01f;
					heldVel.y = 0.01f;
					heldVel.z = 0.01f;
				}
				*heldVelPtr = heldVel;
			}
		}
		{
			f32 len = diffSave.length();
			if (len < mRetractedLength) {
				if (mHeldObject != nullptr) {
					mActorTypeInMouth = mHeldObject->mActorType;
					mHeldObject->receiveMessage(this, 8);
					mHeldObject->receiveMessage(this, 0xB);
					mHeldObject = nullptr;
				}
				mState = STATE_IDLE;
			}
		}
		break;
	}
	case 5:
		mAttackRadius = 10.0f;
		calcEntryRadius();
		mProgress++;
		if (mProgress >= unk82) {
			mState = STATE_RETRACTING;
		}
		break;
	case 6:
	case 7:
		mProgress++;
		break;
	}

	switch ((int)mState) {
	default:
	case 0:
	case 4:
	case 5:
		mTipPos = mHeadPos;
		break;
	case 1: {
		THitActor* target = findTarget(true, true);
		if (target != nullptr && mHeldObject == nullptr) {
			JGeometry::TVec3<f32> targetMid(target->mPosition);
			targetMid.y += 0.5f * target->mDamageHeight;
			f32 speed = mExtendAmount;

			JGeometry::TVec3<f32> tipPos = mTipPos;
			JGeometry::TVec3<f32> dir(targetMid);
			dir -= mTipPos;
			mInitialVelocity = dir * speed;

			JGeometry::TVec3<f32> tipPosNew = mTipPos + mInitialVelocity;
			mTipPos                         = tipPosNew;

			JGeometry::TVec3<f32> distVec = mTipPos - targetMid;
			f32 dist = JGeometry::TUtil<f32>::sqrt(distVec.x * distVec.x
			                                      + distVec.y * distVec.y
			                                      + distVec.z * distVec.z);
			if (dist < 200.0f) {
				if (target->receiveMessage(this, 4) == 1) {
					mHeldObject = (TTakeActor*)target;
					if (gpMSound->gateCheck(0x7920))
						MSoundSESystem::MSoundSE::startSoundActor(
						    0x7920, (Vec*)&mTipPos, 0, nullptr, 0, 4);
					mProgress = 0;
					mState    = STATE_GRABBED;
				}
			}
		} else {
			mTipPos.x += mInitialVelocity.x;
			mTipPos.y += mInitialVelocity.y;
			mTipPos.z += mInitialVelocity.z;
		}
		break;
	}
	case 2:
		mProgress++;
		if (mProgress > 10) {
			mState = STATE_RETRACTING;
		}
		break;
	case 3: {
		f32 retractAmt                = mRetractAmount;
		JGeometry::TVec3<f32> diff    = mTipPos - mHeadPos;
		JGeometry::TVec3<f32> scaled  = diff * retractAmt;
		mTipPos                       = mHeadPos + scaled;
		break;
	}
	case 6:
	case 7: {
		JGeometry::TVec3<f32> diff;
		diff.sub(mTipPos, mHeadPos);
		f32 normInvScale = diff.length();

		MtxPtr yoshiMtx = getTakingMtx();
		f32 dx          = -yoshiMtx[0][0];
		f32 dy          = -yoshiMtx[1][0];
		f32 dz          = -yoshiMtx[2][0];
		f32 pullScale   = normInvScale * mPullAmount;
		if (mState == STATE_PULLING_SLOW) {
			pullScale *= 0.5f;
		}
		dx *= pullScale;
		dy *= pullScale;
		dz *= pullScale;

		JGeometry::TVec3<f32> pullVec(dx, dy, dz);
		JGeometry::TVec3<f32> newTip = mTipPos;
		newTip.add(pullVec);

		if (mHolder->moveRequest(newTip) == 1) {
			mTipPos.x += pullVec.x;
			mTipPos.y += pullVec.y;
			mTipPos.z += pullVec.z;
		}
		break;
	}
	}

	checkTaking();

	mPosition.x = mTipPos.x;
	mPosition.y = mTipPos.y;
	mPosition.z = mTipPos.z;
	mPosition.y -= 0.5f * mAttackHeight;
}

#pragma dont_inline on
namespace JGeometry {
TVec3<f32>& TVec3<f32>::operator-=(const TVec3<f32>& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}
}
#pragma dont_inline off

THitActor* TYoshiTongue::findTarget(bool flagB1, bool flagB2)
{
	f32 bestDist       = 10000.0f;
	THitActor* bestHit = nullptr;
	for (int i = 0; i < mColCount; ++i) {
		u32 ty = mCollisions[i]->mActorType;
		if (ty == 0x10000024 || ty == 0x4000000A) {
			mState = STATE_RETRACTING;
			return nullptr;
		}
	}

	for (int i = 0; i < (int)mColCount; ++i) {
		THitActor* h = mCollisions[i];
		u32 atype    = h->mActorType;
		int isHittable = 0;
		if ((atype - 0x40000000) == 0x390) isHittable = 1;
		if ((atype - 0x40000000) == 0x391) isHittable = 1;
		if ((atype - 0x40000000) == 0x392) isHittable = 1;
		if ((atype - 0x40000000) == 0x393) isHittable = 1;
		if ((atype - 0x40000000) == 0x394) isHittable = 1;
		if ((atype - 0x40000000) == 0x395) isHittable = 1;
		if ((atype - 0x40000000) == 0x396) isHittable = 1;

		if (flagB1 == true) {
			if ((atype - 0x20000000) == 0x1) isHittable = 1;
			if ((atype - 0x20000000) == 0x2) isHittable = 1;
			if ((atype - 0x40000000) == 0x5A) isHittable = 1;
			bool b = ((atype & 0x10000000) != 0) ? true : false;
			if (b) isHittable = 1;
		}

		if (isHittable != 1) continue;

		JGeometry::TVec3<f32> targetMid(h->mPosition);
		targetMid.y += 0.5f * h->mDamageHeight;
		JGeometry::TVec3<f32> diff = targetMid - mTipPos;

		f32 lsq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		if (!(lsq <= 0.0000038146973f)) {
			f32 dist = JGeometry::TUtil<f32>::sqrt(lsq);

			if (lsq <= 0.0000038146973f) {
				diff.x = 0.0f;
				diff.y = 0.0f;
				diff.z = 0.0f;
			} else {
				f32 invl = JGeometry::TUtil<f32>::inv_sqrt(lsq) * 1.0f;
				diff.x *= invl;
				diff.y *= invl;
				diff.z *= invl;
			}

			if (flagB2) {
				f32 dot = diff.x * mHeadDir.x + diff.y * mHeadDir.y
				          + diff.z * mHeadDir.z;
				if (!(dot > 0.5f)) continue;
			}

			if (dist < bestDist) {
				bestDist = dist;
				bestHit  = mCollisions[i];
			}
		}
	}
	return bestHit;
}

#pragma dont_inline on
BOOL TYoshiTongue::canGo()
{
	JGeometry::TVec3<f32> diff = mTipPos - mHeadPos;
	f32 dot = diff.dot(mHeadDir);
	if (dot < 0.0f)
		return FALSE;

	BOOL touchedWall = gpMap->isTouchedOneWallAndMoveXZ(
	    &mTipPos.x, mTipPos.y + 10.0f, &mTipPos.z, 50.0f);
	if (touchedWall > 0)
		return FALSE;

	const TBGCheckData* gp;
	f32 gh = gpMap->checkGround(mTipPos.x, mTipPos.y, mTipPos.z, &gp);
	if (50.0f + gh > mTipPos.y) {
		mTipPos.y = 50.0f + gh;
		return TRUE;
	}

	const TBGCheckData* rp;
	f32 rh = gpMap->checkRoof(mTipPos.x, mTipPos.y, mTipPos.z, &rp) - 50.0f;
	if (rh < mTipPos.y) {
		mTipPos.y = rh;
		return TRUE;
	}
	return TRUE;
}
#pragma dont_inline off

void TYoshiTongue::emit(const JGeometry::TVec3<f32>& headPos,
                        const JGeometry::TVec3<f32>& headDir,
                        const JGeometry::TVec3<f32>& initialVel)
{
	if (mState != 0)
		return;
	mProgress = 0;
	mState    = STATE_EXTENDING;

	mTipPos     = headPos;
	mPosition   = headPos;
	mPosition.y -= 0.5f * mAttackHeight;
	mHeadPos = headPos;
	mHeadDir = headDir;

	mInitialVelocity = headDir * mInitialSpeed;

	JGeometry::TVec3<f32> velScaled = initialVel * 0.5f;
	mInitialVelocity += velScaled;

	if (mInitialVelocity.y < -50.0f)
		mInitialVelocity.y = -50.0f;
	if (mInitialVelocity.y > 50.0f)
		mInitialVelocity.y = 50.0f;
}

void TYoshiTongue::init(TYoshi* yoshi)
{
	void* data = JKRFileLoader::getGlbResource("/mario/bmd/yoshi_tongue.bmd");
	J3DModelData* modelData
	    = J3DModelLoaderDataBase::load(data, 0x10040000);
	mYoshi = yoshi;
	mModel = new J3DModel(modelData, 0x10000, 1);

	{
		J3DModelData* md = mModel->getModelData();
		for (u16 i = 0; i < md->getShapeNum(); ++i) {
			J3DShape* s = md->getShapeNodePointer(i);
			s->unk8 |= 1;
		}
	}

	mTipModel = new J3DModel(
	    J3DModelLoaderDataBase::load(JKRFileLoader::getGlbResource(
	                                     "/mario/bmd/yoshi_tongue_tip.bmd"),
	                                 0x10040000),
	    0x10000, 1);

	{
		J3DModelData* md = mTipModel->getModelData();
		for (u16 i = 0; i < md->getShapeNum(); ++i) {
			J3DShape* s = md->getShapeNodePointer(i);
			s->unk8 |= 1;
		}
	}

	mState           = 0;
	mProgress        = 0;
	mMaxProgress     = 60;
	unk82            = 600;
	mInitialSpeed    = 10.0f;
	mExtendAmount    = 0.05f;
	mRetractAmount   = 0.7f;
	mPullAmount      = 0.01f;
	mRetractedLength = 80.0f;
	mMaxReach        = 500.0f + mRetractedLength;
	mElasticity      = 0.8f;
	mHeadPos.set(0.0f, 0.0f, 0.0f);
	mTipPos.set(0.0f, 0.0f, 0.0f);
	mInitialVelocity.set(0.0f, 0.0f, 0.0f);
	mActorTypeInMouth                                            = 0;
	unkD4                                                        = 0;

	initHitActor(0x08000083, 5, 0x70000000, 1000.0f, 500.0f, 50.0f, 500.0f);
	unk64 &= ~1U;
}

void TYoshiTongue::initInLoadAfter()
{
	JDrama::TNameRefGen::search<TIdxGroupObj>("敵グループ")
	    ->getChildren()
	    .push_back(this);

	TMirrorActor* parentMirror = new TMirrorActor("ヨッシー舌in鏡");
	parentMirror->init(mModel, 4);

	TMirrorActor* tipMirror = new TMirrorActor("ヨッシー舌先in鏡");
	tipMirror->init(mTipModel, 4);
}
