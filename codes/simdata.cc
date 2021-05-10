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
#include "simdata.h"
#include "application.h"
#include "timehistory.h"
#include "physics_list.h"
#include "G4SystemOfUnits.hh"
#include "G4MolecularConfiguration.hh"
#include "G4MoleculeTable.hh"
#include "G4DNAMolecularReactionTable.hh"
#include "G4H2O.hh"
#include "G4Version.hh"
#if G4VERSION_NUMBER >= 1070
#include "G4FakeMolecule.hh"
#endif

#include <string>
#include <sstream>
#include <fstream>
#include <mutex>
#include <iomanip>

namespace {

#if G4VERSION_NUMBER < 1030

const int num_time_bin   = 60;
const int num_time_point = num_time_bin + 1;
const double low_tlim    = 1.0 * picosecond;
const double upp_tlim    = 999999.0 * picosecond;
const double bin_width   = log10(upp_tlim / low_tlim) / num_time_bin;

#else

constexpr int num_time_bin   = 60;
constexpr int num_time_point = num_time_bin + 1;
constexpr double low_tlim    = 1.0 * picosecond;
constexpr double upp_tlim    = 999999.0 * picosecond;
constexpr double bin_width   = log10(upp_tlim / low_tlim) / num_time_bin;

#endif

static int num_mole_kind;
static int matrix_size;

std::mutex mtx;

#if G4VERSION_NUMBER >= 1070

constexpr double dm = 0.1 * m;
constexpr double dm3 = dm * dm * dm;
constexpr double rate_unit = dm3 / (mole * s);

//------------------------------------------------------------------------------
void print_chemical_reaction()
{

  auto pname = PhysicsList::GetInstance()->GetChemistry()->GetPhysicsName();
//  if (pname != "G4EmDNAChemistry_option3") { return; }

  std::cout << "[Message] " << pname << " is set for the chemistry stage."
            << std::endl;

  auto tab = G4DNAMolecularReactionTable::GetReactionTable();
  auto datalist = tab->GetVectorOfReactionData();

  std::stringstream ss;

  ss << std::endl;

  for (auto& x : datalist) {

    auto reac1 = x->GetReactant1()->GetName();
    auto reac2 = x->GetReactant2()->GetName();

    ss << "--------------------------------------------------------------------"
       << std::endl;
    ss << reac1 << " + " << reac2 << " -> ";

    int num_prod = x->GetNbProducts();
    if (num_prod == 0) {
      ss << "None" << std::endl;
    } else {
      for (int i = 0; i < num_prod; i++) {
        auto prod = x->GetProduct(i);
        ss << prod->GetName();
        if (i == num_prod - 1) { ss << std::endl; }
        else { ss << " + "; }
      }
    }

    int type = x->GetReactionType();
    double rc = x->GetOnsagerRadius();

    if (type == 0 && rc == 0.0) {
      type = 1;
    } else if (type == 0 && rc != 0.0) {
      type = 3;
    } else if (type == 1 && rc == 0.0) {
      type = 2;
    } else if (type == 1 && rc != 0.0) {
      type = 4;
    }

    if (x->GetReactant1()->GetDiffusionCoefficient() == 0.0 ||
        x->GetReactant2()->GetDiffusionCoefficient() == 0.0) {
      type = 6;
    }

    double k_obs = x->GetObservedReactionRateConstant();
    double k_dif = x->GetDiffusionRateConstant();
    double k_act = x->GetActivationRateConstant();
    double sigma = x->GetReactionRadius();
    double Reff  = x->GetEffectiveReactionRadius() / nm;
    double prob  = x->GetProbability();

    if (type == 6) {

      ss << "--> Type: " << type << ", k_obs: " << k_obs * s << " s-1";

    } else {

      ss << "--> Type: " << type << ", k_obs: " << k_obs / rate_unit
         << " (M*s)-1, Reff: " << Reff << " nm, ";

      if (type == 1) {
        ss << "Preac: " << prob;
      } else if (type == 2) {
        double alpha = 1.0 / sigma * k_act / k_obs;
        ss << "k_dif: " << k_dif / rate_unit << " (M*s)-1, ";
        ss << "k_act: " << k_act / rate_unit << " (M*s)-1, ";
        ss << "Preac: " << prob << ", ";
        ss << "alpha: " << alpha / (1.0 / nm) << " nm-1";
      } else if (type == 3) {
        ss << ", rc: " << rc / nm << " nm ";
        ss << "Preac: " << prob;
      } else if (type == 4) {
        ss << "k_dif: " << k_dif / rate_unit << " (M*s)-1, ";
        ss << "k_act: " << k_act / rate_unit << " (M*s)-1, ";
        ss << "Preac: " << prob;
      }

    }

    ss << std::endl;

  }

  std::cout << ss.str() << std::endl;
}

#endif // G4VERSION_NUMBER >= 1070

//------------------------------------------------------------------------------
bool check_molecule_type(const G4MoleculeDefinition* part)
{
  bool skip = false;

  if (part == G4H2O::Definition()) { skip = true; }

#if G4VERSION_NUMBER >= 1070
  if (part == G4FakeMolecule::Definition()) { skip = true; }
#endif

  return skip;
}

} // end of anonymous namespace

