#define MSL_STDFMODF_OUT_OF_LINE

#include "MoveBG/MapObjCorona.hpp"
#include "MoveBG/MapObjBase.hpp"

#include <Camera/CameraShake.hpp>
#include <Enemy/BathtubKiller.hpp>
#include <Enemy/Koopa.hpp>
#include <JSystem/J3D/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/J3D/J3DGraphLoader/J3DModelLoader.hpp>
#include <JSystem/JDrama/JDRNameRefGen.hpp>
#include <JSystem/JKernel/JKRFileLoader.hpp>
#include <JSystem/JMath.hpp>
#include <JSystem/JUtility/JUTNameTab.hpp>
#include <M3DUtil/MActor.hpp>
#include <M3DUtil/MActorData.hpp>
#include <M3DUtil/InfectiousStrings.hpp>
#include <Map/MapCollisionEntry.hpp>
#define MAP_COLLISION_ENTRY_DEFINE_SET_UP_TRANS
#include <Map/MapCollisionEntry.hpp>
#undef MAP_COLLISION_ENTRY_DEFINE_SET_UP_TRANS
#include <MarioUtil/RumbleMgr.hpp>
#include <Player/MarioAccess.hpp>
#include <Player/NozzleBase.hpp>
#include <System/Particles.hpp>
#include <math.h>
#include <stdio.h>

#undef MSL_STDFMODF_OUT_OF_LINE

// MapObjCorona is the owner TU for the out-of-line std::fmodf(float, float).
// Other TUs (e.g. Enemy/BathtubPeach) reference this weak owner via
// MSL_STDFMODF_OUT_OF_LINE; reconstruct the real MSL float implementation so
// it links and behaves identically to the target.
namespace std {
#pragma dont_inline on
float fmodf(float x, float y)
{
	if (fabsf(y) > fabsf(x))
		return x;
	return x - y * (f32)(s64)(x / y);
}
#pragma dont_inline off
} // namespace std

// rogue includes for matching __sinit (15 JALList<T> templates)
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

class JAISound;
class TMario;
class TWaterGun {
public:
	TNozzleBase* getCurrentNozzle() const;
};
class TGCConsole2 {
public:
	bool startAppearBalloon(u32, bool);
};
class TMarDirector {
public:
	void fireStreamingMovie(u8);
	void fireStartDemoCamera(const char*, const JGeometry::TVec3<f32>*, s32,
	                         f32, bool, s32 (*)(u32, u32), u32,
	                         JDrama::TActor*, JDrama::TFlagT<u16>);

	/* 0x00 */ u8 unk0[0x58];
	/* 0x58 */ s32 unk58;
	/* 0x5C */ u8 unk5C[0x18];
	/* 0x74 */ TGCConsole2* mConsole;
};
class MSound {
public:
	bool gateCheck(u32);
};

extern TMarDirector* gpMarDirector;
extern TMario* gpMarioOriginal;
extern MSound* gpMSound;
f32 SMSGetAnmFrameRate();

namespace MSoundSESystem {
class MSoundSE {
public:
	static JAISound* startSoundActor(u32, const Vec*, u32, JAISound**, u32,
	                                 u8);
};
} // namespace MSoundSESystem

static inline s32 getParamS32(const TBathtubParams* params, u32 offset)
{
	return *(const s32*)((const u8*)params + offset);
}

static inline bool isGripDead(TBathtubGrip* grip)
{
	return *((u8*)grip + 0x249) == 0;
}

static inline TKoopa* getKoopa()
{
	return JDrama::TNameRefGen::search<TKoopa>("クッパ");
}

static inline bool canUseKiller(const TBathtub* bathtub)
{
	if (bathtub->unk29A != 0)
		return false;

	TKoopa* koopa = getKoopa();
	if (!(u8)koopa->allowsLaunch())
		return false;

	return bathtub->unk248 <= 0;
}

static inline void normalizeQuat(JGeometry::TVec3<f32>& v, f32& w)
{
	f32 lenSq = v.x * v.x + v.y * v.y + v.z * v.z + w * w;
	if (lenSq <= 0.0000038146973f) {
		v.x = 0.0f;
		v.y = 0.0f;
		v.z = 0.0f;
		w   = 0.0f;
	} else {
		f32 scale = 1.0f * JGeometry::TUtil<f32>::inv_sqrt(lenSq);
		v.x *= scale;
		v.y *= scale;
		v.z *= scale;
		w *= scale;
	}
}

static inline void mulQuat(const JGeometry::TVec3<f32>& av, f32 aw,
                           const JGeometry::TVec3<f32>& bv, f32 bw,
                           JGeometry::TVec3<f32>& out, f32& outW)
{
	out.x = aw * bv.x + av.x * bw + av.y * bv.z - av.z * bv.y;
	out.y = aw * bv.y + av.y * bw + av.z * bv.x - av.x * bv.z;
	out.z = aw * bv.z + av.z * bw + av.x * bv.y - av.y * bv.x;
	outW  = aw * bw - av.x * bv.x - av.y * bv.y - av.z * bv.z;
}

static inline f32 wrapAngleDiff(f32 stored, f32 angle)
{
	const f32 minAngle = -180.0f;
	f32 diff = std::fmodf(360.0f + (stored - angle - minAngle), 360.0f);
	diff     = minAngle + diff;
	return fabsf(diff);
}

static inline f32 getLocalAngle(const TBathtub* bathtub,
                                const JGeometry::TVec3<f32>& pos)
{
	MtxPtr rootMtx = *bathtub->getRootJointMtx();
	JGeometry::TVec3<f32> xAxis(rootMtx[0][0], rootMtx[1][0], rootMtx[2][0]);
	JGeometry::TVec3<f32> zAxis(rootMtx[0][2], rootMtx[1][2], rootMtx[2][2]);
	JGeometry::TVec3<f32> origin(rootMtx[0][3], rootMtx[1][3], rootMtx[2][3]);
	JGeometry::TVec3<f32> diff;
	diff.sub(pos, origin);
	f32 localZ = zAxis.dot(diff);
	f32 localX = xAxis.dot(diff);
	return matan(localZ, localX) * 0.005493164f;
}

