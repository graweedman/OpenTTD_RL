/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file company_sl.cpp Code handling saving and loading of company data */

#include "../stdafx.h"

#include "saveload.h"
#include "compat/company_sl_compat.h"

#include "../company_func.h"
#include "../company_manager_face.h"
#include "../fios.h"
#include "../tunnelbridge_map.h"
#include "../tunnelbridge.h"
#include "../station_base.h"
#include "../strings_func.h"

#include "table/strings.h"

#include "../safeguards.h"

/**
 * Search for a face variable by type and name.
 * @param style Face style to find variable in.
 * @param type Type of variable to look up.
 * @param name Name (string) of variable to look up.
 * @return Face variable if present, otherwise nullptr.
 */
static const FaceVar *FindFaceVar(FaceVars style, FaceVarType type, StringID name)
{
	auto it = std::ranges::find_if(style, [type, name](const FaceVar &facevar) { return facevar.type == type && facevar.name == name; });
	if (it != std::end(style)) return &*it;
	return nullptr;
}

/**
 * Converts an old company manager's face format to the new company manager's face format
 *
 * Meaning of the bits in the old face (some bits are used in several times):
 * - 4 and 5: chin
 * - 6 to 9: eyebrows
 * - 10 to 13: nose
 * - 13 to 15: lips (also moustache for males)
 * - 16 to 19: hair
 * - 20 to 22: eye colour
 * - 20 to 27: tie, ear rings etc.
 * - 28 to 30: glasses
 * - 19, 26 and 27: race (bit 27 set and bit 19 equal to bit 26 = black, otherwise white)
 * - 31: gender (0 = male, 1 = female)
 *
 * @param face the face in the old format
 * @return the face in the new format
 */
CompanyManagerFace ConvertFromOldCompanyManagerFace(uint32_t face)
{
	CompanyManagerFace cmf{};

	if (HasBit(face, 31)) cmf.style += 1;
	if (HasBit(face, 27) && (HasBit(face, 26) == HasBit(face, 19))) cmf.style += 2;

	const FaceSpec *spec = GetCompanyManagerFaceSpec(cmf.style);
	FaceVars vars = spec->GetFaceVars();

	cmf.style_label = spec->label;

	if (auto var = FindFaceVar(vars, FaceVarType::Toggle, STR_FACE_GLASSES); var != nullptr) var->SetBits(cmf, GB(face, 28, 3) <= 1);
	if (auto var = FindFaceVar(vars, FaceVarType::Palette, STR_FACE_EYECOLOUR); var != nullptr) var->SetBits(cmf, ClampU(GB(face, 20, 3), 5, 7) - 5);
	if (auto var = FindFaceVar(vars, FaceVarType::Sprite, STR_FACE_CHIN); var != nullptr) var->SetBits(cmf, var->ScaleBits(GB(face, 4, 2)));
	if (auto var = FindFaceVar(vars, FaceVarType::Sprite, STR_FACE_EYEBROWS); var != nullptr) var->SetBits(cmf, var->ScaleBits(GB(face, 6, 4)));
	if (auto var = FindFaceVar(vars, FaceVarType::Sprite, STR_FACE_HAIR); var != nullptr) var->SetBits(cmf, var->ScaleBits(GB(face, 16, 4)));
	if (auto var = FindFaceVar(vars, FaceVarType::Sprite, STR_FACE_JACKET); var != nullptr) var->SetBits(cmf, var->ScaleBits(GB(face, 20, 2)));
	if (auto var = FindFaceVar(vars, FaceVarType::Sprite, STR_FACE_COLLAR); var != nullptr) var->SetBits(cmf, var->ScaleBits(GB(face, 22, 2)));
	if (auto var = FindFaceVar(vars, FaceVarType::Sprite, STR_FACE_GLASSES); var != nullptr) var->SetBits(cmf, GB(face, 28, 1));

	uint lips = GB(face, 10, 4);
	if (cmf.style != 1 && cmf.style != 3 && lips < 4) {
		if (auto var = FindFaceVar(vars, FaceVarType::Toggle, STR_FACE_MOUSTACHE); var != nullptr) var->SetBits(cmf, true);
		if (auto var = FindFaceVar(vars, FaceVarType::Sprite, STR_FACE_MOUSTACHE); var != nullptr) var->SetBits(cmf, std::max(lips, 1U) - 1);
	} else {
		if (auto var = FindFaceVar(vars, FaceVarType::Sprite, STR_FACE_LIPS); var != nullptr) {
			if (cmf.style == 0 || cmf.style == 2) {
				lips = lips * 15 / 16;
				lips -= 3;
				if (cmf.style == 2 && lips > 8) lips = 0;
			} else {
				lips = var->ScaleBits(lips);
			}
			var->SetBits(cmf, lips);
		}

		if (auto var = FindFaceVar(vars, FaceVarType::Sprite, STR_FACE_NOSE); var != nullptr) {
			uint nose = GB(face, 13, 3);
			if (cmf.style == 1) {
				nose = (nose * 3 >> 3) * 3 >> 2; // There is 'hole' in the nose sprites for women
			} else {
				nose = var->ScaleBits(nose);
			}
			var->SetBits(cmf, nose);
		}
	}

	if (auto var = FindFaceVar(vars, FaceVarType::Sprite, STR_FACE_TIE); var != nullptr) {
		uint tie = GB(face, 24, 4);
		var->SetBits(cmf, var->ScaleBits(tie / 2));
	}

	if (auto var = FindFaceVar(vars, FaceVarType::Sprite, STR_FACE_EARRING); var != nullptr) {
		uint earring = GB(face, 24, 4);
		if (earring < 3) { // Not all women have an earring
			if (auto has_earring = FindFaceVar(vars, FaceVarType::Toggle, STR_FACE_EARRING)) {
				has_earring->SetBits(cmf, true);
				var->SetBits(cmf, earring);
			}
		}
	}

	return cmf;
}

