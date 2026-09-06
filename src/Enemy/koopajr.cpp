#define MSL_STDFMODF_OUT_OF_LINE
#define JGEOMETRY_ROTATION3_IDENTITY33_OUT_OF_LINE

#include <Enemy/Koopa.hpp>
#include <Enemy/BathtubBinder.hpp>
#include <JSystem/JGeometry/JGUtil.hpp>
#include <JSystem/JDrama/JDRNameRefGen.hpp>
#include <MarioUtil/MathUtil.hpp>
#include <M3DUtil/MActor.hpp>
#include <MoveBG/MapObjCorona.hpp>
#include <MSound/MSound.hpp>
#include <Player/MarioAccess.hpp>
#include <Player/Watergun.hpp>
#include <Strategic/ObjManager.hpp>
#include <Strategic/ObjModel.hpp>
#include <Strategic/Spine.hpp>
#include <Strategic/Strategy.hpp>
#include <System/Particles.hpp>
#include <System/FlagManager.hpp>
#include <math.h>
#include <stdlib.h>

#undef MSL_STDFMODF_OUT_OF_LINE
#undef JGEOMETRY_ROTATION3_IDENTITY33_OUT_OF_LINE

// The out-of-line std::fmodf(float, float) is owned by MoveBG/MapObjCorona;
// reference it here so std::fmodf calls resolve to fmodf__3stdFff (single
// precision) instead of the inline ::fmod (double), matching the target.
namespace std {
float fmodf(float, float);
}

// rogue includes needed for matching sinit
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

static const char* koopajr_bastable[] = {
	"/scene/koopajr/bas/koopajr_damage.bas",
	"/scene/koopajr/bas/koopajr_shoot.bas",
	nullptr,
	"/scene/koopajr/bas/koopajr_yahoo.bas",
};

static const char* TKoopaJr_jointNameTable[] = {
	"KoopaJr_null",
	"killer_null00",
	"killer_null03",
	"killer_null01",
	"killer_null04",
};

static s32 TKoopaJr_jointIndexTable[5];
static const char* koopajrsubmarine_bastable[] = { nullptr };

DEFINE_NERVE(TNerveKoopaJrWait, TLiveActor)
{
	TKoopaJr* actor = (TKoopaJr*)spine->getBody();
	if (spine->getTime() == 0) {
		actor->mMActor->setBckFromIndex(2);

		const char** bas = actor->getBasNameTable();
		actor->setAnmSound(!bas ? nullptr : bas[2]);

		actor->unk154 = actor->getSaveParam2()->mSLLaunchKillerPeriod.get();
		actor->unk158
		    = actor->getSaveParam2()->mSLLaunchKillerPeriodFast.get();
	}

	return FALSE;
}

DEFINE_NERVE(TNerveKoopaJrSubmarineLaunchKiller, TLiveActor)
{
	TKoopaJrSubmarine* actor = (TKoopaJrSubmarine*)spine->getBody();
	if (actor->unk180 == actor->unk184 && actor->unk150 <= 0) {
		J3DFrameCtrl* ctrl = actor->mMActor->getFrameCtrl(0);
		ctrl->setRate(actor->unk188);
		return TRUE;
	}

	return FALSE;
}

DEFINE_NERVE(TNerveKoopaJrSubmarineCannonOpenClose, TLiveActor)
{
	TKoopaJrSubmarine* actor = (TKoopaJrSubmarine*)spine->getBody();
	if (spine->getTime() == 0) {
		actor->mMActor->setBckFromIndex(0);

		const char** bas = actor->getBasNameTable();
		actor->setAnmSound(!bas ? nullptr : bas[0]);
		J3DFrameCtrl* ctrl = actor->mMActor->getFrameCtrl(0);
		ctrl->setRate(actor->unk188);
	}

	return actor->mMActor->isCurAnmAlreadyEnd(0) ? TRUE : FALSE;
}

DEFINE_NERVE(TNerveKoopaJrSubmarineWait, TLiveActor)
{
	TKoopaJrSubmarine* actor = (TKoopaJrSubmarine*)spine->getBody();
	if (spine->getTime() == 0) {
		actor->mMActor->setBckFromIndex(0);

		const char** bas = actor->getBasNameTable();
		actor->setAnmSound(!bas ? nullptr : bas[0]);
		actor->mMActor->getFrameCtrl(0)->setRate(0.0f);
	}

	return FALSE;
}

DEFINE_NERVE(TNerveKoopaJrLaunch, TLiveActor)
{
	TKoopaJr* actor = (TKoopaJr*)spine->getBody();
	if (spine->getTime() == 0) {
		actor->mMActor->setBckFromIndex(1);

		const char** bas = actor->getBasNameTable();
		actor->setAnmSound(!bas ? nullptr : bas[1]);
	}

	return actor->mMActor->isCurAnmAlreadyEnd(0) ? TRUE : FALSE;
}

DEFINE_NERVE(TNerveKoopaJrDemo, TLiveActor)
{
	TKoopaJr* actor = (TKoopaJr*)spine->getBody();
	if (spine->getTime() == 0) {
		actor->mMActor->setBckFromIndex(0);

		const char** bas = actor->getBasNameTable();
		actor->setAnmSound(!bas ? nullptr : bas[0]);
	}

	return FALSE;
}

DEFINE_NERVE(TNerveKoopaJrDamage, TLiveActor)
{
	TKoopaJr* actor = (TKoopaJr*)spine->getBody();
	if (spine->getTime() == 0) {
		actor->mMActor->setBckFromIndex(0);

		const char** bas = actor->getBasNameTable();
		actor->setAnmSound(!bas ? nullptr : bas[0]);
	}

	if (actor->unk150 <= 0 && actor->mMActor->isCurAnmAlreadyEnd(0))
		return TRUE;
	return FALSE;
}