static inline f32 getNextLocalAngle(const TBathtub* bathtub,
                                    const JGeometry::TVec3<f32>& pos,
                                    const JGeometry::TVec3<f32>& dir)
{
	MtxPtr rootMtx = *bathtub->getRootJointMtx();
	JGeometry::TVec3<f32> diff(pos.x - rootMtx[0][3],
	                           pos.y - rootMtx[1][3],
	                           pos.z - rootMtx[2][3]);

	JGeometry::TVec3<f32> radial;
	f32 lenSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	if (lenSq <= 0.0000038146973f) {
		radial.x = 0.0f;
		radial.y = 0.0f;
		radial.z = 0.0f;
	} else {
		f32 scale = 1.0f * JGeometry::TUtil<f32>::inv_sqrt(lenSq);
		radial.x  = diff.x * scale;
		radial.y  = diff.y * scale;
		radial.z  = diff.z * scale;
	}

	f32 dot = radial.x * dir.x + radial.y * dir.y + radial.z * dir.z;
	JGeometry::TVec3<f32> tangent(dir.x - radial.x * dot,
	                              dir.y - radial.y * dot,
	                              dir.z - radial.z * dot);
	JGeometry::TVec3<f32> next(diff.x + tangent.x, diff.y + tangent.y,
	                           diff.z + tangent.z);

	f32 localX = rootMtx[0][0] * next.x + rootMtx[1][0] * next.y
	             + rootMtx[2][0] * next.z;
	f32 localZ = rootMtx[0][2] * next.x + rootMtx[1][2] * next.y
	             + rootMtx[2][2] * next.z;
	return matan(localZ, localX) * 0.005493164f;
}

static inline MtxPtr getJointMtx(TBathtub* bathtub, s32 index)
{
	return bathtub->mMActor->unk4->getAnmMtx(index);
}

// ============= TBathtubParams =============

TBathtubParams::TBathtubParams()
    : TParams("/MapObj/bathtub.prm")
    , PARAM_INIT(resetGrip, 0)
    , PARAM_INIT(trampleRelease, 10)
    , PARAM_INIT(trampleRecover, 10)
    , PARAM_INIT(quakeRelease, 500)
    , PARAM_INIT(quakeRecover, 500)
    , PARAM_INIT(hipdropRelease, 35)
    , PARAM_INIT(hipdropRecover, 35)
    , PARAM_INIT(breakCount0, 750)
    , PARAM_INIT(breakCount1, 710)
    , PARAM_INIT(breakCount2, 685)
    , PARAM_INIT(breakCount3, 655)
    , PARAM_INIT(launchStopCount, 1000)
    , PARAM_INIT(animSpeed0, 0.15f)
    , PARAM_INIT(animSpeed1, 0.15f)
    , PARAM_INIT(animSpeed2, 0.15f)
    , PARAM_INIT(animSpeed3, 0.15f)
    , PARAM_INIT(animSpeed4, 0.22f)
    , PARAM_INIT(shake, 0.0f)
    , PARAM_INIT(watermark, 0.3f)
    , PARAM_INIT(maxAngle, 35.0f)
    , PARAM_INIT(angleVelDamp, 0.93f)
    , PARAM_INIT(rebound, 0.0005f)
    , PARAM_INIT(shakeDamp, 0.93f)
    , PARAM_INIT(marioWeight, 0.01f)
    , PARAM_INIT(marioDropWeight, 5.0f)
    , PARAM_INIT(outerHeight, 20.0f)
{
	TParams::load(mPrmPath);
}

Mtx* TBathtubGripParts::getRootJointMtx() const
{
	return (Mtx*)unkF4->getModel()->getAnmMtx(unkF4->unk200[unkF8]);
}

BOOL TBathtubGripPartsFragile::receiveMessage(THitActor* sender, u32 message)
{
	return unkF4->receiveMessage(sender, message);
}

BOOL TBathtubGripPartsHard::receiveMessage(THitActor* sender, u32 message)
{
	if (message == HIT_MESSAGE_UNK3)
		message = HIT_MESSAGE_HIP_DROP;
	return unkF4->receiveMessage(sender, message);
}

void TBathtubGrip::kill()
{
	unk24A = 1;
	makeObjDead();

	for (s32 i = 0; i < 17; ++i)
		unk164[i]->remove();
	for (s32 i = 0; i < 5; ++i)
		unk150[i]->remove();
}

TBathtubGrip::TBathtubGrip(TBathtub* bathtub, f32 angle, MActorAnmData* data,
                           const char* name)
    : TMapObjBase(name)
    , unk244(bathtub)
    , unk248(0)
    , unk249(1)
    , unk24A(0)
    , unk24B(0)
    , unk24C(angle)
    , unk250(1.0f)
    , unk254(0)
    , unk258(100)
    , unk260(0)
{
	unk25C = new MActor(data);

	void* res = JKRFileLoader::getGlbResource(
	    "/scene/map/map/stand_effect/stand_effect.bmd");
	J3DModelData* modelData = J3DModelLoaderDataBase::load(res, 0x50050000);
	J3DModel* model         = new J3DModel(modelData, 0, 1);
	unk25C->setModel(model, 0x50050000);

	initAndRegister("stand_break");
	calcRootMatrix();
	getModel()->calc();

	JUTNameTab* names = getModel()->getModelData()->getJointName();
	char jointName[256];
	char collisionPath[256];

	for (s32 i = 0; i < 17; ++i) {
		sprintf(jointName, "c%d", i + 1);
		sprintf(collisionPath, "/scene/mapObj/stand_break_%s.col", jointName);

		unk200[i] = names->getIndex(jointName);
		unk164[i] = new TMapCollisionMove;
		unk1BC[i] = new TBathtubGripPartsHard(
		    this, i, "バスタブの足場の一部（壊れない）");
		unk164[i]->init(collisionPath, 0, unk1BC[i]);

		if (i < 5) {
			sprintf(jointName, "b%d", i + 1);
			sprintf(collisionPath, "/scene/mapObj/stand_break_%s.col",
			        jointName);

			unk150[i] = new TMapCollisionMove;
			unk1A8[i] = new TBathtubGripPartsFragile(
			    this, i, "バスタブの足場の一部（弱点）");
			unk150[i]->init(collisionPath, 0, unk1A8[i]);
		}
	}

	offLiveFlag(LIVE_FLAG_DEAD);
	unk248 = 0;
	unk24A = 0;
	unk249 = 1;
	unk24B = 0;
	startAnim(0);

	J3DFrameCtrl* ctrl = mMActor->getFrameCtrl(0);
	if (ctrl != nullptr) {
		ctrl->setFrame(0.0f);
		ctrl->setRate(0.0f);
	}

	unk250 = 1.0f;
	unk258 = 100;
	unk260 = 0;
}

void TBathtubGrip::perform(u32 flags, JDrama::TGraphics* graphics)
{
	TMapObjBase::perform(flags, graphics);
	if (unk260 == 0) {
		if (flags & 1) {
			PSMTXCopy(*getRootJointMtx(), unk25C->getModel()->getBaseTRMtx());
			if ((unk254 > 0 || unk248 != 0)
			    && unk25C->curAnmEndsNext(0, nullptr))
				unk260 = 1;
		}
		unk25C->perform(flags, graphics);
	}
}