/** Rebuilding of company statistics after loading a savegame. */
void AfterLoadCompanyStats()
{
	/* Reset infrastructure statistics to zero. */
	for (Company *c : Company::Iterate()) c->infrastructure = {};

	/* Collect airport count. */
	for (const Station *st : Station::Iterate()) {
		if (st->facilities.Test(StationFacility::Airport) && Company::IsValidID(st->owner)) {
			Company::Get(st->owner)->infrastructure.airport++;
		}
	}

	Company *c;
	for (const auto tile : Map::Iterate()) {
		switch (GetTileType(tile)) {
			case MP_RAILWAY:
				c = Company::GetIfValid(GetTileOwner(tile));
				if (c != nullptr) {
					uint pieces = 1;
					if (IsPlainRail(tile)) {
						TrackBits bits = GetTrackBits(tile);
						pieces = CountBits(bits);
						if (TracksOverlap(bits)) pieces *= pieces;
					}
					c->infrastructure.rail[GetRailType(tile)] += pieces;

					if (HasSignals(tile)) c->infrastructure.signal += CountBits(GetPresentSignals(tile));
				}
				break;

			case MP_ROAD: {
				if (IsLevelCrossing(tile)) {
					c = Company::GetIfValid(GetTileOwner(tile));
					if (c != nullptr) c->infrastructure.rail[GetRailType(tile)] += LEVELCROSSING_TRACKBIT_FACTOR;
				}

				/* Iterate all present road types as each can have a different owner. */
				for (RoadTramType rtt : _roadtramtypes) {
					RoadType rt = GetRoadType(tile, rtt);
					if (rt == INVALID_ROADTYPE) continue;
					c = Company::GetIfValid(IsRoadDepot(tile) ? GetTileOwner(tile) : GetRoadOwner(tile, rtt));
					/* A level crossings and depots have two road bits. */
					if (c != nullptr) c->infrastructure.road[rt] += IsNormalRoad(tile) ? CountBits(GetRoadBits(tile, rtt)) : 2;
				}
				break;
			}

			case MP_STATION:
				c = Company::GetIfValid(GetTileOwner(tile));
				if (c != nullptr && GetStationType(tile) != StationType::Airport && !IsBuoy(tile)) c->infrastructure.station++;

				switch (GetStationType(tile)) {
					case StationType::Rail:
					case StationType::RailWaypoint:
						if (c != nullptr && !IsStationTileBlocked(tile)) c->infrastructure.rail[GetRailType(tile)]++;
						break;

					case StationType::Bus:
					case StationType::Truck:
					case StationType::RoadWaypoint: {
						/* Iterate all present road types as each can have a different owner. */
						for (RoadTramType rtt : _roadtramtypes) {
							RoadType rt = GetRoadType(tile, rtt);
							if (rt == INVALID_ROADTYPE) continue;
							c = Company::GetIfValid(GetRoadOwner(tile, rtt));
							if (c != nullptr) c->infrastructure.road[rt] += 2; // A road stop has two road bits.
						}
						break;
					}

					case StationType::Dock:
					case StationType::Buoy:
						if (GetWaterClass(tile) == WATER_CLASS_CANAL) {
							if (c != nullptr) c->infrastructure.water++;
						}
						break;

					default:
						break;
				}
				break;

			case MP_WATER:
				if (IsShipDepot(tile) || IsLock(tile)) {
					c = Company::GetIfValid(GetTileOwner(tile));
					if (c != nullptr) {
						if (IsShipDepot(tile)) c->infrastructure.water += LOCK_DEPOT_TILE_FACTOR;
						if (IsLock(tile) && GetLockPart(tile) == LOCK_PART_MIDDLE) {
							/* The middle tile specifies the owner of the lock. */
							c->infrastructure.water += 3 * LOCK_DEPOT_TILE_FACTOR; // the middle tile specifies the owner of the
							break; // do not count the middle tile as canal
						}
					}
				}
				[[fallthrough]];

			case MP_OBJECT:
				if (GetWaterClass(tile) == WATER_CLASS_CANAL) {
					c = Company::GetIfValid(GetTileOwner(tile));
					if (c != nullptr) c->infrastructure.water++;
				}
				break;

			case MP_TUNNELBRIDGE: {
				/* Only count the tunnel/bridge if we're on the northern end tile. */
				TileIndex other_end = GetOtherTunnelBridgeEnd(tile);
				if (tile < other_end) {
					/* Count each tunnel/bridge TUNNELBRIDGE_TRACKBIT_FACTOR times to simulate
					 * the higher structural maintenance needs, and don't forget the end tiles. */
					uint len = (GetTunnelBridgeLength(tile, other_end) + 2) * TUNNELBRIDGE_TRACKBIT_FACTOR;

					switch (GetTunnelBridgeTransportType(tile)) {
						case TRANSPORT_RAIL:
							c = Company::GetIfValid(GetTileOwner(tile));
							if (c != nullptr) c->infrastructure.rail[GetRailType(tile)] += len;
							break;

						case TRANSPORT_ROAD: {
							/* Iterate all present road types as each can have a different owner. */
							for (RoadTramType rtt : _roadtramtypes) {
								RoadType rt = GetRoadType(tile, rtt);
								if (rt == INVALID_ROADTYPE) continue;
								c = Company::GetIfValid(GetRoadOwner(tile, rtt));
								if (c != nullptr) c->infrastructure.road[rt] += len * 2; // A full diagonal road has two road bits.
							}
							break;
						}

						case TRANSPORT_WATER:
							c = Company::GetIfValid(GetTileOwner(tile));
							if (c != nullptr) c->infrastructure.water += len;
							break;

						default:
							break;
					}
				}
				break;
			}

			default:
				break;
		}
	}
}

