/*==============================================================================
  BSD 2-Clause License

  Copyright (c) 2020-2025 Shogo OKADA (shogo.okada@kek.jp)
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
#include "json.hpp"
#include <vector>
#include <string>
#include <map>

//==============================================================================

struct TimeStepInfo {
  double sim_time;
  std::map<std::string, int> species;
};

//==============================================================================

struct ChemInfo {
  double proc_time;
  std::map<std::string, int> species;
};

//==============================================================================
// first element:  energy deposit in eV
// second element: LET in keV/um
using LETInfo = std::pair<double, double>;

//==============================================================================

class SimData {
public:
  static SimData* GetInstance();
  ~SimData() = default;

  SimData(const SimData&) = delete;
  void operator=(SimData&) = delete;

  void Setup();
  void SetThreadNumber(int in);
  void SetEndTime(double in);

  std::vector<double>& GetScoreTime();

  void SetGValueFileName(const std::string& fname);
  void SetLETFileName(const std::string& fname);
  void SetBenchmarkFileName(const std::string& fname);

  void GValue(int id, int tid, const std::string& name, double gval);
  double GetGValue(int id, int tid, const std::string& name);

  void CountAbortEvent(int id);
  void CountChemEvent(int id);

  void SaveSimulationResult(int id = -1);
  void SaveBenchmarkResult();

  std::vector<double>& GetTotElapTime();
  std::vector<double>& GetTotElapTimeChem();
  std::vector<double>& GetElapTimeChem();

  void Performance(int id);

  void OutputResultForEachThread(bool in);
  void RecordBenchmarkScoreForThreads(bool in);
  bool ThreadBenchmarkTestIsEnabled();
  void OutputEventInfo(bool in);

  std::vector<int>& GetNumPhysStep();
  std::vector<int>& GetNumChemStep();

  void PushTimeStepInfo(int id, const TimeStepInfo& info, bool prestep = false);
  void ClearTimeStepInfo(int id, bool prestep = false);
  std::vector<TimeStepInfo>& GetTimeStepInfo(int id, bool prestep = false);

  std::map<std::string, int>& GetScoredMolecule();

  void PushChemInfo(int id, const ChemInfo& info);
  std::vector<ChemInfo>& GetChemInfo(int id);

  void PushLETInfo(int id, const LETInfo& info);
  std::vector<LETInfo>& GetLETInfo(int id);

  void CountProcessedEvent(int id);
  void CheckProcessedEventNumber();

private:
  SimData();
  static SimData* instance_;

  void Merge();

  std::string fname_gval_;
  std::string fname_LET_;
  std::string fname_bench_;

  int num_thread_;
  double end_time_;
  std::vector<double> score_time_;

  std::vector<std::vector<double> > gval_buff_;

  std::vector<std::vector<TimeStepInfo> > tsi_buff_pre_;
  std::vector<std::vector<TimeStepInfo> > tsi_buff_;

  std::vector<std::vector<ChemInfo> > ci_buff_;

  std::vector<std::vector<LETInfo> > LET_buff_;

  std::vector<int> num_abort_event_;
  std::vector<int> num_chem_event_;

  std::vector<double> tot_elap_time_;
  std::vector<double> tot_elap_time_chem_;
  std::vector<double> elap_time_chem_;

  bool result_each_thread_;
  bool benchmark_threads_;
  bool dump_event_info_;

  std::map<std::string, int> mole_map_;
  std::vector<std::string> header_;

  std::vector<int> num_phys_step_;
  std::vector<int> num_chem_step_;

  nlohmann::ordered_json js_;

  std::vector<int> num_processed_event_;

  int tot_num_event_processed_;
};

//==============================================================================
inline void SimData::SetGValueFileName(const std::string& fname)
{
  fname_gval_ = fname;
}

//------------------------------------------------------------------------------
inline void SimData::SetLETFileName(const std::string& fname)
{
  fname_LET_ = fname;
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
inline void SimData::SetEndTime(double in)
{
  end_time_ = in;
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
inline std::vector<double>& SimData::GetTotElapTime()
{
  return tot_elap_time_;
}

//------------------------------------------------------------------------------
inline std::vector<double>& SimData::GetTotElapTimeChem()
{
  return tot_elap_time_chem_;
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
inline void SimData::RecordBenchmarkScoreForThreads(bool in)
{
  benchmark_threads_ = in;
}

//------------------------------------------------------------------------------
inline bool SimData::ThreadBenchmarkTestIsEnabled()
{
  return benchmark_threads_;
}

//------------------------------------------------------------------------------
inline void SimData::OutputEventInfo(bool in)
{
  dump_event_info_ = in;
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
inline void SimData::PushTimeStepInfo(
  int id, const TimeStepInfo& info, bool prestep)
{

  std::vector<std::vector<TimeStepInfo> >* buff = nullptr;
  if (prestep) { buff = &tsi_buff_pre_; }
  else { buff = &tsi_buff_; }

  if ((*buff)[id].size() == 0) {
    (*buff)[id].push_back(info);
    return;
  }

  auto itr = (*buff)[id].end(); itr--;
  if ((*itr).sim_time == info.sim_time) {
    (*buff)[id].erase(itr);
  }
  (*buff)[id].push_back(info);

}

//------------------------------------------------------------------------------
inline std::vector<TimeStepInfo>& SimData::GetTimeStepInfo(int id, bool prestep)
{
  if (prestep) { return tsi_buff_pre_[id]; }
  else { return tsi_buff_[id]; }
}

//------------------------------------------------------------------------------
inline void SimData::ClearTimeStepInfo(int id, bool prestep)
{
  if (prestep) { tsi_buff_pre_[id].clear(); }
  else { tsi_buff_[id].clear(); }
}

//------------------------------------------------------------------------------
inline std::map<std::string, int>& SimData::GetScoredMolecule()
{
  return mole_map_;
}

//------------------------------------------------------------------------------
inline void SimData::PushChemInfo(int id, const ChemInfo& info)
{
  ci_buff_[id].push_back(info);
}

//------------------------------------------------------------------------------
inline std::vector<ChemInfo>& SimData::GetChemInfo(int id)
{
  return ci_buff_[id];
}

//------------------------------------------------------------------------------
inline void SimData::PushLETInfo(int id, const LETInfo& info)
{
  LET_buff_[id].push_back(info);
}

//------------------------------------------------------------------------------
inline std::vector<LETInfo>& SimData::GetLETInfo(int id)
{
  return LET_buff_[id];
}

//------------------------------------------------------------------------------
inline void SimData::CountProcessedEvent(int id)
{
  num_processed_event_[id]++;
}

//------------------------------------------------------------------------------
inline void SimData::CheckProcessedEventNumber()
{
  tot_num_event_processed_ = std::reduce(num_processed_event_.begin(),
                                         num_processed_event_.end());
  std::cout << "[MESSAGE] event-loop check point: "
            << tot_num_event_processed_ << " events processed." << std::endl;
}

#endif // SIMDATA_H_
