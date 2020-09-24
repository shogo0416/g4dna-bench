/*==============================================================================
  BSD 2-Clause License

  Copyright (c) 2020 Shogo OKADA (shogo.okada@kek.jp)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================*/
#ifndef SIMDATA_H_
#define SIMDATA_H_
#include "globals.hh"
#include <vector>
#include <string>
#include <map>

#include "json.hpp"
using json = nlohmann::json;

//==============================================================================

struct TimeStepInfo {
  double sim_time;
  std::map<std::string, int> species;
};

//==============================================================================

class SimData {
public:
  static SimData* GetInstance();
  ~SimData() = default;

  SimData(const SimData&) = delete;
  void operator=(SimData&) = delete;

  void Setup();
  void SetThreadNumber(int in);

  std::vector<double>& GetScoreTime();

  void SetFileName(const std::string& fname);
  void SetBenchmarkFileName(const std::string& fname);

  void AccumulateEdep(int id, double edep);
  void ResetEdep(int id);
  double GetEdep(int id) const;

  void GValue(int id, int tid, const std::string& name, double gval);

  void CountAbortEvent(int id);
  void CountChemEvent(int id);

  void SaveSimulationResult(int id = -1);
  void SaveBenchmarkResult();

  std::vector<double>& GetElapTime();
  std::vector<double>& GetElapTimeChem();

  void Performance(int id);

  void OutputResultForEachThread(bool in);
  void OutputPerformanceForEachThread(bool in);

  std::vector<int>& GetNumPhysStep();
  std::vector<int>& GetNumChemStep();

  void PushTimeStepInfo(int id, const TimeStepInfo& info);
  void ClearTimeStepInfo(int id);
  std::vector<TimeStepInfo>& GetTimeStepInfo(int id);

  std::map<std::string, int>& GetScoredMolecule();

private:
  SimData();
  static SimData* instance_;

  void Merge();

  std::string fname_;
  std::string fname_bench_;

  int num_thread_;
  std::vector<double> score_time_;

  std::vector<double> edep_buff_;
  std::vector<std::vector<double> > gval_buff_;

  std::vector<std::vector<TimeStepInfo> > tsi_buff_;

  std::vector<int> num_abort_event_;
  std::vector<int> num_chem_event_;

  std::vector<double> elap_time_;
  std::vector<double> elap_time_chem_;

  bool result_each_thread_;
  bool performance_each_thread_;

  std::map<std::string, int> mole_map_;
  std::vector<std::string> header_;

  std::vector<int> num_phys_step_;
  std::vector<int> num_chem_step_;

  json js_;
};

//==============================================================================
inline void SimData::SetFileName(const std::string& fname)
{
  fname_ = fname;
}

//------------------------------------------------------------------------------
inline void SimData::SetBenchmarkFileName(const std::string& fname)
{
  fname_bench_ = fname;
}

//------------------------------------------------------------------------------
inline std::vector<double>& SimData::GetScoreTime()
{
  return score_time_;
}

//------------------------------------------------------------------------------
inline void SimData::SetThreadNumber(int in)
{
  num_thread_ = in;
}

//------------------------------------------------------------------------------
inline void SimData::AccumulateEdep(int id, double edep)
{
  edep_buff_[id] += edep;
}

//------------------------------------------------------------------------------
inline void SimData::ResetEdep(int id)
{
  edep_buff_[id] = 0;
}

//------------------------------------------------------------------------------
inline double SimData::GetEdep(int id) const
{
  return edep_buff_[id];
}

//------------------------------------------------------------------------------
inline void SimData::CountAbortEvent(int id)
{
  num_abort_event_[id] += 1;
}

//------------------------------------------------------------------------------
inline void SimData::CountChemEvent(int id)
{
  num_chem_event_[id] += 1;
}

//------------------------------------------------------------------------------
inline std::vector<double>& SimData::GetElapTime()
{
  return elap_time_;
}

//------------------------------------------------------------------------------
inline std::vector<double>& SimData::GetElapTimeChem()
{
  return elap_time_chem_;
}

//------------------------------------------------------------------------------
inline void SimData::OutputResultForEachThread(bool in)
{
  result_each_thread_ = in;
}

//------------------------------------------------------------------------------
inline void SimData::OutputPerformanceForEachThread(bool in)
{
  performance_each_thread_ = in;
}

//------------------------------------------------------------------------------
inline std::vector<int>& SimData::GetNumPhysStep()
{
  return num_phys_step_;
}

//------------------------------------------------------------------------------
inline std::vector<int>& SimData::GetNumChemStep()
{
  return num_chem_step_;
}

//------------------------------------------------------------------------------
inline void SimData::PushTimeStepInfo(int id, const TimeStepInfo& info)
{
  if (tsi_buff_[id].size() == 0) {
    tsi_buff_[id].push_back(info);
    return;
  }
  auto itr = tsi_buff_[id].end(); itr--;
  if ((*itr).sim_time == info.sim_time) {
    tsi_buff_[id].erase(itr);
  }
  tsi_buff_[id].push_back(info);
}

//------------------------------------------------------------------------------
inline std::vector<TimeStepInfo>& SimData::GetTimeStepInfo(int id)
{
  return tsi_buff_[id];
}

//------------------------------------------------------------------------------
inline void SimData::ClearTimeStepInfo(int id)
{
  tsi_buff_[id].clear();
}

//------------------------------------------------------------------------------
inline std::map<std::string, int>& SimData::GetScoredMolecule()
{
  return mole_map_;
}

#endif // SIMDATA_H_