//==============================================================================

SimData* SimData::instance_ = nullptr;

//------------------------------------------------------------------------------
SimData::SimData()
{
  fname_ = "result.csv";
  fname_bench_ = "benchmark.json";
  num_thread_ = 1;
  result_each_thread_ = false;
  performance_each_thread_ = true;
  dump_event_info_ = false;
}

//------------------------------------------------------------------------------
SimData* SimData::GetInstance()
{
  if (!instance_) { instance_ = new SimData(); }
  return instance_;
}

//------------------------------------------------------------------------------
void SimData::Setup()
{
  ::mtx.lock();

  static bool setup = false;

  if (setup) {
    ::mtx.unlock();
    return;
  }

  score_time_.resize(::num_time_point);
  double exponent = 0.0;
  for (int i = 0; i < ::num_time_point; i++) {
    if (i == 0) { exponent = log10(::low_tlim); }
    else { exponent += ::bin_width; }
    double t = pow(10.0, exponent);
    score_time_[i] = t;
  }

#if G4VERSION_NUMBER >= 1020
  auto miterator = G4MoleculeTable::Instance()->GetConfigurationIterator();
#else
  auto miterator = G4MoleculeTable::Instance()->GetDefintionIterator();
#endif

  int counter = 0;
  while ((miterator)()) {

    auto val = miterator.value();

#if G4VERSION_NUMBER >= 1020
    if (::check_molecule_type(val->GetDefinition())) { continue; }
#else
    if (::check_molecule_type(val)) { continue; }
#endif

    auto name = val->GetName();
    if (mole_map_.count(name)) { continue; }

    mole_map_.insert(std::pair<std::string, int>(name, counter));

    counter++;
  }

  ::num_mole_kind = counter;
  ::matrix_size = ::num_mole_kind * num_time_point;

  edep_buff_.resize(num_thread_, 0.0);
  gval_buff_.resize(num_thread_);

  for (int i = 0; i < num_thread_; i++) {
    gval_buff_[i].resize(::matrix_size, 0.0);
  }

  tsi_buff_pre_.resize(num_thread_);
  tsi_buff_.resize(num_thread_);

  ci_buff_.resize(num_thread_);

  header_.resize(::num_mole_kind + 1);
  header_[0] = "Time_ps";
  for (auto x: mole_map_) { header_[x.second + 1] = x.first; }

  num_abort_event_.resize(num_thread_, 0);
  num_chem_event_.resize(num_thread_, 0);

  tot_elap_time_.resize(num_thread_, 0.0);
  tot_elap_time_chem_.resize(num_thread_, 0.0);
  elap_time_chem_.resize(num_thread_, 0.0);

  num_phys_step_.resize(num_thread_, 0);
  num_chem_step_.resize(num_thread_, 0);

  setup = true;

#if G4VERSION_NUMBER >= 1070
  ::print_chemical_reaction();
#endif

  ::mtx.unlock();
}