/* We do need to read this single value, as the bigger it gets, the more data is stored */
struct CompanyOldAI {
	uint8_t num_build_rec;
};

class SlCompanyOldAIBuildRec : public DefaultSaveLoadHandler<SlCompanyOldAIBuildRec, CompanyOldAI> {
public:
	static inline const SaveLoad description[] = {{}}; // Needed to keep DefaultSaveLoadHandler happy.
	static inline const SaveLoadCompatTable compat_description = _company_old_ai_buildrec_compat;

	SaveLoadTable GetDescription() const override { return {}; }

	void Load(CompanyOldAI *old_ai) const override
	{
		for (int i = 0; i != old_ai->num_build_rec; i++) {
			SlObject(nullptr, this->GetLoadDescription());
		}
	}

	void LoadCheck(CompanyOldAI *old_ai) const override { this->Load(old_ai); }
};

class SlCompanyOldAI : public DefaultSaveLoadHandler<SlCompanyOldAI, CompanyProperties> {
public:
	static inline const SaveLoad description[] = {
		SLE_CONDVAR(CompanyOldAI, num_build_rec, SLE_UINT8, SL_MIN_VERSION, SLV_107),
		SLEG_STRUCTLIST("buildrec", SlCompanyOldAIBuildRec),
	};
	static inline const SaveLoadCompatTable compat_description = _company_old_ai_compat;