BOOL TBathtubGrip::receiveMessage(THitActor* sender, u32 message)
{
	switch (message) {
	case HIT_MESSAGE_UNK3: {
		if (unk248 != 0 || unk249 == 0)
			return false;

		unk138.x = gpMarioPos->x;
		unk138.y = gpMarioPos->y;
		unk138.z = gpMarioPos->z;
		if (gpMSound->gateCheck(0x3821)) {
			MSoundSESystem::MSoundSE::startSoundActor(
			    0x3821, (Vec*)&unk138, 0, nullptr, 0, 4);
		}

		s32 deadCount = (u16)unk244->getNumGripsDead();
		if (deadCount == 4) {
			unk244->startDemo();
			return true;
		}

		unk244->quake(sender->mPosition);

		s32 breakCount;
		f32 animSpeed;
		switch (deadCount) {
		case 1:
			breakCount = unk244->unk16C->breakCount0.get();
			animSpeed  = unk244->unk16C->animSpeed1.get();
			break;
		case 2:
			breakCount = unk244->unk16C->breakCount1.get();
			animSpeed  = unk244->unk16C->animSpeed2.get();
			break;
		case 3:
			breakCount = unk244->unk16C->breakCount2.get();
			animSpeed  = unk244->unk16C->animSpeed3.get();
			break;
		case 4:
			breakCount = unk244->unk16C->breakCount3.get();
			animSpeed  = unk244->unk16C->animSpeed4.get();
			break;
		default:
			breakCount = unk244->unk16C->breakCount0.get();
			animSpeed  = unk244->unk16C->animSpeed0.get();
			break;
		}

		unk258 = breakCount;
		unk250 = animSpeed;
		unk254 = 1;
		unk25C->setBck("stand_effect");
		unk25C->setBtk("stand_effect");
		unk25C->setBrk("stand_effect");
		J3DFrameCtrl* ctrl = unk25C->getFrameCtrl(0);
		if (ctrl != nullptr)
			ctrl->setRate(12.0f * SMSGetAnmFrameRate() * 0.5f);

		unk248 = 1;
		unk249 = 0;
		if (deadCount == 0)
			deadCount = 1;
		else if (deadCount == 1)
			deadCount = 0;
		startAnim(deadCount);
		return true;
	}

	case HIT_MESSAGE_HIP_DROP:
		unk244->hipdrop(sender->mPosition);
		return true;

	case HIT_MESSAGE_TRAMPLE: {
		TBathtub* bathtub = unk244;
		if (bathtub->unk29A == 0
		    && bathtub->unk250 <= bathtub->unk16C->trampleRelease.get()) {
			bathtub->unk250 = bathtub->unk16C->trampleRelease.get();
			bathtub->unk258 = bathtub->unk16C->trampleRecover.get();
			bathtub->unk25C = bathtub->unk16C->trampleRecover.get();
			bathtub->unk254 = bathtub->unk16C->hipdropRelease.get();
		}
		return true;
	}

	default:
		return false;
	}
}

Mtx* TBathtubGrip::getRootJointMtx() const
{
	return (Mtx*)getModel()->getBaseTRMtx();
}

void TBathtubGrip::calcRootMatrix()
{
	TPosition3f* modelMtx = (TPosition3f*)getModel()->getBaseTRMtx();
	TRotation3f rot;
	MsMtxSetRotRPH(rot, 0.0f, unk24C, 0.0f);
	rot.ref(0, 3) = 0.0f;
	rot.ref(1, 3) = 0.0f;
	rot.ref(2, 3) = 0.0f;

	const TSMtx34f& parent
	    = *(const TSMtx34f*)unk244->getRootJointMtx();
	modelMtx->concat(parent, rot);
}

void TBathtubGrip::control()
{
	calcRootMatrix();
	TMapObjBase::control();

	if (unk24A != 0) {
		for (s32 i = 0; i < 17; ++i)
			unk164[i]->remove();
		for (s32 i = 0; i < 5; ++i)
			unk150[i]->remove();
		return;
	}

	mMActor->calcAnm();

	if (unk24B != 0) {
		for (s32 i = 0; i < 17; ++i) {
			unk164[i]->moveMtx(*unk1BC[i]->getRootJointMtx());
			unk164[i]->setUp();
		}

		for (s32 i = 0; i < 5; ++i) {
			unk150[i]->moveMtx(*unk1A8[i]->getRootJointMtx());
			unk150[i]->setUp();
		}
	}

	if (unk248 != 0) {
		if (animIsFinished()) {
			if (unk244->unk16C->resetGrip.get()) {
				offLiveFlag(LIVE_FLAG_DEAD);
				unk248 = 0;
				unk24A = 0;
				unk249 = 1;
				unk24B = 0;
				startAnim(0);

				J3DFrameCtrl* ctrl = mMActor->getFrameCtrl(0);
				if (ctrl != nullptr) {
					ctrl->setFrame(0.0f);
					ctrl->setRate(0.0f);
				}

				unk250 = 1.0f;
				unk258 = 100;
				unk260 = 0;
			} else {
				kill();
			}
			return;
		}

		J3DFrameCtrl* ctrl = mMActor->getFrameCtrl(0);
		if (ctrl != nullptr)
			ctrl->setRate(unk250 * SMSGetAnmFrameRate() * 0.5f);

		MtxPtr mtx = *unk1A8[0]->getRootJointMtx();
		unk144.set(mtx[0][3], mtx[1][3], mtx[2][3]);
		if (gpMSound->gateCheck(0x300D)) {
			MSoundSESystem::MSoundSE::startSoundActor(
			    0x300D, (Vec*)&unk144, 0, nullptr, 0, 4);
		}
		SMSRumbleMgr->start(8, (Vec*)&unk138);
		return;
	}

	J3DFrameCtrl* ctrl = mMActor->getFrameCtrl(0);
	ctrl->setRate(0.0f);
	if (unk254 > 0) {
		ctrl->setFrame(1.0f);
		s32 nextTimer = unk254 + 1;
		unk254       = nextTimer;
		if (nextTimer > unk258) {
			unk248 = 1;
			unk254 = 0;
			gpMarioParticleManager->emitAndBindToMtxPtr(
			    0xF6, *getRootJointMtx(), 0, this);
			gpMarioParticleManager->emitAndBindToMtxPtr(
			    0xF7, *getRootJointMtx(), 0, this);
		}
	} else {
		ctrl->setFrame(0.0f);
	}
}

void TBathtub::loadAfter()
{
	SMS_LoadParticle("/scene/map/map/ms_lkp_yuge1.jpa", 0x1BE);
	SMS_LoadParticle("/scene/map/map/ms_kp_funsui.jpa", 0x1BF);
	SMS_LoadParticle("/scene/map/map/ms_kp_break_a.jpa", 0xF6);
	SMS_LoadParticle("/scene/map/map/ms_kp_break_b.jpa", 0xF7);
}