DEFINE_NERVE(TNerveKoopaJrYahoo, TLiveActor)
{
	TKoopaJr* actor = (TKoopaJr*)spine->getBody();
	if (spine->getTime() == 0) {
		actor->mMActor->setBckFromIndex(3);

		const char** bas = actor->getBasNameTable();
		actor->setAnmSound(!bas ? nullptr : bas[3]);
	}

	return actor->mMActor->isCurAnmAlreadyEnd(0) ? TRUE : FALSE;
}

TDirectionCalc::TDirectionCalc()
    : mDirection(0.0f)
{
}

TDirectionCalc::TDirectionCalc(f32 direction)
    : mDirection(direction)
{
}

TDirectionCalc::TDirectionCalc(JGeometry::TVec3<f32> direction)
{
	makeDirection(direction);
}

f32 TDirectionCalc::r2d(f32 radians)
{
	return 180.0f * radians / 3.1415927f;
}

f32 TDirectionCalc::d2r(f32 degrees)
{
	degrees *= 3.1415927f;
	return degrees / 180.0f;
}

f32 TDirectionCalc::absDirection(f32 direction)
{
	mDirection = JGeometry::TUtil<f32>::zero()
	             + JGeometry::TUtil<f32>::mod(
	                 6.2831855f
	                     + (mDirection - JGeometry::TUtil<f32>::zero()),
	                 6.2831855f);

	if (direction >= mDirection) {
		f32 diff = direction - mDirection;
		if (6.2831855f - diff < diff)
			direction -= 6.2831855f;
	} else {
		f32 diff = mDirection - direction;
		if (6.2831855f - diff < diff)
			direction += 6.2831855f;
	}

	return __fabsf(mDirection - direction);
}

JGeometry::TVec3<f32> TDirectionCalc::calcDirectionVector()
{
	f32 z = cosf(mDirection);
	f32 angle = mDirection;
	f32 x = sinf(angle);
	return JGeometry::TVec3<f32>(x, 0.0f, z);
}

void TDirectionCalc::makeDirection(JGeometry::TVec3<f32> direction)
{
	f32 z = direction.z;
	f32 x = direction.x;
	mDirection = atan2f(x, z);
}

f32 TDirectionCalc::calcTurnDirection(f32 direction, f32 maxTurn)
{
	mDirection = JGeometry::TUtil<f32>::zero()
	             + std::fmodf(
	                 6.2831855f
	                     + (mDirection - JGeometry::TUtil<f32>::zero()),
	                 6.2831855f);
	mDirection = JGeometry::TUtil<f32>::zero()
	             + JGeometry::TUtil<f32>::mod(
	                 6.2831855f
	                     + (mDirection - JGeometry::TUtil<f32>::zero()),
	                 6.2831855f);

	if (direction >= mDirection) {
		f32 diff = direction - mDirection;
		if (6.2831855f - diff < diff)
			direction -= 6.2831855f;
	} else {
		f32 diff = mDirection - direction;
		if (6.2831855f - diff < diff)
			direction += 6.2831855f;
	}

	f32 turn = maxTurn;
	if (direction > mDirection) {
		f32 diff = direction - mDirection;
		if (diff < turn)
			turn = diff;
		return mDirection + turn;
	}

	f32 diff = mDirection - direction;
	if (diff < turn)
		turn = diff;
	return mDirection - turn;
}

f32 TDirectionCalc::sub(f32 direction)
{
	mDirection = JGeometry::TUtil<f32>::zero()
	             + JGeometry::TUtil<f32>::mod(
	                 6.2831855f
	                     + (mDirection - JGeometry::TUtil<f32>::zero()),
	                 6.2831855f);

	if (direction >= mDirection) {
		f32 diff = direction - mDirection;
		if (6.2831855f - diff < diff)
			direction -= 6.2831855f;
	} else {
		f32 diff = mDirection - direction;
		if (6.2831855f - diff < diff)
			direction += 6.2831855f;
	}

	return mDirection - direction;
}

#pragma dont_inline on
f32 TDirectionCalc::calcNearerDirection(f32 direction)
{
	mDirection = JGeometry::TUtil<f32>::zero()
	             + std::fmodf(
	                 6.2831855f
	                     + (mDirection - JGeometry::TUtil<f32>::zero()),
	                 6.2831855f);

	if (direction >= mDirection) {
		f32 diff = direction - mDirection;
		if (6.2831855f - diff < diff)
			direction -= 6.2831855f;
	} else {
		f32 diff = mDirection - direction;
		if (6.2831855f - diff < diff)
			direction += 6.2831855f;
	}

	return direction;
}
#pragma dont_inline off

TKoopaJrParams::TKoopaJrParams(const char* path)
    : TSpineEnemyParams(path)
    , PARAM_INIT(mSLLaunchKillerLimit, 10000.0f)
    , PARAM_INIT(mSLDamageRadius, 1000.0f)
    , PARAM_INIT(mSLDamageHeight, 4000.0f)
    , PARAM_INIT(mSLKoopaJrScale, 1.6f)
    , PARAM_INIT(mSLFastLaunchDistance, 10000.0f)
    , PARAM_INIT(mSLDamagePeriod, 360)
    , PARAM_INIT(mSLLaunchKillerPeriod, 1200)
    , PARAM_INIT(mSLLaunchKillerPeriodFast, 360)
{
	TParams::load(mPrmPath);

	mSLLaunchKillerLimit.set(4200.0f);
	mSLDamageRadius.set(100.0f);
	mSLDamageHeight.set(300.0f);
	mSLKoopaJrScale.set(2.0f);
	mSLFastLaunchDistance.set(4600.0f);
	mSLDamagePeriod.set(240);
	mSLLaunchKillerPeriod.set(840);
	mSLLaunchKillerPeriodFast.set(360);
}

