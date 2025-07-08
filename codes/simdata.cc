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
#include "G4Threading.hh"

#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

namespace {

int num_node;
int num_mole_kind;
int matrix_size;
constexpr int kNumTimeBinPerOrderOfMagnitude = 10;
constexpr double kTimeLowLim = 1.0 * picosecond;

#if G4VERSION_NUMBER >= 1070

constexpr double dm = 0.1 * m;
constexpr double dm3 = dm * dm * dm;
constexpr double rate_unit = dm3 / (mole * s);

//------------------------------------------------------------------------------
void show_line(std::stringstream& message)
{
  message <<
"------------------------------------------------------------------------------"
  << std::endl;
}

//------------------------------------------------------------------------------
void show_value(const std::string& kind, const int precision, const double val,
                const std::string& unit, std::stringstream& message,
                bool exponential = false)
{
  if (exponential) {
    message << kind << ": " << std::scientific << std::setprecision(precision);
  } else {
    message << kind << ": " << std::fixed << std::setprecision(precision);
  }
  message << val << " " << unit;
  message << std::defaultfloat;
}

//------------------------------------------------------------------------------
void print_chemical_reaction()
{
  constexpr bool exponential = true;

  auto tab = G4DNAMolecularReactionTable::GetReactionTable();
  auto datalist = tab->GetVectorOfReactionData();

  std::stringstream ss;

  ss << "[Message] Chemical reactions" << std::endl;

  for (auto& x : datalist) {

    auto reac1 = x->GetReactant1()->GetName();
    auto reac2 = x->GetReactant2()->GetName();

    show_line(ss);

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

      ss << "--> Type: " << type << ", ";

      show_value("k_obs", 2, k_obs * s, "s^-1", ss, exponential);


    } else {

      ss << "--> Type: " << type << ", ";

      show_value("k_obs", 2, k_obs / rate_unit, "(M*s)^-1, ", ss, exponential);

      show_value("Reff", 2, Reff, "nm, ", ss);

      if (type == 1) {

        show_value("Preac", 3, prob * 100.0, "%", ss);

      } else if (type == 2) {
        double alpha = 1.0 / sigma * k_act / k_obs;

        show_value("k_dif", 2, k_dif / rate_unit, "(M*s)^-1, ",
                   ss, exponential);

        ss << "\n    ";

        show_value("k_act", 2, k_act / rate_unit, "(M*s)^-1, ",
                   ss, exponential);

        show_value("Preac", 3, prob * 100.0, "%, ", ss);

        show_value("alpha", 3, alpha / (1.0 / nm), "nm^-1", ss);

      } else if (type == 3) {

        show_value("rc", 2, rc / nm, "nm,", ss);

        ss << "\n    ";

        show_value("Preac", 3, prob * 100.0, "%", ss);

      } else if (type == 4) {

        show_value("k_dif", 2, k_dif / rate_unit, "(M*s)^-1, ",
                   ss, exponential);

        ss << "\n    ";

        show_value("k_act", 2, k_act / rate_unit, "(M*s)^-1, ",
                   ss, exponential);

        show_value("Preac", 3, prob * 100.0, "%", ss);

      }

    }

    ss << std::endl;

  }

  show_line(ss);

  std::cout << ss.str() << std::endl;
}

#endif // G4VERSION_NUMBER >= 1070

//------------------------------------------------------------------------------
bool check_molecule_type(const G4MolecularConfiguration* mconf)
{
  bool skip = false;
  const G4MoleculeDefinition* part = mconf->GetDefinition();

  if (part == G4H2O::Definition()) { skip = true; }

#if G4VERSION_NUMBER >= 1070
  if (part == G4FakeMolecule::Definition()) { skip = true; }
#endif

#if G4VERSION_NUMBER >= 1130
  if (mconf->GetName() == "O^0") { skip = true; }
#endif

  return skip;
}

} // end of anonymous namespace

//==============================================================================

SimData* SimData::instance_ = nullptr;