#pragma dont_inline on
void TBathtub::hipdrop(const JGeometry::TVec3<f32>& pos)
{
	if (unk29A != 0)
		return;

	if (unk250 > getParamS32(unk16C, 0x7C))
		return;

	JGeometry::TVec3<f32> diff;
	diff.x = pos.x - mInitialPosition.x;
	diff.y = 0.0f;
	diff.z = pos.z - mInitialPosition.z;
	diff.normalize(diff);

	unk250 = getParamS32(unk16C, 0x7C);
	unk258 = getParamS32(unk16C, 0x90);
	unk25C = getParamS32(unk16C, 0x90);
	unk254 = getParamS32(unk16C, 0x7C);

	getKoopa()->stagger(false);
}
#pragma dont_inline off

void TBathtub::quake(const JGeometry::TVec3<f32>& pos)
{
	if (unk29A != 0)
		return;

	JGeometry::TVec3<f32> diff;
	diff.x = pos.x - mInitialPosition.x;
	diff.y = 0.0f;
	diff.z = pos.z - mInitialPosition.z;
	diff.normalize(diff);

	unk24C = 300;
	unk250 = getParamS32(unk16C, 0x54);
	unk258 = getParamS32(unk16C, 0x68);
	unk25C = getParamS32(unk16C, 0x68);
	unk254 = getParamS32(unk16C, 0x7C);
	unk248 = getParamS32(unk16C, 0xF4);

	TKoopa* koopa = getKoopa();
	gpCameraShake->startShake((EnumCamShakeMode)0x25, 1.0f);
	gpCameraShake->startShake((EnumCamShakeMode)0x26, 1.0f);
	SMSRumbleMgr->start(4, (f32*)nullptr);

	JGeometry::TVec3<f32> throwDir;
	throwDir.x = 0.0f;
	throwDir.y = 1.0f;
	throwDir.z = 0.0f;
	SMS_ThrowMario(throwDir, 60.0f);
	koopa->getDown();
}

s32 TBathtub::getNumGripsDead() const
{
	s32 count = 0;
	for (s32 i = 0; i < 5; ++i) {
		if (isGripDead(unk168[i]))
			++count;
	}
	return count;
}

void TBathtub::tumble(f32 angle, f32 power)
{
	if (unk29A != 0)
		return;

	s16 shortAngle = angle * (65536.0f / 360.0f);
	f32 scaled     = power * 0.0001f;
	f32 cosine     = JMASCos(shortAngle);
	f32 sine       = -JMASSin(shortAngle);
	JGeometry::TVec3<f32> impulse(scaled * cosine, 0.0f, scaled * sine);
	unk1E8.add(impulse);
}

MtxPtr TBathtub::getTakingMtx() { return getJointMtx(this, unk260); }

MtxPtr TBathtub::getSubmarineMtxInDemo() { return getJointMtx(this, unk26C); }

MtxPtr TBathtub::getPeachMtxInDemo() { return getJointMtx(this, unk270); }

MtxPtr TBathtub::getKoopaJrMtxInDemo() { return getJointMtx(this, unk274); }

BOOL TBathtub::receiveMessage(THitActor* sender, u32 message)
{
	switch (message) {
	case HIT_MESSAGE_HIP_DROP:
		hipdrop(*gpMarioPos);
		return true;
	case HIT_MESSAGE_UNK3:
		hipdrop(*gpMarioPos);
		return true;
	case HIT_MESSAGE_TRAMPLE:
		if (unk29A == 0 && unk250 <= getParamS32(unk16C, 0x2C)) {
			unk250 = getParamS32(unk16C, 0x2C);
			unk258 = getParamS32(unk16C, 0x40);
			unk25C = getParamS32(unk16C, 0x40);
			unk254 = getParamS32(unk16C, 0x7C);
		}
		return true;
	default:
		return false;
	}
}

Mtx* TBathtub::getRootJointMtx() const
{
	if (unk29A != 0)
		return (Mtx*)getModel()->getAnmMtx(0);
	return (Mtx*)getModel()->getBaseTRMtx();
}

void TBathtub::perform(u32 flags, JDrama::TGraphics* graphics)
{
	TMapObjBase::perform(flags, graphics);

	if (flags & 1) {
		PSMTXCopy(mMActor->unk4->getAnmMtx(unk264),
		          unk29C->unk4->getBaseTRMtx());
		Vec scale;
		scale.x = 1.5f;
		scale.y = 1.5f;
		scale.z = 1.5f;
		unk29C->unk4->setBaseScale(scale);
	}

	if (flags & 1) {
		TMarDirector* director = gpMarDirector;
		s32 deadCount = getNumGripsDead();
		s32 frame     = director->unk58;
		switch (deadCount) {
		case 0:
			if (frame >= 7200) {
				if ((unk2A0 & 0x2) == 0)
					director->mConsole->startAppearBalloon(0xE001F,
					                                             true);
				unk2A0 |= 0x2;
			} else if (frame >= 3600) {
				if ((unk2A0 & 0x1) == 0)
					director->mConsole->startAppearBalloon(0xE001E,
					                                             true);
				unk2A0 |= 0x1;
			}
			break;
		case 1:
			if ((unk2A0 & 0x4) == 0)
				director->mConsole->startAppearBalloon(0xE0020, true);
			unk2A0 |= 0x4;
			break;
		case 2:
			if ((unk2A0 & 0x8) == 0)
				director->mConsole->startAppearBalloon(0xE0021, true);
			unk2A0 |= 0x8;
			break;
		case 3:
			if ((unk2A0 & 0x10) == 0)
				director->mConsole->startAppearBalloon(0xE0022, true);
			unk2A0 |= 0x10;
			break;
		case 4:
			if ((unk2A0 & 0x40000) == 0)
				director->mConsole->startAppearBalloon(0xE0030, true);
			unk2A0 |= 0x40000;
			break;
		case 5:
			if ((unk2A0 & 0x20) == 0)
				director->mConsole->startAppearBalloon(0xE0023, true);
			unk2A0 |= 0x20;
			break;
		}
	}

	if (flags & 2)
		unk29C->calc();

	if (flags & 4)
		unk29C->viewCalc();

	if (flags & 0x200) {
		MtxPtr mtx = unk29C->getModel()->getBaseTRMtx();
		JGeometry::TVec3<f32> lightPos(mtx[0][3], mtx[1][3], mtx[2][3]);
		unk29C->setLightData(mGroundPlane, lightPos);
		unk29C->entry();
	}
}