TKoopaJrSubmarineParams::TKoopaJrSubmarineParams(const char* path)
    : TSpineEnemyParams(path)
    , PARAM_INIT(killerTargetDistanceMin, 500.0f)
    , PARAM_INIT(killerTargetDistance, 500.0f)
    , PARAM_INIT(bottomHeight, 0.0f)
    , PARAM_INIT(centerZ, 0.0f)
    , PARAM_INIT(aboidKoopaFlameAngle, 0.31415927f)
    , PARAM_INIT(traceMarioAngle, 0.31415927f)
    , PARAM_INIT(mSLWavePhaseVelocity, 0.31415927f)
    , PARAM_INIT(mSLWaveAmplitudeMin, 0.37699112f)
    , PARAM_INIT(mSLWaveAmplitudeMaxLaunch, 0.37699112f)
    , PARAM_INIT(mSLWaveAmplitudeMax, 0.37699112f)
    , PARAM_INIT(mSLSwingPhaseVelocity, 0.31415927f)
    , PARAM_INIT(mSLSwingAmplitudeMin, 0.37699112f)
    , PARAM_INIT(mSLSwingAmplitudeMax, 0.37699112f)
    , PARAM_INIT(mSLRoundAngleVelocity, 0.05f)
    , PARAM_INIT(mSLRoundDistance, 1.0f)
    , PARAM_INIT(mSLAcceleration, 1.0f)
    , PARAM_INIT(mSLRotationSpeed, 1.0f)
    , PARAM_INIT(mSLSpeedMax, 8.0f)
    , PARAM_INIT(mSLKoopaJrSubmarineScale, 1.6f)
    , PARAM_INIT(mSLDamageRadius, 1000.0f)
    , PARAM_INIT(mSLDamageHeight, 4000.0f)
    , PARAM_INIT(shineKillerProbability0, 0.0f)
    , PARAM_INIT(shineKillerProbability1, 0.0f)
    , PARAM_INIT(mSLKillerIntervalFast, 30)
    , PARAM_INIT(mSLKillerInterval, 30)
{
	TParams::load(mPrmPath);

	killerTargetDistanceMin.set(500.0f);
	killerTargetDistance.set(700.0f);
	bottomHeight.set(0.0f);
	centerZ.set(200.0f);
	aboidKoopaFlameAngle.set(0.62831855f);
	traceMarioAngle.set(0.31415927f);
	mSLWavePhaseVelocity.set(0.09424778f);
	mSLWaveAmplitudeMin.set(0.12566371f);
	mSLWaveAmplitudeMaxLaunch.set(0.37699112f);
	mSLWaveAmplitudeMax.set(0.31415927f);
	mSLSwingPhaseVelocity.set(0.18849556f);
	mSLSwingAmplitudeMin.set(0.12566371f);
	mSLSwingAmplitudeMax.set(0.5654867f);
	mSLRoundAngleVelocity.set(0.12f);
	mSLRoundDistance.set(2000.0f);
	mSLAcceleration.set(1.0f);
	mSLRotationSpeed.set(1.0f);
	mSLSpeedMax.set(5.0f);
	mSLKoopaJrSubmarineScale.set(2.0f);
	mSLDamageRadius.set(240.0f);
	mSLDamageHeight.set(120.0f);
	shineKillerProbability0.set(0.5f);
	shineKillerProbability1.set(0.125f);
	mSLKillerIntervalFast.set(30);
	mSLKillerInterval.set(90);
}

TKoopaJrManager::TKoopaJrManager(const char* name)
    : TEnemyManager(name)
{
}

TSpineEnemy* TKoopaJrManager::createEnemyInstance() { return nullptr; }

void TKoopaJrManager::loadAfter()
{
	static const char* onetimeFilenames[] = {
		"/scene/koopajr/jpa/ms_koopajr_killer.jpa",
	};
	if (!gParticleFlagLoaded[0xef]) {
		gpResourceManager->load(onetimeFilenames[0], 0xef);
		gParticleFlagLoaded[0xef] = true;
	}
}

void TKoopaJrManager::load(JSUMemoryInputStream& stream)
{
	TEnemyManager::load(stream);
	unk38 = new TKoopaJrParams("/enemy/koopajr.prm");
}

void TKoopaJrManager::createModelData()
{
	static const TModelDataLoadEntry entry[] = {
		{ "koopajr_model.bmd", 0x54220000, 0 },
		{ nullptr, 0, 0 },
	};
	createModelDataArray(entry);
}

TKoopaJrSubmarineManager::TKoopaJrSubmarineManager(const char* name)
    : TEnemyManager(name)
{
}

TSpineEnemy* TKoopaJrSubmarineManager::createEnemyInstance()
{
	return new TKoopaJrSubmarine("クッパジュニアサブマリン");
}

#define ASSERT_MSG(msg, line) (void)((msg), (line))
#define ASSERT_TEST(expr)                                                      \
	(void)((expr) ? true : (ASSERT_MSG(__FILE__, __LINE__), false));

void TKoopaJrSubmarineManager::loadAfter()
{
	JDrama::TNameRef::loadAfter();
	if (!unk38)
		return;
}

void TKoopaJrSubmarineManager::load(JSUMemoryInputStream& stream)
{
	ASSERT_TEST(unk38);
	TEnemyManager::load(stream);
	unk38 = new TKoopaJrSubmarineParams("/enemy/koopajrsubmarine.prm");
	ASSERT_TEST(unk38);
}

void TKoopaJrSubmarineManager::createModelData()
{
	static const TModelDataLoadEntry entry[] = {
		{ "LastKoopaJrSubmarine.bmd", 0x54220000, 0 },
		{ nullptr, 0, 0 },
	};
	createModelDataArray(entry);
}

