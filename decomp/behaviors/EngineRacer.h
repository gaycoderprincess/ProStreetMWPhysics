class EngineRacer : public VehicleBehavior {
  public:
	struct Clutch {
	  public:
		enum State { ENGAGED, ENGAGING, DISENGAGED };

		Clutch() {
			mState = ENGAGED;
			mTime = 0.0f;
			mEngageTime = 0.0f;
			mShiftingUp = false;
		}

		void Disengage() {
			if (mState == ENGAGED) {
				mState = DISENGAGED;
			}
		}

		void Engage(float time) {
			if (mState == DISENGAGED) {
				if (time > 0.0f) {
					mState = ENGAGING;
				} else {
					mState = ENGAGED;
				}
				mTime = time;
				mEngageTime = time;
			}
		}

		void Reset() {
			mState = ENGAGED;
			mTime = 0.0f;
			mShiftingUp = false;
		}

		float Update(float dT) {
			if (mTime > 0.0f) {
				mTime -= dT;
				if (mTime <= 0.0f && mState == ENGAGING) {
					mState = ENGAGED;
				}
			}

			switch (mState) {
				case DISENGAGED:
					return 0.25f;
				case ENGAGING:
					return 1.0f - UMath::Ramp(mTime, 0.0f, mEngageTime) * 0.75f;
				case ENGAGED:
					return 1.0f;
				default:
					return 1.0f;
			}
		}

		State GetState() {
			return mState;
		}

		State mState;
		float mTime;
		float mEngageTime;
		bool mShiftingUp;
	};

	ISimable* GetOwner() const {
		return Behavior::mIOwner;
	}

	IVehicle* GetVehicle() const {
		return mVehicle;
	}

	void Create(const BehaviorParams &bp);
	GearID GuessGear(float speed) const;
	float GuessRPM(float speed, GearID atgear) const;
	ShiftPotential FindShiftPotential(GearID gear, float rpm, float rpmFromGround) const;
	float GetDifferentialAngularVelocity(bool locked) const;
	float GetDriveWheelSlippage() const;
	void SetDifferentialAngularVelocity(float w);
	float CalcSpeedometer(float rpm, unsigned int gear) const;
	void LimitFreeWheels(float w);
	float GetBrakingTorque(float engine_torque, float rpm) const;
	void CalcShiftPoints();
	bool DoGearChange(GearID gear, bool automatic);
	void AutoShift(float dT);

	void OnOwnerAttached(IAttachable* pOther) { ENGINERACER_FUNCTION_LOG("OnOwnerAttached"); }
	void OnOwnerDetached(IAttachable* pOther) { ENGINERACER_FUNCTION_LOG("OnOwnerDetached"); }
	void OnPause() { ENGINERACER_FUNCTION_LOG("OnPause");  }
	void OnUnPause() { ENGINERACER_FUNCTION_LOG("OnUnPause");  }
	int GetPriority() { ENGINERACER_FUNCTION_LOG("GetPriority"); return mPriority; }

	Physics::Tunings* GetVehicleTunings() const {
		return GetVehicleMWTunings(GetVehicle());
	}

	void dtor(char a2);

	// IEngine
	void MatchSpeed(float speed);
	float GetHorsePower() const;

	// Behavior
	void Reset();
	void OnTaskSimulate(float dT);
	void OnBehaviorChange(const UCrc32 &mechanic);

	// ITransmission
	float GetSpeedometer() const;
	float GetMaxSpeedometer() const;
	float GetShiftPoint(GearID from_gear, GearID to_gear) const;

	// ITiptronic
	bool SportShift(GearID gear);

	// IEngineDamage
	void Sabotage(float time);
	bool Blow();

	// IRaceEngine
	// Credits: Brawltendo
	float GetPerfectLaunchRange(float &range) {
		// perfect launch only applies to first gear
		if (mGear != G_FIRST) {
			range = 0.0f;
			return 0.0f;
		} else {
			range = (mMWInfo->RED_LINE - mMWInfo->IDLE) * 0.25f;
			float upper_limit = mMWInfo->RED_LINE + 500.0f;
			return UMath::Min(mPeakTorqueRPM + range, upper_limit) - range;
		}
	}

	// IEngine
	float GetMaxHorsePower() const {
		return mMaxHP;
	}
	Hp GetMinHorsePower() const {
		return FTLB2HP(Physics::Info::Torque(mMWInfo, mMWInfo->IDLE) * mMWInfo->IDLE, 1.0f);
	}
	float GetRPM() const {
		return mRPM;
	}
	float GetMaxRPM() const {
		return mMWInfo->MAX_RPM;
	}
	float GetPeakTorqueRPM() const {
		return mPeakTorqueRPM;
	}
	float GetRedline() const {
		return mMWInfo->RED_LINE;
	}
	float GetMinRPM() const {
		return mMWInfo->IDLE;
	}
	float GetNOSCapacity() const {
		return mNOSCapacity;
	}
	float GetNOSBoost() const {
		return mNOSBoost;
	}
	bool IsNOSEngaged() const {
		return mNOSEngaged >= 1.0f;
	}
	bool HasNOS() const {
		return mMWInfo->NOS_CAPACITY > 0.0f && mMWInfo->TORQUE_BOOST > 0.0f;
	}
	float GetNOSFlowRate() const {
		return mMWInfo->FLOW_RATE;
	}

	void ChargeNOS(float charge) {
		if (HasNOS()) {
			mNOSCapacity = UMath::Clamp(mNOSCapacity + charge, 0.0f, 1.0f);
		}
	}

	bool IsEngineBraking() {
		return mEngineBraking;
	}
	bool IsShiftingGear() {
		return mGearShiftTimer > 0.0f;
	}
	bool IsReversing() const {
		return mGear == G_REVERSE;
	}

	// IInductable
	Physics::Info::eInductionType InductionType() const {
		return Physics::Info::InductionType(mMWInfo);
	}
	float GetInductionPSI() const {
		return mPSI;
	}
	float InductionSpool() const {
		return mSpool;
	}
	float GetMaxInductionPSI() const {
		return mMWInfo->PSI;
	}

	// IEngineDamage
	bool IsBlown() const {
		return mBlown;
	}
	void Repair() {
		mSabotage = 0.0f;
		mBlown = false;
	}
	bool IsSabotaged() const {
		return mSabotage > 0.0f;
	}

	// ITransmission
	float GetDriveTorque() const {
		return mDriveTorque;
	}
	GearID GetTopGear() const {
		return (GearID)(GetNumGearRatios() - 1);
	}
	GearID GetGear() const {
		return (GearID)mGear;
	}
	bool IsGearChanging() const {
		return mGearShiftTimer > 0.0f;
	}

	bool Shift(GearID gear) {
		return DoGearChange(gear, false);
	}
	ShiftStatus GetShiftStatus() const {
		return mShiftStatus;
	}
	ShiftPotential GetShiftPotential() const {
		return mShiftPotential;
	}

	ShiftStatus OnGearChange(GearID gear);
	bool UseRevLimiter() const {
		return bRevLimiter;
	}
	void DoECU();
	float DoThrottle(float dT);
	void DoInduction(const Physics::Tunings *tunings, float dT);
	float DoNos(const Physics::Tunings *tunings, float dT, bool engaged);
	void DoShifting(float dT);
	ShiftPotential UpdateShiftPotential(GearID gear, float rpm, float rpmFromGround);
	float GetEngineTorque(float rpm) const;

	// Inlines
	unsigned int GetNumGearRatios() const {
		return mMWInfo->GEAR_RATIO.size();
	}

	float GetGearRatio(unsigned int idx) const {
		return mMWInfo->GEAR_RATIO[idx];
	}

	float GetGearEfficiency(unsigned int idx) const {
		return mMWInfo->GEAR_EFFICIENCY[idx];
	}

	float GetFinalGear() const {
		return mMWInfo->FINAL_GEAR;
	}

	float GetRatioChange(unsigned int from, unsigned int to) const {
		float ratio1 = mMWInfo->GEAR_RATIO[from];
		float ratio2 = mMWInfo->GEAR_RATIO[to];

		if (ratio1 > 0.0f && ratio2 > FLOAT_EPSILON) {
			return ratio1 / ratio2;
		} else {
			return 0.0f;
		}
	}

	float GetShiftDelay(unsigned int gear, bool shiftUp) const {
		return mMWInfo->SHIFT_SPEED * GetGearRatio(gear);
	}

	bool RearWheelDrive() const {
		return mMWInfo->TORQUE_SPLIT < 1.0f;
	}

	bool FrontWheelDrive() const {
		return mMWInfo->TORQUE_SPLIT > 0.0f;
	}

	float GetShiftUpRPM(int gear) const {
		return mShiftUpRPM[gear];
	}

	float GetShiftDownRPM(int gear) const {
		return mShiftDownRPM[gear];
	}

	float GetCatchupCheat() const {
		if (fOpponentRubberband > 0.0 && GetVehicle()->GetDriverClass() != DRIVER_HUMAN) {
			if (mCheater) mCheater->SetCatchupCheatOverride(fOpponentRubberband);
			return fOpponentRubberband;
		}

		if (mCheater) {
			return mCheater->GetCatchupCheat();
		}
		return 0.0;
	}

	float mDriveTorque;
	float mDriveTorqueAtEngine;
	int mGear;
	float mGearShiftTimer;
	float mThrottle;
	float mSpool;
	float mPSI;
	float mInductionBoost;
	float mShiftUpRPM[10];
	float mShiftDownRPM[10];
	float mAngularVelocity;
	float mAngularAcceleration;
	float mTransmissionVelocity;
	float mNOSCapacity;
	float mNOSBoost;
	float mNOSEngaged;
	float mClutchRPMDiff;
	bool mEngineBraking;
	float mSportShifting;
	IInput *mIInput;
	IChassis *mSuspension;
	ICheater *mCheater;
	MWCarTuning* mMWInfo;
	Attrib::Gen::vehicle mCarInfo;
	float mRPM;
	ShiftStatus mShiftStatus;
	ShiftPotential mShiftPotential;
	float mPeakTorque;
	float mPeakTorqueRPM;
	float mMaxHP;
	Clutch mClutch;
	bool mBlown;
	float mSabotage;
	eTransmissionOverride mTransmissionOverride;

	IEngine tmpEngine;
	ITransmission tmpTransmission;
	IInductable tmpInductable;
	ITiptronic tmpTiptronic;
	IRaceEngine tmpRaceEngine;
	IEngineDamage tmpEngineDamage;

	IEngine* GetIEngine() { GET_FAKE_INTERFACE(EngineRacer, IEngine, tmpEngine) }
	ITransmission* GetITransmission() { GET_FAKE_INTERFACE(EngineRacer, ITransmission, tmpTransmission) }
	IInductable* GetIInductable() { GET_FAKE_INTERFACE(EngineRacer, IInductable, tmpInductable) }
	ITiptronic* GetITiptronic() { GET_FAKE_INTERFACE(EngineRacer, ITiptronic, tmpTiptronic) }
	IRaceEngine* GetIRaceEngine() { GET_FAKE_INTERFACE(EngineRacer, IRaceEngine, tmpRaceEngine) }
	IEngineDamage* GetIEngineDamage() { GET_FAKE_INTERFACE(EngineRacer, IEngineDamage, tmpEngineDamage) }
};
EngineRacer* pEngine = nullptr;