//------------------------------------------------------------------------------
SimData::SimData()
{
  fname_gval_  = "result_gval.csv";
  fname_LET_   = "result_LET.csv";
  fname_bench_ = "benchmark.json";
  num_thread_ = 1;
  result_each_thread_ = false;
  benchmark_threads_ = false;
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
  const auto time_upplim = end_time_ - ::kTimeLowLim;
  const auto num_bin = ::kNumTimeBinPerOrderOfMagnitude
    * static_cast<int>(log10(round(time_upplim) / ::kTimeLowLim));
  const auto bin_width = log10(time_upplim / ::kTimeLowLim) / num_bin;
  ::num_node = num_bin + 1;

  score_time_.resize(::num_node);

  double exponent{0.0};
  for (int i = 0; i < ::num_node; i++) {
    if (i == 0) { exponent = log10(::kTimeLowLim); }
    else { exponent += bin_width; }
    double t = pow(10.0, exponent);
    score_time_[i] = t;
  }

  auto miterator = G4MoleculeTable::Instance()->GetConfigurationIterator();
  const auto H3OpB = G4MoleculeTable::Instance()->GetConfiguration("H3Op(B)");
  const auto OHmB  = G4MoleculeTable::Instance()->GetConfiguration("OHm(B)");

  int counter{0};
  while ((miterator)()) {

    const G4MolecularConfiguration* mconf = miterator.value();

    if (::check_molecule_type(mconf)) { continue; }

    auto name = mconf->GetName();
    if (mconf == H3OpB || mconf == OHmB) {
      std::cout << ">> " << name << std::endl;
    }

    if (mole_map_.count(name)) { continue; }

    mole_map_.insert(std::pair<std::string, int>(name, counter));

    counter++;
  }

  ::num_mole_kind = counter;
  ::matrix_size = ::num_mole_kind * ::num_node;

  gval_buff_.resize(num_thread_);

  for (int i = 0; i < num_thread_; i++) {
    gval_buff_[i].resize(::matrix_size, 0.0);
  }

  tsi_buff_pre_.resize(num_thread_);
  tsi_buff_.resize(num_thread_);

  ci_buff_.resize(num_thread_);

  LET_buff_.resize(num_thread_);

  header_.resize(::num_mole_kind + 1);
  header_[0] = "Time(ps)";
  for (auto x: mole_map_) { header_[x.second + 1] = x.first; }

  num_abort_event_.resize(num_thread_, 0);
  num_chem_event_.resize(num_thread_, 0);

  tot_elap_time_.resize(num_thread_, 0.0);
  tot_elap_time_chem_.resize(num_thread_, 0.0);
  elap_time_chem_.resize(num_thread_, 0.0);

  num_phys_step_.resize(num_thread_, 0);
  num_chem_step_.resize(num_thread_, 0);

#if G4VERSION_NUMBER >= 1070
  ::print_chemical_reaction();
#endif

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
double SimData::GetGValue(int id, int tid, const std::string& name)
{
  int mid = mole_map_[name];
  int idx = tid * ::num_mole_kind + mid;
  return gval_buff_[id][idx];
}

//------------------------------------------------------------------------------
void SimData::Merge()
{
  int num_thread{0};
  for (int id = 0; id < num_thread_; id++) {
    // check event number processed by each worker thread
    auto& num_event = num_chem_event_[id];
    if (num_event == 0) { continue; }

    double factor = 1.0 / static_cast<double>(num_event);
    auto& gval = gval_buff_[id];
    for (auto& x : gval) { x *= factor; }

    num_thread++; // count a worker thread which processed events
  }

  if (num_thread_ == 1) { return; }

  auto& gval0 = gval_buff_[0];

  for (int id = 1; id < num_thread_; id++) {
    if (num_chem_event_[id] == 0) { continue; }
    auto& gval = gval_buff_[id];
    for (int i = 0; i < ::matrix_size; i++) {
      gval0[i] += gval[i];
    }
  }

  double factor = 1.0 / static_cast<double>(num_thread);
  for (auto& x: gval0) { x *= factor; }

}

//------------------------------------------------------------------------------
void SimData::SaveSimulationResult(int id)
{

  if (id >= 0 && !result_each_thread_) { return; }

  // ===========================================================================
  //  save G value time profile
  // ===========================================================================
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
    fout.open(fname_gval_);
  } else {
    std::string fname;
    std::stringstream sline; sline << fname_gval_;
    std::getline(sline, fname, '.');
    fname += "-th" + std::to_string(id) + ".csv";
    fout.open(fname);
  }

  fout << ss.str();
  fout.close();

  // ===========================================================================
  //  save energy deposit and LET for each event
  // ===========================================================================
  if (id < 0) {

    ss.str(""); // clear
    ss << "EnergyDeposit(eV),LET(keV/um)" << std::endl;

    for (i = 0; i < num_thread_; i++) {
      const auto& buff = GetLETInfo(i);
      for (const auto& x : buff) {
        ss << x.first << "," << x.second << std::endl;
      }
    }

    fout.open(fname_LET_);
    fout << ss.str();
    fout.close();

  }

}