TKoopaJr::TKoopaJr(const char* name)
    : TSpineEnemy(name)
    , unk15C(nullptr)
    , unk160(nullptr)
    , unk164(nullptr)
    , unk168(nullptr)
    , unk16C(nullptr)
{
	mLiveFlag |= 0x10;
	mLiveFlag &= ~0x100;
}

BOOL TKoopaJr::receiveMessage(THitActor* sender, u32 message)
{
	if (message == HIT_MESSAGE_SPRAYED_BY_WATER) {
		gpMarioParticleManager->emit(0xE7, &sender->mPosition, 0, nullptr);
		gpMSound->startSoundSet(0x6802, &mPosition, 0, 0.0f, 0, 0, 4);

		unk150 = getSaveParam2()->mSLDamagePeriod.get();

		if (mSpine->getCurrentNerve() == &TNerveKoopaJrWait::theNerve())
			mSpine->pushNerve(&TNerveKoopaJrDamage::theNerve());

		if (mSpine->getCurrentNerve() == &TNerveKoopaJrLaunch::theNerve()
		    || mSpine->getCurrentNerve() == &TNerveKoopaJrYahoo::theNerve())
			mSpine->setNext(&TNerveKoopaJrDamage::theNerve());

		return TRUE;
	}

	return FALSE;
}

void TKoopaJr::perform(u32 flags, JDrama::TGraphics* graphics)
{
	if (!unk164) {
		unk164        = (TKoopaJrSubmarine*)unk168->getObj(0);
		unk164->unk1A0 = this;
	}

	if (!unk160) {
		TKoopaManager* manager
		    = JDrama::TNameRefGen::search<TKoopaManager>("クッパマネージャー");
		unk160 = (TKoopa*)manager->getObj(0);
	}

	if (!unk15C)
		unk15C = JDrama::TNameRefGen::search<TBathtub>("バスタブ");

	if (flags & 2) {
		f32 limit
		    = 0.5f * unk164->getSaveParam2()->mSLSwingAmplitudeMax.get();
		if (unk164->unk190 >= limit) {
			unk150 = getSaveParam2()->mSLDamagePeriod.get();

			if (mSpine->getCurrentNerve() == &TNerveKoopaJrWait::theNerve())
				mSpine->pushNerve(&TNerveKoopaJrDamage::theNerve());

			if (mSpine->getCurrentNerve() == &TNerveKoopaJrLaunch::theNerve()
			    || mSpine->getCurrentNerve()
			        == &TNerveKoopaJrYahoo::theNerve()) {
				mSpine->becomeNerveAfterPop(&TNerveKoopaJrDamage::theNerve());
			}
		}
	}

	if (flags & 1) {
		if (unk150 > 0)
			--unk150;
		if (unk154 > 0)
			--unk154;
		if (unk158 > 0)
			--unk158;

		if (!*((u8*)unk15C + 0x29A)) {
			if (mSpine->getCurrentNerve() == &TNerveKoopaJrWait::theNerve()) {
				checkNerveKillerLaunchNormal();
				checkNerveKillerLaunchFast();
				checkNerveKillerHit();
			}

			JGeometry::TVec3<f32> toMario;
			toMario.sub(*gpMarioPos, mPosition);
			toMario.y = 0.0f;
			mRotation.y = 180.0f * atan2f(toMario.z, toMario.x) / 3.1415927f;
		}
	}

	TSpineEnemy::perform(flags, graphics);
}

const char** TKoopaJr::getBasNameTable() const { return koopajr_bastable; }

void TKoopaJr::reset()
{
	TSpineEnemy::reset();
	mSpine->reset();
	unk150 = 0;
	unk154 = 0;
	unk158 = 0;
	unk154 = getSaveParam2()->mSLLaunchKillerPeriod.get();
	unk158 = 0;
}

void TKoopaJr::init(TLiveManager* manager)
{
	mManager = manager;
	mManager->manageActor(this);

	mMActorKeeper = new TMActorKeeper(mManager, 1);
	mMActor       = mMActorKeeper->createMActor("koopajr_model.bmd", 0);
	mMActor->setLightType(1);

	initAnmSound();

	f32 damageHeight = getSaveParam2()->mSLDamageHeight.get();
	f32 damageRadius = getSaveParam2()->mSLDamageRadius.get();
	initHitActor(0x8000028, 1, 0, 0.0f, 0.0f, damageRadius,
	             damageHeight);
	offHitFlag(1);

	mSpine->initWith(&TNerveKoopaJrWait::theNerve());

	const char* killerManagerName = "バスタブキラーマネージャー";
	JDrama::TNameRef* root
	    = JDrama::TNameRefGen::getInstance()->getRootNameRef();
	unk16C = (TEnemyManager*)root->searchF(
	    JDrama::TNameRef::calcKeyCode(killerManagerName), killerManagerName);

	if (!unk168) {
		const char* submarineManagerName
		    = "クッパジュニアサブマリンマネージャー";
		root = JDrama::TNameRefGen::getInstance()->getRootNameRef();
		unk168 = (TEnemyManager*)root->searchF(
		    JDrama::TNameRef::calcKeyCode(submarineManagerName),
		    submarineManagerName);
	}

	f32 scale = getSaveParam2()->mSLKoopaJrScale.get();
	mScaling.x = scale;
	mScaling.y = scale;
	mScaling.z = scale;

	mSpine->reset();
	unk150 = 0;
	unk154 = 0;
	unk158 = 0;
	unk154 = getSaveParam2()->mSLLaunchKillerPeriod.get();
	unk158 = 0;
}