	void Load(CompanyProperties *c) const override
	{
		if (!c->is_ai) return;

		CompanyOldAI old_ai;
		SlObject(&old_ai, this->GetLoadDescription());
	}

	void LoadCheck(CompanyProperties *c) const override { this->Load(c); }
};

class SlCompanySettings : public DefaultSaveLoadHandler<SlCompanySettings, CompanyProperties> {
public:
	static inline const SaveLoad description[] = {
		/* Engine renewal settings */
		SLE_CONDREF(CompanyProperties, engine_renew_list,            REF_ENGINE_RENEWS,   SLV_19, SL_MAX_VERSION),
		SLE_CONDVAR(CompanyProperties, settings.engine_renew,        SLE_BOOL,            SLV_16, SL_MAX_VERSION),
		SLE_CONDVAR(CompanyProperties, settings.engine_renew_months, SLE_INT16,           SLV_16, SL_MAX_VERSION),
		SLE_CONDVAR(CompanyProperties, settings.engine_renew_money,  SLE_UINT32,          SLV_16, SL_MAX_VERSION),
		SLE_CONDVAR(CompanyProperties, settings.renew_keep_length,   SLE_BOOL,             SLV_2, SL_MAX_VERSION),

		/* Default vehicle settings */
		SLE_CONDVAR(CompanyProperties, settings.vehicle.servint_ispercent,   SLE_BOOL,     SLV_120, SL_MAX_VERSION),
		SLE_CONDVAR(CompanyProperties, settings.vehicle.servint_trains,    SLE_UINT16,     SLV_120, SL_MAX_VERSION),
		SLE_CONDVAR(CompanyProperties, settings.vehicle.servint_roadveh,   SLE_UINT16,     SLV_120, SL_MAX_VERSION),
		SLE_CONDVAR(CompanyProperties, settings.vehicle.servint_aircraft,  SLE_UINT16,     SLV_120, SL_MAX_VERSION),
		SLE_CONDVAR(CompanyProperties, settings.vehicle.servint_ships,     SLE_UINT16,     SLV_120, SL_MAX_VERSION),
	};
	static inline const SaveLoadCompatTable compat_description = _company_settings_compat;

	void Save(CompanyProperties *c) const override
	{
		SlObject(c, this->GetDescription());
	}

	void Load(CompanyProperties *c) const override
	{
		SlObject(c, this->GetLoadDescription());
	}

	void FixPointers(CompanyProperties *c) const override
	{
		SlObject(c, this->GetDescription());
	}

	void LoadCheck(CompanyProperties *c) const override { this->Load(c); }
};

