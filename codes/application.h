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
#ifndef APPLICATION_H_
#define APPLICATION_H_
#include "G4VUserActionInitialization.hh"
#include <string>

class Application : public G4VUserActionInitialization {
public:
  static Application* GetInstance();
  virtual ~Application() = default;

  Application(const Application&) = delete;
  void operator=(const Application&) = delete;

  virtual void BuildForMaster() const;
  virtual void Build() const;

  void LoadConfigFile(const std::string& conf_file);
  void SetupRandomEngine(long seed);
  void Setup();

  int GetEventNumber() const;
  int GetThreadNumber() const;

  bool PrimaryRemovalIsEnabled() const;
  double GetKillEnergyUppLim() const;
  double GetKillEnergyLowLim() const;

  void SetOutputFile(const std::string& output);

private:
  Application();
  static Application* instance_;

  int num_event_;
  int num_thread_;

  bool primary_removal_;
  double kill_eupp_;
  double kill_elow_;

  std::string output_;
};

//==============================================================================
inline int Application::GetEventNumber() const
{
  return num_event_;
}

//------------------------------------------------------------------------------
inline int Application::GetThreadNumber() const
{
  return num_thread_;
}

//------------------------------------------------------------------------------
inline bool Application::PrimaryRemovalIsEnabled() const
{
  return primary_removal_;
}

//------------------------------------------------------------------------------
inline double Application::GetKillEnergyUppLim() const
{
  return kill_eupp_;
}

//------------------------------------------------------------------------------
inline double Application::GetKillEnergyLowLim() const
{
  return kill_elow_;
}

//------------------------------------------------------------------------------
inline void Application::SetOutputFile(const std::string& output)
{
  output_ = output;
}

#endif // APPLICATION_H_