void TBathtub::control()
{
	if (unk29A != 0) {
		if (mMActor->curAnmEndsNext(0, nullptr)) {
			s32 demoStep = unk294++;
			switch (demoStep) {
			case 0:
				startBck("bath_overturn2");
				break;
			case 1:
				startBck("bath_overturn3");
				break;
			}
		}

		JGeometry::TVec3<f32> scale(3.0f);
		JPABaseEmitter* emitter = gpMarioParticleManager->emitAndBindToMtxPtr(
		    0x128, unk29C->unk4->getAnmMtx(unk268), 1, this);
		if (emitter != nullptr)
			emitter->setScale(scale);

		emitter = gpMarioParticleManager->emitAndBindToMtxPtr(
		    0x129, unk29C->unk4->getAnmMtx(unk268), 1, this);
		if (emitter != nullptr)
			emitter->setScale(scale);

		MtxPtr starMtx = mMActor->unk4->getAnmMtx(unk264);
		unk200.set(starMtx[0][3], starMtx[1][3], starMtx[2][3]);
		if (gpMSound->gateCheck(0x81C1)) {
			MSoundSESystem::MSoundSE::startSoundActor(
			    0x81C1, (Vec*)&unk200, 0, nullptr, 0, 4);
		}

		calcRootMatrix();
		calcBathtubData();

		for (s32 i = 0; i < 30; ++i)
			unk164[i]->remove();

		TLiveActor* mario = SMS_GetMarioLiveActor();
		if (mario->receiveMessage(this, HIT_MESSAGE_TAKE))
			mHeldObject = (TTakeActor*)mario;

		*(u16*)((u8*)gpMarioOriginal + 0x96) = 0x7FFF;

		if (unk290 > 0)
			--unk290;
		return;
	}

	if (unk24C > 0)
		--unk24C;
	if (unk248 > 0)
		--unk248;

	bool applyMarioWeight = false;
	if (unk250 > unk16C->hipdropRelease.get()) {
		applyMarioWeight = true;
	} else if (marioIsOn() && mHeldObject == nullptr) {
		applyMarioWeight = true;
	}

	if (applyMarioWeight) {
		f32 weight = unk16C->marioWeight.get();
		if (unk250 > 0)
			weight += unk16C->marioDropWeight.get();

		if (weight > 0.0000000001f) {
			static JGeometry::TVec3<f32> yDown(0.0f, -1.0f, 0.0f);

			f32 x = gpMarioPos->x - mPosition.x;
			f32 y = gpMarioPos->y - mPosition.y;
			f32 z = gpMarioPos->z - mPosition.z;

			JGeometry::TVec3<f32> torque;
			torque.x = y * yDown.z - z * yDown.y;
			torque.y = z * yDown.x - x * yDown.z;
			torque.z = x * yDown.y - y * yDown.x;

			unk1E8.x += torque.x * weight;
			unk1E8.y += torque.y * weight;
			unk1E8.z += torque.z * weight;
		}
	}

	updatePosture_();
	calcRootMatrix();
	TMapObjBase::control();
	calcBathtubData();
	getModel()->calc();

	f32 uprightDot = unk17C.x * unk188[3] + unk17C.y * unk188[4]
	                 + unk17C.z * unk188[5];
	if (uprightDot > 0.995f)
		gpMarioParticleManager->emit(0x1BE, &unk1F4, 1, this);

	for (s32 i = 0; i < 5; ++i) {
		if (unk168[i]->unk248 != 0) {
			s32 joint = (&unk27C)[i];
			gpMarioParticleManager->emitAndBindToMtxPtr(
			    0x1BF, mMActor->unk4->getAnmMtx(joint), 1, this);
		}
	}

	if (mHeldObject == nullptr) {
		setupCollisions_();
	} else {
		for (s32 i = 0; i < 30; ++i)
			unk164[i]->remove();
	}
}

void TBathtub::calcBathtubData()
{
	MtxPtr rootMtx = *getRootJointMtx();

	unk188[0] = rootMtx[0][0];
	unk188[1] = rootMtx[1][0];
	unk188[2] = rootMtx[2][0];
	unk188[3] = rootMtx[0][1];
	unk188[4] = rootMtx[1][1];
	unk188[5] = rootMtx[2][1];
	unk188[6] = rootMtx[0][2];
	unk188[7] = rootMtx[1][2];
	unk188[8] = rootMtx[2][2];

	unk170.x = rootMtx[0][3];
	unk170.y = rootMtx[1][3];
	unk170.z = rootMtx[2][3];

	f32 upright = 1.0f - unk188[4] * unk188[4];
	if (upright > 0.0f)
		upright = JGeometry::TUtil<f32>::sqrt(upright);
	else
		upright = 0.0f;

	f32 waterLevel = unk16C->watermark.get();
	if (waterLevel < upright)
		waterLevel = upright;

	unk1B4 = unk1AC * waterLevel;
	unk1B8 = unk16C->outerHeight.get();

	unk17C.x = unk188[3];
	unk17C.y = unk188[4];
	unk17C.z = unk188[5];
	unk1C8.x = 0.0f;
	unk1C8.y = 0.0f;
	unk1C8.z = 0.0f;

	if (getKoopa()->effectsTumble() || unk24C > 0) {
		JGeometry::TVec3<f32> axis;
		axis.x = unk17C.z;
		axis.y = 0.0f;
		axis.z = -unk17C.x;

		f32 lengthSq = axis.x * axis.x + axis.y * axis.y + axis.z * axis.z;
		if (lengthSq > 0.0f) {
			axis.scale(1.0f * JGeometry::TUtil<f32>::inv_sqrt(lengthSq));

			f32 angle = 0.5f * (unk16C->maxAngle.get() * 6.2831855f / 360.0f);
			f32 sinAngle = sinf(angle);
			f32 cosAngle = cosf(angle);
			f32 oneMinusCos = 1.0f - cosAngle;
			JGeometry::TVec3<f32> normal = unk17C;

			JGeometry::TVec3<f32> cross;
			cross.x = axis.y * normal.z - axis.z * normal.y;
			cross.y = axis.z * normal.x - axis.x * normal.z;
			cross.z = axis.x * normal.y - axis.y * normal.x;
			f32 dot = axis.x * normal.x + axis.y * normal.y
			          + axis.z * normal.z;

			unk17C.x = normal.x * cosAngle + cross.x * sinAngle
			           + axis.x * dot * oneMinusCos;
			unk17C.y = normal.y * cosAngle + cross.y * sinAngle
			           + axis.y * dot * oneMinusCos;
			unk17C.z = normal.z * cosAngle + cross.z * sinAngle
			           + axis.z * dot * oneMinusCos;

			unk1C8.y = unk16C->shake.get();
		}
	} else {
		unk17C.x = 0.0f;
		unk17C.y = 1.0f;
		unk17C.z = 0.0f;
	}

	unk1D4 = (unk250 < unk254 / 2 && unk258 > 0) ? 1 : 0;
	unk1D5[0] = unk29A;
	JGeometry::TVec3<f32> pos(unk170.x, unk170.y - unk1B4, unk170.z);
	unk1F4 = pos;
}