class SlCompanyEconomy : public DefaultSaveLoadHandler<SlCompanyEconomy, CompanyProperties> {
public:
	static inline const SaveLoad description[] = {
		SLE_CONDVAR(CompanyEconomyEntry, income,              SLE_FILE_I32 | SLE_VAR_I64, SL_MIN_VERSION, SLV_2),
		SLE_CONDVAR(CompanyEconomyEntry, income,              SLE_INT64,                  SLV_2, SL_MAX_VERSION),
		SLE_CONDVAR(CompanyEconomyEntry, expenses,            SLE_FILE_I32 | SLE_VAR_I64, SL_MIN_VERSION, SLV_2),
		SLE_CONDVAR(CompanyEconomyEntry, expenses,            SLE_INT64,                  SLV_2, SL_MAX_VERSION),
		SLE_CONDVAR(CompanyEconomyEntry, company_value,       SLE_FILE_I32 | SLE_VAR_I64, SL_MIN_VERSION, SLV_2),
		SLE_CONDVAR(CompanyEconomyEntry, company_value,       SLE_INT64,                  SLV_2, SL_MAX_VERSION),

		SLE_CONDVAR(CompanyEconomyEntry, delivered_cargo[NUM_CARGO - 1], SLE_INT32,       SL_MIN_VERSION, SLV_170),
		SLE_CONDARR(CompanyEconomyEntry, delivered_cargo,     SLE_UINT32, 32,           SLV_170, SLV_EXTEND_CARGOTYPES),
		SLE_CONDARR(CompanyEconomyEntry, delivered_cargo,     SLE_UINT32, NUM_CARGO,    SLV_EXTEND_CARGOTYPES, SL_MAX_VERSION),
		    SLE_VAR(CompanyEconomyEntry, performance_history, SLE_INT32),
	};
	static inline const SaveLoadCompatTable compat_description = _company_economy_compat;

	void Save(CompanyProperties *c) const override
	{
		SlObject(&c->cur_economy, this->GetDescription());
	}

	void Load(CompanyProperties *c) const override
	{
		SlObject(&c->cur_economy, this->GetLoadDescription());
	}

	void FixPointers(CompanyProperties *c) const override
	{
		SlObject(&c->cur_economy, this->GetDescription());
	}

	void LoadCheck(CompanyProperties *c) const override { this->Load(c); }
};

class SlCompanyOldEconomy : public SlCompanyEconomy {
public:
	void Save(CompanyProperties *c) const override
	{
		SlSetStructListLength(c->num_valid_stat_ent);
		for (int i = 0; i < c->num_valid_stat_ent; i++) {
			SlObject(&c->old_economy[i], this->GetDescription());
		}
	}

	void Load(CompanyProperties *c) const override
	{
		if (!IsSavegameVersionBefore(SLV_SAVELOAD_LIST_LENGTH)) {
			c->num_valid_stat_ent = (uint8_t)SlGetStructListLength(UINT8_MAX);
		}
		if (c->num_valid_stat_ent > std::size(c->old_economy)) SlErrorCorrupt("Too many old economy entries");

		for (int i = 0; i < c->num_valid_stat_ent; i++) {
			SlObject(&c->old_economy[i], this->GetLoadDescription());
		}
	}

	void LoadCheck(CompanyProperties *c) const override { this->Load(c); }
};

class SlCompanyLiveries : public DefaultSaveLoadHandler<SlCompanyLiveries, CompanyProperties> {
public:
	static inline const SaveLoad description[] = {
		SLE_CONDVAR(Livery, in_use,  SLE_UINT8, SLV_34, SL_MAX_VERSION),
		SLE_CONDVAR(Livery, colour1, SLE_UINT8, SLV_34, SL_MAX_VERSION),
		SLE_CONDVAR(Livery, colour2, SLE_UINT8, SLV_34, SL_MAX_VERSION),
	};
	static inline const SaveLoadCompatTable compat_description = _company_liveries_compat;