void TKoopaJr::calcRootMatrix()
{
	J3DModel* model = getModel();
	if (unk15C->getUnk29A()) {
		MtxPtr mtx       = unk15C->getKoopaJrMtxInDemo();
		J3DModel* model2 = getModel();
		PSMTXCopy(mtx, model2->unk20);
	} else {
		unk164->getJointTransByIndex(TKoopaJr_jointIndexTable[0], &mPosition);
		MsMtxSetXYZRPH(model->getBaseTRMtx(), mPosition.x, mPosition.y,
		                mPosition.z, mRotation.x, mRotation.y, mRotation.z);
	}
	model->setBaseScale(mScaling);
}

void TKoopaJr::checkNerveKillerLaunchFast()
{
	if (unk158 > 0)
		return;

	int num = unk15C->getNumKillerBurstable();
	if (num == 0)
		return;
	TKoopaJrSubmarine* submarine = unk164;
	if (num > 8)
		num = 8;

	submarine->unk180 = 0;
	submarine->unk184 = num;
	for (int i = 0; i < submarine->unk184; ++i)
		submarine->unk178[i] = 2;

	if (submarine->appearShineKiller(submarine->unk184))
		submarine->unk178[submarine->unk184 - 1] = 1;

	mSpine->pushNerve(&TNerveKoopaJrLaunch::theNerve());
	unk164->mSpine->pushNerve(
	    &TNerveKoopaJrSubmarineCannonOpenClose::theNerve());
}

void TKoopaJr::checkNerveKillerLaunchNormal()
{
	if (unk154 > 0)
		return;

	int num = unk15C->getNumKillerLaunchable();
	if (num == 0)
		return;
	TKoopaJrSubmarine* submarine = unk164;
	if (num > 8)
		num = 8;

	submarine->unk180 = 0;
	submarine->unk184 = num;
	for (int i = 0; i < submarine->unk184; ++i)
		submarine->unk178[i] = 0;

	if (submarine->appearShineKiller(submarine->unk184))
		submarine->unk178[submarine->unk184 - 1] = 1;

	mSpine->pushNerve(&TNerveKoopaJrLaunch::theNerve());
	unk164->mSpine->pushNerve(
	    &TNerveKoopaJrSubmarineCannonOpenClose::theNerve());
}

void TKoopaJr::checkNerveKillerHit()
{
	for (int i = 0; i < unk16C->getActiveObjNum(); ++i) {
		TBathtubKiller* killer = (TBathtubKiller*)unk16C->getObj(i);
		if (killer->unk21C == 1) {
			mSpine->pushNerve(&TNerveKoopaJrYahoo::theNerve());
			return;
		}
	}
}

#pragma dont_inline on
TKoopaJrSubmarine::TKoopaJrSubmarine(const char* name)
    : TSpineEnemy(name)
    , unk164(0.0f)
    , unk16C(0.0f)
    , unk188(0.0f)
    , unk1A0(nullptr)
{
	mLiveFlag &= ~0x10;
	mLiveFlag &= ~0x100;
}
#pragma dont_inline off

const char** TKoopaJrSubmarine::getBasNameTable() const
{
	return koopajrsubmarine_bastable;
}

BOOL TKoopaJrSubmarine::receiveMessage(THitActor* sender, u32 message)
{
	if (message == HIT_MESSAGE_SPRAYED_BY_WATER) {
		gpMarioParticleManager->emit(0xE7, &sender->mPosition, 0, nullptr);
		gpMSound->startSoundSet(0x6802, &mPosition, 0, 0.0f, 0, 0, 4);
		unk18C = 1;
		return TRUE;
	}

	return FALSE;
}

void TKoopaJrSubmarine::calcRootMatrix()
{
	if (unk1A0->unk15C->getUnk29A()) {
		MtxPtr mtx = unk1A0->unk15C->getSubmarineMtxInDemo();
		PSMTXCopy(mtx, getModel()->getBaseTRMtx());
	} else {
		f32 swingHalf = 0.5f * (unk190 * sinf(unk194));
		f32 swingSin  = sinf(swingHalf);
		f32 swingCos  = cosf(swingHalf);

		f32 waveHalf = 0.5f * (unk198 * sinf(unk19C));
		f32 waveSin  = sinf(waveHalf);
		f32 waveCos  = cosf(waveHalf);

		f32 yawHalf = 0.5f * unk16C.mDirection;
		f32 yawSin  = sinf(yawHalf);
		f32 yawCos  = cosf(yawHalf);

		JGeometry::TQuat4<f32> q;
		q.x = yawSin * swingSin;
		q.y = yawSin * swingCos;
		q.z = yawCos * swingSin;
		q.w = yawCos * swingCos;

		JGeometry::TQuat4<f32> finalQ;
		finalQ.x = q.x * waveCos + q.w * waveSin;
		finalQ.y = q.y * waveCos + q.z * waveSin;
		finalQ.z = q.z * waveCos - q.y * waveSin;
		finalQ.w = q.w * waveCos - q.x * waveSin;

		f32 center = getSaveParam2()->centerZ.get();
		JGeometry::TVec3<f32> centerOffset(0.0f, 0.0f, center);
		TPosition3f centerMtx;
		centerMtx.translation(centerOffset);

		JGeometry::TVec3<f32> origin = centerOffset;
		origin.negate();
		origin.add(mPosition);

		TPosition3f mtx;
		mtx.setQuat(finalQ);
		mtx.setTrans(origin);
		mtx.concat(centerMtx);

		PSMTXCopy(mtx.mMtx, getModel()->getBaseTRMtx());
	}

	getModel()->setBaseScale(mScaling);
}