void TBathtub::setupCollisions_()
{
	f32 diffX = gpMarioPos->x - unk170.x;
	f32 diffY = gpMarioPos->y - unk170.y;
	f32 diffZ = gpMarioPos->z - unk170.z;
	f32 localZ = unk188[6] * diffX + unk188[7] * diffY + unk188[8] * diffZ;
	f32 localX = unk188[0] * diffX + unk188[1] * diffY + unk188[2] * diffZ;
	f32 distSq = localZ * localZ + localX * localX;

	if (gpMarioPos->y < 300.0f || distSq < 0.01f) {
		for (s32 i = 0; i < 30; ++i)
			unk164[i]->remove();

		for (s32 i = 0; i < 5; ++i)
			unk168[i]->unk24B = 0;
		return;
	}

	f32 angle = atan2f(localX, localZ);
	if (angle < 0.0f)
		angle += 6.2831855f;

	f32 collisionPos = angle * 4.774648f;
	if (collisionPos < 0.0f)
		collisionPos += 30.0f;

	s32 base = ((s32)(collisionPos + 0.5f - 1.0f) + 30) % 30;

	for (s32 i = 0; i < 2; ++i) {
		s32 index = (base + i) % 30;
		f32 rot = (((f32)(index + 1) * 6.2831855f) / 30.0f) - 3.1415927f;
		TPosition3f transform;
		transform.setEularY(rot);
		transform.zeroTrans();
		TPosition3f mtx;
		mtx.concat(*(const TSMtx34f*)getModel()->getBaseTRMtx(), transform);

		unk164[index]->moveMtx(mtx);
		unk164[index]->setUp();
	}

	for (s32 i = 2; i < 30; ++i)
		unk164[(base + i) % 30]->remove();

	for (s32 i = 0; i < 5; ++i)
		unk168[i]->unk24B = 0;

	s32 gripIndex = ((s32)(5.0f * angle / 6.2831855f) + 10) % 5;
	unk168[gripIndex]->unk24B = 1;
}

void TBathtub::startDemo()
{
	if (unk29A != 0)
		return;

	MSBgm::stopTrackBGMs(7, 10);

	if ((unk2A0 & 0x20) == 0)
		gpMarDirector->mConsole->startAppearBalloon(0xE0023, true);

	unk2A0 |= 0x20;
	unk290 = 10;

	for (s32 i = 0; i < 5; ++i)
		unk168[i]->kill();

	TBathtubGrip* grip = unk168[2];
	grip->offLiveFlag(LIVE_FLAG_DEAD);
	grip->unk248 = 0;
	grip->unk24A = 0;
	grip->unk249 = 1;
	grip->unk24B = 0;
	grip->startAnim(0);
	J3DFrameCtrl* ctrl = grip->mMActor->getFrameCtrl(0);
	if (ctrl != nullptr) {
		ctrl->setFrame(0.0f);
		ctrl->setRate(0.0f);
	}
	grip->unk250 = 1.0f;
	grip->unk258 = 100;
	grip->unk260 = 0;

	f32 animSpeed = unk16C->animSpeed1.get();
	grip = unk168[2];
	grip->unk258 = 2;
	grip->unk250 = animSpeed;
	grip->unk254 = 1;
	grip->unk25C->setBck("stand_effect");
	grip->unk25C->setBtk("stand_effect");
	grip->unk25C->setBrk("stand_effect");
	ctrl = grip->unk25C->getFrameCtrl(0);
	if (ctrl != nullptr)
		ctrl->setRate(12.0f * SMSGetAnmFrameRate() * 0.5f);
	grip->unk248 = 1;
	grip->unk249 = 0;
	grip->startAnim(1);

	onMapObjFlag(0x8);
	offMapObjFlag(0x100);
	startBck("bath_overturn1");

	TLiveActor* mario = SMS_GetMarioLiveActor();
	if (mario->receiveMessage(this, HIT_MESSAGE_TAKE))
		mHeldObject = (TTakeActor*)mario;

	*(u16*)((u8*)gpMarioOriginal + 0x96) = 0x7FFF;

	TMarDirector* director = gpMarDirector;
	JDrama::TFlagT<u16> flags(0);
	director->fireStartDemoCamera(
	    "koopa_last2", &mPosition, -1, mRotation.y, false, nullptr, 0,
	    nullptr, flags);
	gpMarDirector->fireStreamingMovie(14);
	getKoopa()->fall();
	unk29A = 1;
}

bool TBathtub::allowsTumble() const
{
	JGeometry::TVec3<f32> marioPos = *gpMarioPos;
	f32 nearGripAngle;
	if (getNearGrip(marioPos, 18.0f, &nearGripAngle)) {
		f32 diffX = marioPos.x - unk170.x;
		f32 diffY = marioPos.y - unk170.y;
		f32 diffZ = marioPos.z - unk170.z;

		f32 localX = unk188[0] * diffX + unk188[1] * diffY
		             + unk188[2] * diffZ;
		f32 localY = unk188[3] * diffX + unk188[4] * diffY
		             + unk188[5] * diffZ;
		f32 localZ = unk188[6] * diffX + unk188[7] * diffY
		             + unk188[8] * diffZ;

		JGeometry::TVec3<f32> local;
		local.set(localX, localY, localZ);
		JGeometry::TVec3<f32> horizontal(local);
		horizontal.y = 0.0f;

		f32 dist = horizontal.length();
		if (dist < 4200.0f)
			return false;

		if (4700.0f < dist) {
			if (unk188[4] > 0.99f) {
				TBathtubKillerManager* manager
				    = JDrama::TNameRefGen::search<TBathtubKillerManager>(
				        "バスタブキラーマネージャー");

				u32 status = SMS_GetMarioStatus();
				if (status == 0x8008A9)
					return false;
				if (status == 0x88B)
					return false;
				if (status == 0x88D)
					return false;

				TWaterGun* waterGun
				    = *(TWaterGun**)((u8*)gpMarioOriginal + 0x3E4);
				if (waterGun != nullptr) {
					TNozzleBase* nozzle = waterGun->getCurrentNozzle();
					if (nozzle != nullptr
					    && nozzle->getNozzleKind() == 1) {
						if (*(f32*)((u8*)nozzle + 0x388) > 0.0f)
							return false;
					}
				}

				return manager->countActiveKillers() == 0;
			}
			return false;
		}
		return true;
	}
	return false;
}