	/**
	 * Get the number of liveries used by this savegame version.
	 * @return The number of liveries used by this savegame version.
	 */
	size_t GetNumLiveries() const
	{
		if (IsSavegameVersionBefore(SLV_63)) return LS_END - 4;
		if (IsSavegameVersionBefore(SLV_85)) return LS_END - 2;
		if (IsSavegameVersionBefore(SLV_SAVELOAD_LIST_LENGTH)) return LS_END;
		/* Read from the savegame how long the list is. */
		return SlGetStructListLength(LS_END);
	}

	void Save(CompanyProperties *c) const override
	{
		SlSetStructListLength(LS_END);
		for (int i = 0; i < LS_END; i++) {
			SlObject(&c->livery[i], this->GetDescription());
		}
	}

	void Load(CompanyProperties *c) const override
	{
		size_t num_liveries = this->GetNumLiveries();
		bool update_in_use = IsSavegameVersionBefore(SLV_GROUP_LIVERIES);

		for (size_t i = 0; i < num_liveries; i++) {
			SlObject(&c->livery[i], this->GetLoadDescription());
			if (update_in_use && i != LS_DEFAULT) {
				if (c->livery[i].in_use == 0) {
					c->livery[i].colour1 = c->livery[LS_DEFAULT].colour1;
					c->livery[i].colour2 = c->livery[LS_DEFAULT].colour2;
				} else {
					c->livery[i].in_use = 3;
				}
			}
		}

		if (IsSavegameVersionBefore(SLV_85)) {
			/* We want to insert some liveries somewhere in between. This means some have to be moved. */
			std::move_backward(&c->livery[LS_FREIGHT_WAGON - 2], &c->livery[LS_END - 2], &c->livery[LS_END]);
			c->livery[LS_PASSENGER_WAGON_MONORAIL] = c->livery[LS_MONORAIL];
			c->livery[LS_PASSENGER_WAGON_MAGLEV]   = c->livery[LS_MAGLEV];
		}

		if (IsSavegameVersionBefore(SLV_63)) {
			/* Copy bus/truck liveries over to trams */
			c->livery[LS_PASSENGER_TRAM] = c->livery[LS_BUS];
			c->livery[LS_FREIGHT_TRAM]   = c->livery[LS_TRUCK];
		}
	}

	void LoadCheck(CompanyProperties *c) const override { this->Load(c); }
};

class SlAllowListData : public VectorSaveLoadHandler<SlAllowListData, CompanyProperties, std::string> {
public:
	struct KeyWrapper {
		std::string key;
	};

	static inline const SaveLoad description[] = {
		SLE_SSTR(KeyWrapper, key, SLE_STR),
	};
	static inline const SaveLoadCompatTable compat_description = {};

	std::vector<std::string> &GetVector(CompanyProperties *cprops) const override { return cprops->allow_list; }

	void LoadCheck(CompanyProperties *cprops) const override { this->Load(cprops); }
};