//------------------------------------------------------------------------------
void SimData::GValue(int id, int tid, const std::string& name, double gval)
{

  int mid = mole_map_[name];
  int idx = tid * ::num_mole_kind + mid;

  if (idx >= ::matrix_size) {
    std::cerr << "[SimData::ERROR] matrix_size: " << ::matrix_size
              << ", idx: " << idx << std::endl;
    std::exit(EXIT_FAILURE);
  }

  gval_buff_[id][idx] += gval;
}

//------------------------------------------------------------------------------
void SimData::Merge()
{

  for (int id = 0; id < num_thread_; id++) {
    double factor = 1.0 / static_cast<double>(num_chem_event_[id]);
    auto& gval = gval_buff_[id];
    for (auto& x : gval) {
      x *= factor;
    }
  }

  if (num_thread_ == 1) { return; }

  auto& gval0 = gval_buff_[0];

  for (int id = 1; id < num_thread_; id++) {
    auto& gval = gval_buff_[id];
    for (int i = 0; i < ::matrix_size; i++) {
      gval0[i] += gval[i];
    }
  }

  double factor = 1.0 / static_cast<double>(num_thread_);
  for (auto& x: gval0) {
    x *= factor;
  }

}

//------------------------------------------------------------------------------
void SimData::SaveSimulationResult(int id)
{

  if (id >= 0 && !result_each_thread_) { return; }

  if (id < 0) { Merge(); }

  std::stringstream ss;

  int i = 0;
  const int vsize = header_.size() - 1;
  for (auto x : header_) {
    ss << x;
    if (i == vsize) { ss << std::endl; }
    else { ss << ","; }
    i++;
  }

  std::vector<double> gval;
  if (id < 0) {
    gval = gval_buff_[0];
  } else {
    gval = gval_buff_[id];
    double factor = 1.0 / static_cast<double>(num_chem_event_[id]);
    for (auto& x : gval) { x *= factor; }
  }

  i = 0;
  for (auto t : score_time_) {
    ss << t / picosecond << ",";
    for (int j = 0; j < ::num_mole_kind; j++) {
      int idx = i * ::num_mole_kind + j;
      ss << gval[idx];
      if (j == ::num_mole_kind - 1) { ss << std::endl; }
      else { ss << ","; }
    }
    i++;
  }

  std::ofstream fout;

  if (id < 0) {
    fout.open(fname_);
  } else {
    std::string fname;
    std::stringstream sline; sline << fname_;
    std::getline(sline, fname, '.');
    fname += "-th" + std::to_string(id) + ".csv";
    fout.open(fname);
  }

  fout << ss.str();
  fout.close();
}