void TBathtub::calcRootMatrix()
{
	if (unk29A != 0) {
		MtxPtr mtx = getModel()->getBaseTRMtx();
		MsMtxSetRotRPH(mtx, 0.0f, mRotation.y, 0.0f);
		mtx[0][3] = mPosition.x;
		mtx[1][3] = mPosition.y;
		mtx[2][3] = mPosition.z;
		return;
	}

	MtxPtr mtx = getModel()->getBaseTRMtx();
	f32 x = unk1D8.x;
	f32 y = unk1D8.y;
	f32 z = unk1D8.z;
	f32 w = unk1E4;

	f32 x2 = 2.0f * x;
	f32 y2 = 2.0f * y;
	f32 z2 = 2.0f * z;
	f32 w2 = 2.0f * w;

	mtx[0][0] = 1.0f - y2 * y - z2 * z;
	mtx[0][1] = x2 * y - w2 * z;
	mtx[0][2] = x2 * z + w2 * y;
	mtx[1][0] = x2 * y + w2 * z;
	mtx[1][1] = 1.0f - x2 * x - z2 * z;
	mtx[1][2] = y2 * z - w2 * x;
	mtx[2][0] = x2 * z - w2 * y;
	mtx[2][1] = y2 * z + w2 * x;
	mtx[2][2] = 1.0f - x2 * x - y2 * y;
	mtx[0][3] = mPosition.x;
	mtx[1][3] = mPosition.y;
	mtx[2][3] = mPosition.z;
}

#pragma dont_inline on
bool TBathtub::getNearGrip(const JGeometry::TVec3<f32>& pos, f32 threshold,
                           f32* out) const
{
	f32 angle = getLocalAngle(this, pos);
	f32 best  = 360.0f;
	s32 index = 0;

	for (s32 i = 0; i < 5; ++i) {
		f32 diff = wrapAngleDiff(unk150[i], angle);
		if (diff < best) {
			index = i;
			best  = diff;
		}
	}

	if (best < threshold) {
		*out = unk150[index];
		return true;
	}

	return false;
}
#pragma dont_inline off

f32 TBathtub::getNextJuncture(const JGeometry::TVec3<f32>& pos,
                              const JGeometry::TVec3<f32>& dir) const
{
	f32 angle = getNextLocalAngle(this, pos, dir);
	f32 best  = 360.0f;
	s32 index = 0;

	for (s32 i = 0; i < 5; ++i) {
		f32 diff = wrapAngleDiff(unk13C[i], angle);
		if (diff < best) {
			index = i;
			best  = diff;
		}
	}

	return unk13C[index];
}

u8 TBathtub::getNextGrip(const JGeometry::TVec3<f32>& pos,
                         const JGeometry::TVec3<f32>& dir, f32 threshold,
                         f32* out) const
{
	f32 angle = getNextLocalAngle(this, pos, dir);
	f32 best  = 360.0f;
	s32 index = 0;

	for (s32 i = 0; i < 5; ++i) {
		f32 diff = wrapAngleDiff(unk150[i], angle);
		if (diff < best) {
			index = i;
			best  = diff;
		}
	}

	if (best < threshold) {
		*out = unk150[index];
		return true;
	}

	return false;
}

void TBathtub::updatePosture_()
{
	static JGeometry::TVec3<f32> yDown(0.0f, -1.0f, 0.0f);

	if (unk250 == 0) {
		f32 recover = 1.0f;
		if (unk258 != 0) {
			--unk258;
			recover = 1.0f - (f32)unk258 / (f32)unk25C;
		}

		// up = quaternion rotation of (0,1,0): the 2nd column of the
		// orientation matrix built from the quaternion.
		f32 x = unk1D8.x;
		f32 y = unk1D8.y;
		f32 z = unk1D8.z;
		f32 w = unk1E4;
		f32 upX = 2.0f * (x * y - w * z);
		f32 upY = 1.0f - 2.0f * (x * x + z * z);
		f32 upZ = 2.0f * (y * z + w * x);

		// axis = cross(yDown, up)
		JGeometry::TVec3<f32> axis;
		axis.x = yDown.y * upZ - yDown.z * upY;
		axis.y = yDown.z * upX - yDown.x * upZ;
		axis.z = yDown.x * upY - yDown.y * upX;

		f32 lenSq = axis.x * axis.x + axis.y * axis.y + axis.z * axis.z;
		if (lenSq <= 0.0000038146973f) {
			axis.x = 0.0f;
			axis.y = 0.0f;
			axis.z = 0.0f;
		} else {
			axis.scale(1.0f * JGeometry::TUtil<f32>::inv_sqrt(lenSq));
		}

		f32 dot = yDown.x * upX + yDown.y * upY + yDown.z * upZ;
		f32 correction
		    = -acosf(dot) * recover * unk16C->rebound.get();
		axis.x *= correction;
		axis.y *= correction;
		axis.z *= correction;

		f32 damp = unk16C->angleVelDamp.get();
		unk1E8.x = unk1E8.x * damp + axis.x;
		unk1E8.y = unk1E8.y * damp + axis.y;
		unk1E8.z = unk1E8.z * damp + axis.z;
	} else {
		--unk250;
	}

	JGeometry::TVec3<f32> delta;
	delta.x = unk1E8.x * 0.5f;
	delta.y = unk1E8.y * 0.5f;
	delta.z = unk1E8.z * 0.5f;

	JGeometry::TVec3<f32> step;
	f32 stepW;
	mulQuat(delta, 0.0f, unk1D8, unk1E4, step, stepW);

	unk1D8.x += step.x;
	unk1D8.y += step.y;
	unk1D8.z += step.z;
	unk1E4 += stepW;
	normalizeQuat(unk1D8, unk1E4);

	f32 x = unk1D8.x;
	f32 y = unk1D8.y;
	f32 z = unk1D8.z;
	f32 w = unk1E4;
	f32 upX = 2.0f * (x * y - w * z);
	f32 upY = 1.0f - 2.0f * (x * x + z * z);
	f32 upZ = 2.0f * (y * z + w * x);

	f32 dot          = yDown.x * upX + yDown.y * upY + yDown.z * upZ;
	f32 currentAngle = acosf(dot);
	f32 maxAngle     = unk16C->maxAngle.get() * 0.017453292f;
	f32 excess       = currentAngle - maxAngle;

	if (excess > 0.0f) {
		JGeometry::TVec3<f32> axis;
		axis.x = upZ * yDown.x - upX * yDown.y;
		axis.y = upY * yDown.y - upZ * yDown.z;
		axis.z = upX * yDown.z - upY * yDown.x;

		f32 lenSq = axis.x * axis.x + axis.y * axis.y + axis.z * axis.z;
		f32 len   = 0.0f;
		if (lenSq > 0.0f)
			len = JGeometry::TUtil<f32>::sqrt(lenSq);

		JGeometry::TVec3<f32> limit;
		f32 limitW;
		if (len <= 0.0000038146973f) {
			limit.x = 0.0f;
			limit.y = 0.0f;
			limit.z = 0.0f;
			limitW  = 1.0f;
		} else {
			f32 axisDot
			    = upX * yDown.x + upY * yDown.z + upZ * yDown.y;
			f32 angle = 0.5f * atan2f(len, axisDot) * (excess / currentAngle);
			f32 scale = sinf(angle) / len;
			limit.x   = axis.x * scale;
			limit.y   = axis.y * scale;
			limit.z   = axis.z * scale;
			limitW    = cosf(angle);
		}

		JGeometry::TVec3<f32> limited;
		f32 limitedW;
		mulQuat(limit, limitW, unk1D8, unk1E4, limited, limitedW);
		unk1D8 = limited;
		unk1E4 = limitedW;
	}

	normalizeQuat(unk1D8, unk1E4);
}