void TKoopaJrSubmarine::checkNerve()
{
	if (unk1A0->mSpine->getCurrentNerve() == &TNerveKoopaJrWait::theNerve()) {
		makeRelativeAngle();
		unk168 = getSaveParam2()->mSLRoundDistance.get();
		makeRoundVelocity();
	}

	mVelocity.x *= 0.95f;
	mVelocity.y *= 0.95f;
	mVelocity.z *= 0.95f;

	if (!unk170) {
		JGeometry::TVec3<f32> direction = mVelocity;
		direction.normalize();
		unk16C.mDirection = unk16C.calcTurnDirection(
		    atan2f(direction.z, direction.x),
		    getSaveParam2()->mSLRotationSpeed.get() * 3.1415927f / 180.0f);
	}

	if (mSpine->getCurrentNerve() == &TNerveKoopaJrSubmarineWait::theNerve())
		return;

	if (mSpine->getCurrentNerve()
	    == &TNerveKoopaJrSubmarineCannonOpenClose::theNerve()) {
		if (unk180 != 0)
			return;

		J3DFrameCtrl* ctrl = mMActor->getFrameCtrl(0);
		if (!ctrl->checkPass(30.0f))
			return;

		ctrl->setRate(0.0f);
		mSpine->pushNerve(&TNerveKoopaJrSubmarineLaunchKiller::theNerve());
		return;
	}

	(void)&TNerveKoopaJrSubmarineLaunchKiller::theNerve();
}

#pragma dont_inline on
namespace JGeometry {
f32 TUtil<f32>::mod(f32 value, f32 modulus)
{
	if (__fabsf(modulus) > __fabsf(value))
		return value;

	return value - modulus * (f32)(s64)(value / modulus);
}
} // namespace JGeometry
#pragma dont_inline off

void TKoopaJrSubmarine::makeRoundVelocity()
{
	JGeometry::TVec3<f32> round = unk164.calcDirectionVector();
	round.scale(unk168);

	JGeometry::TVec3<f32> target = unk1A0->unk15C->mPosition;
	target.add(round);

	JGeometry::TVec3<f32> toTarget(target.x - mPosition.x, 0.0f,
	                               target.z - mPosition.z);
	f32 dist = JGeometry::TUtil<f32>::sqrt(toTarget.squared());
	if (dist < 100.0f) {
		unk170 = 1;
		return;
	}

	unk170 = 0;
	toTarget.normalize();
	toTarget.scale(getSaveParam2()->mSLAcceleration.get());
	mVelocity.add(toTarget);

	f32 speed = JGeometry::TUtil<f32>::sqrt(mVelocity.squared());
	f32 max   = getSaveParam2()->mSLSpeedMax.get();
	if (speed > max) {
		mVelocity.normalize();
		mVelocity.scale(max);
	}
}

void TKoopaJrSubmarine::makeRelativeAngle()
{
	f32 flameAngle
	    = unk1A0->unk160->getFlameDirDegree() * 3.1415927f / 180.0f;
	f32 nearerFlame = unk164.calcNearerDirection(flameAngle);
	f32 flameDiff   = __fabsf(unk164.mDirection - nearerFlame);

	JGeometry::TVec3<f32> marioDiff;
	marioDiff.sub(*gpMarioPos, unk1A0->unk15C->mPosition);
	marioDiff.y = 0.0f;

	TDirectionCalc marioDirection;
	marioDirection.makeDirection(marioDiff);
	f32 marioAngle  = marioDirection.mDirection;
	f32 nearerMario = unk164.calcNearerDirection(marioAngle);
	f32 current     = unk164.mDirection;
	f32 marioAbs    = __fabsf(current - nearerMario);

	if (unk1A0->unk160->isFlaming()
	    && flameDiff <= getSaveParam2()->aboidKoopaFlameAngle.get()) {
		current = flameAngle + 3.1415927f;
	} else if (marioAbs > getSaveParam2()->traceMarioAngle.get()) {
		current = marioAngle;
	}

	f32 turn = getSaveParam2()->mSLRoundAngleVelocity.get() * 0.017453294f;
	unk164.mDirection
	    = JGeometry::TUtil<f32>::zero()
	      + JGeometry::TUtil<f32>::mod(
	          6.2831855f
	              + (unk164.mDirection - JGeometry::TUtil<f32>::zero()),
	          6.2831855f);
	f32 nearer = unk164.calcNearerDirection(current);
	f32 direction = unk164.mDirection;
	if (nearer > direction) {
		f32 diff = nearer - direction;
		if (diff < turn)
			turn = diff;
		direction += turn;
	} else {
		f32 diff = direction - nearer;
		if (diff < turn)
			turn = diff;
		direction -= turn;
	}
	unk164.mDirection = direction;
}

bool TKoopaJrSubmarine::appearShineKiller(int)
{
	f32 probability;

	if (((TWaterGun*)SMS_GetMarioWaterGun())->mCurrentWater == 0) {
		probability = 0.5f;
	} else {
		s32 shineFlag = TFlagManager::smInstance->getFlag(0x20001);
		TBathtubKillerManager* manager
		    = (TBathtubKillerManager*)unk1A0->unk16C;
		if (manager->unk60 == shineFlag) {
			probability = 0.5f;
		} else {
			TWaterGun* waterGun = (TWaterGun*)SMS_GetMarioWaterGun();
			s32 amountMax
			    = waterGun->getCurrentNozzle()->mEmitParams.mAmountMax.get();
			s32 currentWater = ((TWaterGun*)SMS_GetMarioWaterGun())->mCurrentWater;
			f32 min = getSaveParam2()->shineKillerProbability0.get();
			f32 max = getSaveParam2()->shineKillerProbability1.get();
			probability
			    = ((f32)currentWater / (f32)amountMax) * (max - min) + min;
		}
	}

	bool result = false;
	if (rand() * 0.000030517578f < probability)
		result = true;
	return result;
}