/* Save/load of companies */
static const SaveLoad _company_desc[] = {
	    SLE_VAR(CompanyProperties, name_2,          SLE_UINT32),
	    SLE_VAR(CompanyProperties, name_1,          SLE_STRINGID),
	SLE_CONDSSTR(CompanyProperties, name,            SLE_STR | SLF_ALLOW_CONTROL, SLV_84, SL_MAX_VERSION),

	    SLE_VAR(CompanyProperties, president_name_1, SLE_STRINGID),
	    SLE_VAR(CompanyProperties, president_name_2, SLE_UINT32),
	SLE_CONDSSTR(CompanyProperties, president_name,  SLE_STR | SLF_ALLOW_CONTROL, SLV_84, SL_MAX_VERSION),

	SLE_CONDVECTOR(CompanyProperties, allow_list, SLE_STR, SLV_COMPANY_ALLOW_LIST, SLV_COMPANY_ALLOW_LIST_V2),
	SLEG_CONDSTRUCTLIST("allow_list", SlAllowListData, SLV_COMPANY_ALLOW_LIST_V2, SL_MAX_VERSION),

	SLE_VARNAME(CompanyProperties, face.bits, "face", SLE_UINT32),
	SLE_CONDSSTRNAME(CompanyProperties, face.style_label, "face_style", SLE_STR, SLV_FACE_STYLES, SL_MAX_VERSION),

	/* money was changed to a 64 bit field in savegame version 1. */
	SLE_CONDVAR(CompanyProperties, money,                 SLE_VAR_I64 | SLE_FILE_I32,  SL_MIN_VERSION, SLV_1),
	SLE_CONDVAR(CompanyProperties, money,                 SLE_INT64,                   SLV_1, SL_MAX_VERSION),

	SLE_CONDVAR(CompanyProperties, current_loan,          SLE_VAR_I64 | SLE_FILE_I32,  SL_MIN_VERSION, SLV_65),
	SLE_CONDVAR(CompanyProperties, current_loan,          SLE_INT64,                  SLV_65, SL_MAX_VERSION),
	SLE_CONDVAR(CompanyProperties, max_loan,              SLE_INT64, SLV_MAX_LOAN_FOR_COMPANY, SL_MAX_VERSION),

	    SLE_VAR(CompanyProperties, colour,                SLE_UINT8),
	    SLE_VAR(CompanyProperties, money_fraction,        SLE_UINT8),
	    SLE_VAR(CompanyProperties, block_preview,         SLE_UINT8),

	SLE_CONDVAR(CompanyProperties, location_of_HQ,        SLE_FILE_U16 | SLE_VAR_U32,  SL_MIN_VERSION,  SLV_6),
	SLE_CONDVAR(CompanyProperties, location_of_HQ,        SLE_UINT32,                  SLV_6, SL_MAX_VERSION),
	SLE_CONDVAR(CompanyProperties, last_build_coordinate, SLE_FILE_U16 | SLE_VAR_U32,  SL_MIN_VERSION,  SLV_6),
	SLE_CONDVAR(CompanyProperties, last_build_coordinate, SLE_UINT32,                  SLV_6, SL_MAX_VERSION),
	SLE_CONDVAR(CompanyProperties, inaugurated_year,      SLE_FILE_U8  | SLE_VAR_I32,  SL_MIN_VERSION, SLV_31),
	SLE_CONDVAR(CompanyProperties, inaugurated_year,      SLE_INT32,                  SLV_31, SL_MAX_VERSION),
	SLE_CONDVAR(CompanyProperties, inaugurated_year_calendar, SLE_INT32,               SLV_COMPANY_INAUGURATED_PERIOD_V2, SL_MAX_VERSION),

	SLE_CONDVAR(CompanyProperties, num_valid_stat_ent,    SLE_UINT8,                   SL_MIN_VERSION, SLV_SAVELOAD_LIST_LENGTH),

	    SLE_VAR(CompanyProperties, months_of_bankruptcy,  SLE_UINT8),
	SLE_CONDVAR(CompanyProperties, bankrupt_asked,        SLE_FILE_U8  | SLE_VAR_U16,  SL_MIN_VERSION, SLV_104),
	SLE_CONDVAR(CompanyProperties, bankrupt_asked,        SLE_UINT16,                SLV_104, SL_MAX_VERSION),
	    SLE_VAR(CompanyProperties, bankrupt_timeout,      SLE_INT16),
	SLE_CONDVAR(CompanyProperties, bankrupt_value,        SLE_VAR_I64 | SLE_FILE_I32,  SL_MIN_VERSION, SLV_65),
	SLE_CONDVAR(CompanyProperties, bankrupt_value,        SLE_INT64,                  SLV_65, SL_MAX_VERSION),

	/* yearly expenses was changed to 64-bit in savegame version 2. */
	SLE_CONDARR(CompanyProperties, yearly_expenses,       SLE_FILE_I32 | SLE_VAR_I64, 3 * 13, SL_MIN_VERSION, SLV_2),
	SLE_CONDARR(CompanyProperties, yearly_expenses,       SLE_INT64, 3 * 13,                  SLV_2, SL_MAX_VERSION),

	SLE_CONDVAR(CompanyProperties, is_ai,                 SLE_BOOL,                    SLV_2, SL_MAX_VERSION),

	SLE_CONDVAR(CompanyProperties, terraform_limit,       SLE_UINT32,                SLV_156, SL_MAX_VERSION),
	SLE_CONDVAR(CompanyProperties, clear_limit,           SLE_UINT32,                SLV_156, SL_MAX_VERSION),
	SLE_CONDVAR(CompanyProperties, tree_limit,            SLE_UINT32,                SLV_175, SL_MAX_VERSION),
	SLEG_STRUCT("settings", SlCompanySettings),
	SLEG_CONDSTRUCT("old_ai", SlCompanyOldAI,                                        SL_MIN_VERSION, SLV_107),
	SLEG_STRUCT("cur_economy", SlCompanyEconomy),
	SLEG_STRUCTLIST("old_economy", SlCompanyOldEconomy),
	SLEG_CONDSTRUCTLIST("liveries", SlCompanyLiveries,                               SLV_34, SL_MAX_VERSION),
};