//------------------------------------------------------------------------------
void SimData::Performance(int id)
{

  if (!benchmark_threads_) { return; }

  auto elap_time = tot_elap_time_[id];
  auto elap_time_chem = tot_elap_time_chem_[id];
  auto elap_time_phys = elap_time - elap_time_chem;

  auto num_event_chem  = num_chem_event_[id];
  auto num_event_abort = num_abort_event_[id];
  auto num_event = num_event_abort + num_event_chem;

  auto eps = static_cast<double>(num_event) / elap_time * 60.0;
  auto eps_phys = static_cast<double>(num_event) / elap_time_phys * 60.0;
  auto eps_chem = static_cast<double>(num_event_chem) / elap_time_chem * 60.0;

  int nmol{0};
  auto ci = ci_buff_[id];
  for (auto x : ci) {
    for (auto y : x.species) { nmol += y.second; }
  }

  auto mps = static_cast<double>(nmol) / elap_time_chem;
  auto avg_nmol = static_cast<double>(nmol) / num_event_chem;

  std::stringstream ss;

  ss << " [Run Summary : Thread #" << id << "]" << std::endl;
  ss << " * Event Number: " << num_event << std::endl;
  ss << " * Simulation Time : " << elap_time << " sec" << std::endl;
  ss << "     -> EPS Score  : " << eps << " Events/min" << std::endl;
  ss << " * Physics Stage" << std::endl;
  ss << "   - Event Number  : " << num_event << std::endl;
  ss << "   - Elapsed Time  : " << elap_time_phys << " sec" << std::endl;
  ss << "     -> EPS Score  : " << eps_phys << " Events/min" << std::endl;
  ss << " * Chemistry Stage" << std::endl;
  ss << "   - Event Number  : " << num_event_chem
     << " (Abort Events : " << num_event_abort << ")" << std::endl;
  ss << "   - Molecule Number @1ps : " << nmol
     << " (Average : " << avg_nmol << " /Event)" << std::endl;
  ss << "   - Elapsed Time  : " << elap_time_chem << " sec" << std::endl;
  ss << "     -> EPS Score  : " << eps_chem << " Events/min" << std::endl;
  ss << "     -> MPS Score  : " << mps << " Molecules/sec" << std::endl;

  std::cout << ss.str() << std::endl;

  std::string title = "thread" + std::to_string(id);

  js_[title] = {
    {"EventNumber", num_event},
    {"SimulationTime", elap_time},
    {"EPSScore", eps},
    {"PhysicsStage", {
      {"EventNumber", num_event},
      {"ElapsedTime", elap_time_phys},
      {"EPSScore", eps_phys}
    }},
    {"ChemistryStage", {
      {"EventNumber", num_event_chem},
      {"MoleculeNumber", nmol},
      {"ElapsedTime", elap_time_chem},
      {"EPSScore", eps_chem},
      {"MPSScore", mps}
    }}
  };

}

//------------------------------------------------------------------------------
void SimData::SaveBenchmarkResult()
{

  int num_event = Application::GetInstance()->GetEventNumber();

  auto timer = TimeHistory::GetTimeHistory();
  double elap_time = timer->GetTime("RunEnd") - timer->GetTime("RunOn");
  double throughput = double(num_event) / elap_time * 60.0;

  double gval_OH[2]   = { GetGValue(0, 0, "OH^0"),
                          GetGValue(0, ::num_node - 1, "OH^0") };

  double gval_eaq[2]  = { GetGValue(0, 0, "e_aq^-1"),
                          GetGValue(0, ::num_node - 1, "e_aq^-1") };

  double gval_H2O2[2] = { GetGValue(0, 0, "H2O2^0"),
                          GetGValue(0, ::num_node - 1, "H2O2^0") };

  std::stringstream msg;

  msg << "\n================================================================\n";
  msg << " Run summary\n";
  msg << " - Thread Number: " << num_thread_ << "\n";
  msg << " - Total Event Number: " << num_event << "\n";
  msg << " - Total Elapsed Time: " << elap_time<< " sec\n";
  msg << " - Throughput: " << throughput << " events/min.\n";
  msg << " *** Physics Regression (G-value)\n";
  msg << " - Hydroxyl radical: " << gval_OH[0] << ", " << gval_OH[1] << "\n";
  msg << " - Solvated electron: " << gval_eaq[0] << ", " << gval_eaq[1] << "\n";
  msg << " - H2O2: " << gval_H2O2[0] << ", " << gval_H2O2[1] << "\n";
  msg << "================================================================\n";

  std::cout << msg.str() << std::endl;

  js_["summary"] = {
    {"thread_number", num_thread_},
    {"event_number", num_event},
    {"elapsed_time", elap_time},
    {"throughput", throughput},
    {"gvalue", {
      {"hydroxyl_radical", {
        {"chem_start", gval_OH[0]}, {"chem_end", gval_OH[1]}
      }},
      {"solvated_electron", {
        {"chem_start", gval_eaq[0]}, {"chem_end", gval_eaq[1]}
      }},
      {"H2O2", {
        {"chem_start", gval_H2O2[0]}, {"chem_end", gval_H2O2[1]}
      }}
    }}
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
  fout << std::setw(2) << js_ << std::endl;
  fout.close();

}