void TKoopaJrSubmarine::makeKillerVelocity(TBathtubKiller* killer,
                                           JGeometry::TVec3<f32> direction)
{
	int queuedType = unk178[unk180];
	if (queuedType == 2) {
		direction.set(0.0f, 1.0f, 0.0f);

		JGeometry::TVec3<f32> toMario = *gpMarioPos;
		toMario.sub(killer->mPosition);
		toMario.y = 0.0f;
		toMario.normalize();

		JGeometry::TVec3<f32> axis;
		axis.cross(direction, toMario);
		axis.normalize();

		f32 angle = 0.31415927f;
		f32 s     = sinf(angle);
		f32 c     = cosf(angle);
		f32 dot   = axis.dot(direction);
		f32 invC  = 1.0f - c;
		JGeometry::TVec3<f32> cross;
		cross.cross(axis, direction);
		direction.set(direction.x * c + cross.x * s + axis.x * dot * invC,
		              direction.y * c + cross.y * s + axis.y * dot * invC,
		              direction.z * c + cross.z * s + axis.z * dot * invC);

		int slot     = unk180 % 4;
		f32 fanAngle = 0.62831855f;
		if (slot == 0) {
			fanAngle = -0.15707964f;
		} else if (slot == 1) {
			fanAngle = 0.15707964f;
		} else if (slot == 2) {
			fanAngle = -0.31415927f;
		} else if (slot == 3) {
			fanAngle = 0.31415927f;
		}

		angle = 0.5f * fanAngle;
		s     = sinf(angle);
		c     = cosf(angle);
		dot   = toMario.dot(direction);
		invC  = 1.0f - c;
		cross.cross(toMario, direction);
		direction.set(direction.x * c + cross.x * s + toMario.x * dot * invC,
		              direction.y * c + cross.y * s + toMario.y * dot * invC,
		              direction.z * c + cross.z * s + toMario.z * dot * invC);

		direction.normalize();
		direction.scale(killer->unk1A4);
	} else {
		direction.normalize();
		direction.scale(killer->unk1A4);

		JGeometry::TVec3<f32> target = *gpMarioPos;
		Mtx* rootMtx                = unk1A0->unk15C->getRootJointMtx();
		target.y                    = (*rootMtx)[1][3];

		f32 dx       = target.x - killer->mPosition.x;
		f32 dz       = target.z - killer->mPosition.z;
		f32 distSq   = dx * dx + dz * dz;
		f32 distance = JGeometry::TUtil<f32>::sqrt(distSq)
		               - getSaveParam2()->killerTargetDistance.get();
		f32 offset = getSaveParam2()->killerTargetDistanceMin.get();
		if (distance >= offset)
			offset = distance;

		JGeometry::TVec3<f32> toTarget(dx, 0.0f, dz);
		toTarget.normalize();

		JGeometry::TVec3<f32> jumpTarget;
		jumpTarget.x = killer->mPosition.x + toTarget.x * offset;
		jumpTarget.y = target.y;
		jumpTarget.z = killer->mPosition.z + toTarget.z * offset;

		direction = calcVelocityToJumpToY(
		    jumpTarget, direction.y,
		    killer->getSaveParam2()->mSLFlyingGravityY.get());
	}

	killer->makeInitialVelocity(direction);
}

void TKoopaJrSubmarine::launchKiller()
{
	int jointSlot = unk180 % 4;
	TBathtubKiller* killer
	    = (TBathtubKiller*)unk1A0->unk16C->getDeadEnemy();
	if (!killer)
		return;

	killer->unk194 = unk178[unk180];
	killer->reset();

	int jointIndexSlot = jointSlot + 1;
	int jointIndex     = TKoopaJr_jointIndexTable[jointIndexSlot];
	MtxPtr mtx         = getModel()->getAnmMtx(jointIndex);
	killer->mPosition.set(mtx[0][3], mtx[1][3], mtx[2][3]);

	JGeometry::TVec3<f32> direction;
	direction.x = mtx[0][2];
	direction.y = mtx[1][2];
	direction.z = mtx[2][2];
	makeKillerVelocity(killer, direction);

	if (gpMSound->gateCheck(0x285D))
		MSoundSESystem::MSoundSE::startSoundActor(
		    0x285D, &killer->mPosition, 0, nullptr, 0, 4);
}

void TKoopaJrSubmarine::moveSwing()
{
	if (unk18C) {
		unk18C = 0;
		unk190 += 0.06283186f;
		f32 swing = unk190;
		f32 maxSwing = getSaveParam2()->mSLSwingAmplitudeMax.get();
		unk190 = maxSwing >= swing ? swing : maxSwing;
	}

	unk190 -= 0.009424779f;
	f32 swing = unk190;
	f32 minSwing = getSaveParam2()->mSLSwingAmplitudeMin.get();
	unk190 = minSwing >= swing ? minSwing : swing;

	if (unk190 <= 0.0f)
		unk194 = 0.0f;

	unk194 = std::fmodf(6.2831855f
	                        + (unk194
	                           + getSaveParam2()->mSLSwingPhaseVelocity.get()
	                           - 0.0f),
	                    6.2831855f)
	         + 0.0f;

	f32 speed = JGeometry::TUtil<f32>::sqrt(mVelocity.squared());
	f32 ratio = speed / getSaveParam2()->mSLSpeedMax.get();

	if (unk150 > 0) {
		unk198 += 0.03141593f;
		f32 wave = unk198;
		f32 maxLaunchWave = getSaveParam2()->mSLWaveAmplitudeMaxLaunch.get();
		unk198 = maxLaunchWave >= wave ? wave : maxLaunchWave;
	}

	if (ratio > 0.5f) {
		unk198 += 0.03141593f;
		f32 wave = unk198;
		f32 maxWave = getSaveParam2()->mSLWaveAmplitudeMax.get();
		unk198 = maxWave >= wave ? wave : maxWave;
	}

	unk198 -= 0.018849557f;
	f32 wave = unk198;
	f32 minWave = getSaveParam2()->mSLWaveAmplitudeMin.get();
	unk198 = minWave >= wave ? minWave : wave;

	if (unk198 <= 0.0f)
		unk19C = 0.0f;

	unk19C = std::fmodf(6.2831855f
	                        + (unk19C
	                           + getSaveParam2()->mSLWavePhaseVelocity.get()
	                           - 0.0f),
	                    6.2831855f)
	         + 0.0f;
}