TBathtub::TBathtub(const char* name)
    : TMapObjBase(name)
    , unk164(nullptr)
    , unk290(0)
{
	unk16C = new TBathtubParams;

	unk1D8.x = 0.0f;
	unk1D8.y = 0.0f;
	unk1D8.z = 0.0f;
	unk1E4   = 1.0f;

	mPosition.z = 0.0f;
	mPosition.y = 0.0f;
	mPosition.x = 0.0f;

	unk1E8.z = 0.0f;
	unk1E8.y = 0.0f;
	unk1E8.x = 0.0f;

	unk250 = 0;
	unk254 = 1;
	unk258 = 0;
	unk25C = 1;
	unk248 = 0;
	unk298 = 0;

	unk23C.z = 0.0f;
	unk23C.y = 0.0f;
	unk23C.x = 0.0f;

	unk299 = 0;
	unk29A = 0;
	unk2A0 = 0;
	unk294 = 0;
}

void TBathtub::load(JSUMemoryInputStream& stream)
{
	unk24C = 0;
	TMapObjBase::load(stream);

	mPosition.x = mInitialPosition.x;
	mPosition.y = mInitialPosition.y;
	mPosition.z = mInitialPosition.z;

	unk164 = new TMapCollisionMove*[30];
	for (s32 i = 0; i < 30; ++i) {
		unk164[i] = new TMapCollisionMove;
		const char* path = nullptr;
		switch (i % 6) {
		case 0:
			path = "/scene/mapObj/bath_col_inside3.col";
			break;
		case 1:
			path = "/scene/mapObj/bath_col_inside2.col";
			break;
		case 2:
			path = "/scene/mapObj/bath_col_inside1.col";
			break;
		case 3:
			path = "/scene/mapObj/bath_col_inside6.col";
			break;
		case 4:
			path = "/scene/mapObj/bath_col_inside5.col";
			break;
		case 5:
			path = "/scene/mapObj/bath_col_inside4.col";
			break;
		}
		unk164[i]->init(path, 0, this);
		((TMapCollisionBase*)unk164[i])->remove();
	}

	unk170.x = mInitialPosition.x;
	unk170.y = mInitialPosition.y;
	unk170.z = mInitialPosition.z;

	unk188[7] = 0.0f;
	unk188[6] = 0.0f;
	unk188[5] = 0.0f;
	unk188[3] = 0.0f;
	unk188[2] = 0.0f;
	unk188[1] = 0.0f;
	unk188[8] = 1.0f;
	unk188[4] = 1.0f;
	unk188[0] = 1.0f;

	unk1AC = 3000.0f;
	unk1B0 = 3600.0f;
	unk1B4 = unk1AC * sinf(0.27925268f);

	unk1BC.z = 0.0f;
	unk1BC.y = 0.0f;
	unk1BC.x = 0.0f;
	unk1C8.z = 0.0f;
	unk1C8.y = 0.0f;
	unk1C8.x = 0.0f;
	unk1B8   = 100.0f;
	unk1D4   = 0;

	unk17C.x = 0.0f;
	unk17C.y = 1.0f;
	unk17C.z = 0.0f;

	unk168 = new TBathtubGrip*[5];
	unk138 = new MActorAnmData;
	unk138->init("scene/map/map/stand_effect", nullptr);

	for (s32 i = 0; i < 5; ++i) {
		f32 midAngle = 360.0f * (i + 0.5f) / 5.0f;
		f32 angle    = midAngle - 180.0f;
		unk168[i]
		    = new TBathtubGrip(this, angle, unk138,
		                       "壊れかけのバスタブの取っ手");
		unk168[i]->initMapObj();
		unk13C[i] = -180.0f + midAngle;
		unk150[i] = -180.0f + (360.0f * i) / 5.0f;
	}

	JUTNameTab* names = getModel()->getModelData()->getJointName();
	unk260            = names->getIndex("mario");
	unk264            = names->getIndex("star");
	unk27C            = names->getIndex("water4");
	unk280            = names->getIndex("water5");
	unk284            = names->getIndex("water1");
	unk288            = names->getIndex("water2");
	unk28C            = names->getIndex("water3");
	unk270            = names->getIndex("ahiru");
	unk26C            = names->getIndex("submarin");
	unk274            = names->getIndex("Jr");
	unk278            = names->getIndex("koopa");

	MActorAnmData* shineData = new MActorAnmData;
	shineData->init("/scene/map/map/shine", nullptr);
	unk29C = new MActor(shineData);

	void* res = JKRFileLoader::getGlbResource(
	    "/scene/map/map/shine/shine_3bai.bmd");
	J3DModelData* modelData = J3DModelLoaderDataBase::load(res, 0x10000000);
	J3DModel* model         = new J3DModel(modelData, 0, 1);
	unk29C->setModel(model, 0x10000000);
	unk268 = unk29C->getModel()->getModelData()->getJointName()->getIndex(
	    "body");
	unk298 = 1;
}

s32 TBathtub::getNumKillerLaunchable() const
{
	if (!canUseKiller(this))
		return 0;

	s32 count = getNumGripsDead() + 1;
	if (count < 2)
		count = 2;
	if (count > 4)
		count = 4;
	return count;
}

bool TBathtub::isKillerAttackable() const { return unk248 <= 0; }

s32 TBathtub::getNumKillerBurstable() const
{
	if (!canUseKiller(this))
		return 0;

	s32 count = getNumGripsDead();
	if (count >= 4)
		return 8;

	if (!allowsTumble() && unk250 == 0 && unk258 == 0) {
		switch (count) {
		case 1:
			return 4;
		case 2:
			return 6;
		case 3:
			return 8;
		case 4:
			return 8;
		}
	}

	return 0;
}