//------------------------------------------------------------------------------
void SimData::Performance(int id)
{

  if (!performance_each_thread_) { return; }

  double elap_time = tot_elap_time_[id];
  double elap_time_chem = tot_elap_time_chem_[id];
  double elap_time_phys = elap_time - elap_time_chem;

  int num_event_chem  = num_chem_event_[id];
  int num_event_abort = num_abort_event_[id];
  int num_event = num_event_abort + num_event_chem;

  double avg_time_phys = elap_time_phys / num_event;
  double avg_time_chem = elap_time_chem / num_event_chem;

  double thr_phys = 60.0 / avg_time_phys;
  double thr_chem = 60.0 / avg_time_chem;

  int num_phys_step = num_phys_step_[id];
  int num_chem_step = num_chem_step_[id];
  double avg_time_phys_step = elap_time_phys / num_phys_step * 1000.0;
  double avg_time_chem_step = elap_time_chem / num_chem_step * 1000.0;

  int nmol = 0;
  auto ci = ci_buff_[id];
  for (auto x : ci) {
    for (auto y : x.species) { nmol += y.second; }
  }
  double avg_time_species = elap_time_chem / nmol * 1000.0;

  std::cout << " Thread #" << id << std::endl;
  std::cout << " - Total Event: " << num_event << " (Abort: " << num_event_abort
            << ", Chemistry: " << num_event_chem << ")" << std::endl;

  std::cout << " - Total Elapsed Time: " << elap_time << " sec" << std::endl;
  std::cout << "   - Physics:   " << elap_time_phys << " sec ("
            << avg_time_phys << " sec/event)" << std::endl;
  std::cout << "   - Chemistry: " << elap_time_chem << " sec ("
            << avg_time_chem << " sec/event)" << std::endl;

  std::cout << " - Throughput:" << std::endl;
  std::cout << "   - Physics:   " << thr_phys << " events/min." << std::endl;
  std::cout << "   - Chemistry: " << thr_chem << " events/min." << std::endl;

  std::cout << " - Number of Steps:" << std::endl;
  std::cout << "   - Physics:   " << num_phys_step << std::endl;
  std::cout << "   - Chemistry: " << num_chem_step << std::endl;

  std::cout << " - Elapsed Time per Step:" << std::endl;
  std::cout << "   - Physics:   " << avg_time_phys_step
            << " msec/step" << std::endl;
  std::cout << "   - Chemistry: " << avg_time_chem_step
            << " msec/step" << std::endl;

  std::cout << " - Elapsed Time per Species" << std::endl;
  std::cout << "   - # of species: " << nmol << std::endl;
  std::cout << "   - Throughput:   " << avg_time_species
            << " msec/species" << std::endl;

  std::cout << std::endl;

  std::string title = "thread" + std::to_string(id);

  js_[title] = {
    {"event_number",             {num_event, num_event_abort, num_event_chem}},
    {"elapsed_time",             {elap_time, elap_time_phys, elap_time_chem}},
    {"elapsed_time_per_event",   {avg_time_phys, avg_time_chem}},
    {"throughput",               {thr_phys, thr_chem}},
    {"step_number",              {num_phys_step, num_chem_step}},
    {"elapsed_time_per_step",    {avg_time_phys_step, avg_time_chem_step}},
    {"elapsed_time_per_species", {nmol, avg_time_species}}
  };

}

//------------------------------------------------------------------------------
void SimData::SaveBenchmarkResult()
{

  int num_event = Application::GetInstance()->GetEventNumber();

  auto timer = TimeHistory::GetTimeHistory();
  double elap_time = timer->GetTime("BeamEnd") - timer->GetTime("BeamOn");
  double throughput = double(num_event) / elap_time * 60.0;

  std::cout << std::endl;
  std::cout << " - Total Event Number: "    << num_event << std::endl;
  std::cout << " - Total Elapsed Time: " << elap_time<< " sec" << std::endl;
  std::cout << "   --> Throughput: " << throughput << " events/min."
            << std::endl;
  std::cout << std::endl;

  js_["all"] = {
    {"event_number", num_event},
    {"elapsed_time", elap_time},
    {"throughput", throughput}
  };

  if (dump_event_info_) {

    std::vector<int> mole_number;
    std::vector<double> proc_time;

    for (int id = 0; id < num_thread_; id++) {
      auto ci = ci_buff_[id];
      for (auto x : ci) {
        int num = 0;
        for (auto y : x.species) { num += y.second; }
        proc_time.push_back(x.proc_time);
        mole_number.push_back(num);
      }
    }

    js_["chemistry_stage"] = {
      {"mole_number", mole_number},
      {"proc_time",   proc_time}
    };

  }

  // save benchmark result
  std::ofstream fout(fname_bench_);
  fout << std::setw(4) << js_ << std::endl;
  fout.close();

}