void TKoopaJrSubmarine::makeCollisionPositions()
{
	f32 x = 0.0f;
	f32 y = 0.0f;
	f32 z = 0.0f;

	for (int i = 0; i < 2; ++i) {
		MtxPtr mtx = getModel()->getAnmMtx(TKoopaJr_jointIndexTable[i + 3]);
		x += mtx[0][3];
		y += mtx[1][3];
		z += mtx[2][3];
	}

	x *= 0.5f;
	y *= 0.5f;
	z *= 0.5f;
	unk1A4->mPosition.set(x, y, z);
	getJointTransByIndex(TKoopaJr_jointIndexTable[0], &unk1A8->mPosition);
}

void TKoopaJrSubmarine::bind()
{
	JGeometry::TVec3<f32> nextPosition;
	nextPosition = mPosition;
	nextPosition.add(mLinearVelocity);
	nextPosition.add(mVelocity);

	mLinearVelocity = nextPosition - mPosition;
	unk174->bind(this);
}

void TKoopaJrSubmarine::perform(u32 flags, JDrama::TGraphics* graphics)
{
	if (flags & 2) {
		if (mSpine->getCurrentNerve()
		    == &TNerveKoopaJrSubmarineLaunchKiller::theNerve()) {
			if (unk180 < unk184 && unk150 <= 0) {
				launchKiller();
				s32 killerType = unk178[unk180];
				if (killerType == 2)
					unk150 = getSaveParam2()->mSLKillerIntervalFast.get();
				else
					unk150 = getSaveParam2()->mSLKillerInterval.get();
				++unk180;
			}
		}

		makeCollisionPositions();
		moveSwing();
	}

	if (flags & 1) {
		if (unk150 > 0)
			--unk150;

		checkNerve();
	}

	TSpineEnemy::perform(flags, graphics);
	unk1A4->perform(flags, graphics);
	unk1A8->perform(flags, graphics);
}

void TKoopaJrSubmarine::init(TLiveManager* manager)
{
	mManager = manager;
	mManager->manageActor(this);
	initAnmSound();

	f32 damageHeight = getSaveParam2()->mSLDamageHeight.get();
	f32 damageRadius = getSaveParam2()->mSLDamageRadius.get();
	initHitActor(0x8000020, 0, 0, 0.0f, 0.0f, damageRadius,
	             damageHeight);
	onHitFlag(1);

	unk1A4 = new TCallbackHitActor("サブマリンリアボディ", this);
	unk1A4->initHitActor(0x800002D, 0, 0, 0.0f, 0.0f, damageRadius,
	                      damageHeight);
	unk1A4->offHitFlag(1);
	JDrama::TNameRefGen::search<TIdxGroupObj>("敵グループ")->add(unk1A4);

	unk1A8 = new TCallbackHitActor("サブマリンフロントボディ", this);
	unk1A8->initHitActor(0x800002D, 0, 0, 0.0f, 0.0f, damageRadius,
	                      damageHeight);
	unk1A8->offHitFlag(1);
	JDrama::TNameRefGen::search<TIdxGroupObj>("敵グループ")->add(unk1A8);

	mMActorKeeper = new TMActorKeeper(mManager, 1);
	mMActor       = mMActorKeeper->createMActor("LastKoopaJrSubmarine.bmd", 0);
	mMActor->setLightType(1);

	mSpine->initWith(&TNerveKoopaJrSubmarineWait::theNerve());

	JUTNameTab* jointNames = getModel()->mModelData->getJointName();
	for (int i = 0; i < 5; ++i)
		TKoopaJr_jointIndexTable[i]
		    = jointNames->getIndex(TKoopaJr_jointNameTable[i]);

	f32 scale = getSaveParam2()->mSLKoopaJrSubmarineScale.get();
	mScaling.x = scale;
	mScaling.y = scale;
	mScaling.z = scale;

	unk174  = new TBathtubBinder();
	mBinder = unk174;
	resetKoopaJrSubmarine();
}

void TKoopaJrSubmarine::resetKoopaJrSubmarine()
{
	mSpine->reset();
	unk150 = 0;
	mMActor->setBckFromIndex(0);

	const char** table = getBasNameTable();
	setAnmSound(!table ? nullptr : table[0]);

	unk188 = mMActor->getFrameCtrl(0)->getRate();
	unk180 = 0;
	unk184 = 0;
	for (int i = 0; i < 8; ++i)
		unk178[i] = 0;

	unk154 = 0.0f;
	unk158 = 0.0f;
	unk15C = 0.0f;
	unk160 = 1.0f;
	unk16C.mDirection = 0.0f;
	unk164.mDirection = 0.0f;
	unk170 = 0;
	unk18C = 0;
	unk190 = 0.0f;
	unk194 = 0.0f;
	unk198 = 0.0f;
	unk19C = 0.0f;

	f32 bottomHeight = getSaveParam2()->bottomHeight.get();
	unk174->init(100.0f, 3.1415927f, 100.0f, 3.1415927f, bottomHeight);
}

void TKoopaJrSubmarine::reset()
{
	TSpineEnemy::reset();
	resetKoopaJrSubmarine();
}

BOOL TCallbackHitActor::receiveMessage(THitActor* sender, u32 message)
{
	return unk68->receiveMessage(sender, message);
}