struct PLYRChunkHandler : ChunkHandler {
	PLYRChunkHandler() : ChunkHandler('PLYR', CH_TABLE) {}

	void Save() const override
	{
		SlTableHeader(_company_desc);

		for (Company *c : Company::Iterate()) {
			SlSetArrayIndex(c->index);
			SlObject(c, _company_desc);
		}
	}

	void Load() const override
	{
		const std::vector<SaveLoad> slt = SlCompatTableHeader(_company_desc, _company_sl_compat);

		int index;
		while ((index = SlIterateArray()) != -1) {
			Company *c = new (CompanyID(index)) Company();
			SlObject(c, slt);
			_company_colours[index] = c->colour;
		}
	}


	void LoadCheck(size_t) const override
	{
		const std::vector<SaveLoad> slt = SlCompatTableHeader(_company_desc, _company_sl_compat);

		int index;
		while ((index = SlIterateArray()) != -1) {
			std::unique_ptr<CompanyProperties> cprops = std::make_unique<CompanyProperties>();
			SlObject(cprops.get(), slt);

			/* We do not load old custom names */
			if (IsSavegameVersionBefore(SLV_84)) {
				if (GetStringTab(cprops->name_1) == TEXT_TAB_OLD_CUSTOM) {
					cprops->name_1 = STR_GAME_SAVELOAD_NOT_AVAILABLE;
				}

				if (GetStringTab(cprops->president_name_1) == TEXT_TAB_OLD_CUSTOM) {
					cprops->president_name_1 = STR_GAME_SAVELOAD_NOT_AVAILABLE;
				}
			}

			if (cprops->name.empty() && !IsInsideMM(cprops->name_1, SPECSTR_COMPANY_NAME_START, SPECSTR_COMPANY_NAME_END) &&
				cprops->name_1 != STR_GAME_SAVELOAD_NOT_AVAILABLE && cprops->name_1 != STR_SV_UNNAMED &&
				cprops->name_1 != SPECSTR_ANDCO_NAME && cprops->name_1 != SPECSTR_PRESIDENT_NAME &&
				cprops->name_1 != SPECSTR_SILLY_NAME) {
				cprops->name_1 = STR_GAME_SAVELOAD_NOT_AVAILABLE;
			}

			if (_load_check_data.companies.count(index) == 0) {
				_load_check_data.companies[index] = std::move(cprops);
			}
		}
	}

	void FixPointers() const override
	{
		for (Company *c : Company::Iterate()) {
			SlObject(c, _company_desc);
		}
	}
};

static const PLYRChunkHandler PLYR;
static const ChunkHandlerRef company_chunk_handlers[] = {
	PLYR,
};

extern const ChunkHandlerTable _company_chunk_handlers(company_chunk_handlers);
