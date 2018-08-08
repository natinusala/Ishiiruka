
#include <libretro.h>

#include "DolphinLibretro/Options.h"

namespace Libretro
{
extern retro_environment_t environ_cb;

namespace Options
{
static std::vector<retro_variable> optionsList;
static std::vector<bool*> dirtyPtrList;

template <typename T>
void Option<T>::Register()
{
  if (!m_options.empty())
    return;

  m_options = m_name;
  m_options.push_back(';');
  for (auto& option : m_list)
  {
    if (option.first == m_list.begin()->first)
      m_options += std::string(" ") + option.first;
    else
      m_options += std::string("|") + option.first;
  }
  optionsList.push_back({m_id, m_options.c_str()});
  dirtyPtrList.push_back(&m_dirty);
  Updated();
  m_dirty = true;
}

void SetVariables()
{
  if (optionsList.empty())
    return;
  if (optionsList.back().key)
    optionsList.push_back({});
  environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)optionsList.data());
}

void CheckVariables()
{
  bool updated = false;
  if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && !updated)
    return;

  for (bool* ptr : dirtyPtrList)
    *ptr = true;
}

template <typename T>
Option<T>::Option(const char* id, const char* name,
                  std::initializer_list<std::pair<const char*, T>> list)
    : m_id(id), m_name(name), m_list(list.begin(), list.end())
{
  Register();
}

template <typename T>
Option<T>::Option(const char* id, const char* name, std::initializer_list<const char*> list)
    : m_id(id), m_name(name)
{
  for (auto option : list)
    m_list.push_back({option, (T)m_list.size()});
  Register();
}
template <>
Option<std::string>::Option(const char* id, const char* name,
                            std::initializer_list<const char*> list)
    : m_id(id), m_name(name)
{
  for (auto option : list)
    m_list.push_back({option, option});
  Register();
}
template <>
Option<const char*>::Option(const char* id, const char* name,
                            std::initializer_list<const char*> list)
    : m_id(id), m_name(name)
{
  for (auto option : list)
    m_list.push_back({option, option});
  Register();
}

template <typename T>
Option<T>::Option(const char* id, const char* name, T first,
                  std::initializer_list<const char*> list)
    : m_id(id), m_name(name)
{
  for (auto option : list)
    m_list.push_back({option, first + (int)m_list.size()});
  Register();
}

template <typename T>
Option<T>::Option(const char* id, const char* name, T first, int count, int step)
    : m_id(id), m_name(name)
{
  for (T i = first; i < first + count; i += step)
    m_list.push_back({std::to_string(i), i});
  Register();
}

template <>
Option<bool>::Option(const char* id, const char* name, bool initial) : m_id(id), m_name(name)
{
  m_list.push_back({initial ? "enabled" : "disabled", initial});
  m_list.push_back({!initial ? "enabled" : "disabled", !initial});
  Register();
}

template <typename T>
bool Option<T>::Updated()
{
  if (m_dirty)
  {
    m_dirty = false;

    retro_variable var{m_id};
    T value = m_list.front().second;

    if (environ_cb && environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
      for (auto option : m_list)
      {
        if (option.first == var.value)
        {
          value = option.second;
          break;
        }
      }
    }

    if (m_value != value)
    {
      m_value = value;
      return true;
    }
  }
  return false;
}

Option<int> efbScale("ishiiruka_efb_scale", "EFB Scale", 1,
                     {"x1 (640 x 528)", "x2 (1280 x 1056)", "x3 (1920 x 1584)", "x4 (2560 * 2112)",
                      "x5 (3200 x 2640)", "x6 (3840 x 3168)"});
Option<LogTypes::LOG_LEVELS> logLevel("ishiiruka_log_level", "Log Level",
                                      {{"Info", LogTypes::LINFO},
#if defined(_DEBUG) || defined(DEBUGFAST)
                                       {"Debug", LogTypes::LDEBUG},
#endif
                                       {"Notice", LogTypes::LNOTICE},
                                       {"Error", LogTypes::LERROR},
                                       {"Warning", LogTypes::LWARNING}});
Option<float> cpuClockRate("ishiiruka_cpu_clock_rate", "CPU Clock Rate",
                           {{"100%", 1.0},
                            {"150%", 1.5},
                            {"200%", 2.0},
                            {"250%", 2.5},
                            {"300%", 3.0},
                            {"5%", 0.05},
                            {"10%", 0.1},
                            {"20%", 0.2},
                            {"30%", 0.3},
                            {"40%", 0.4},
                            {"50%", 0.5},
                            {"60%", 0.6},
                            {"70%", 0.7},
                            {"80%", 0.8},
                            {"90%", 0.9}});
Option<std::string> renderer("ishiiruka_renderer", "Renderer", {"Hardware", "Software", "Null"});
Option<bool> fastmem("ishiiruka_fastmem", "Fastmem", true);
Option<bool> DSPHLE("ishiiruka_dsp_hle", "DSP HLE", true);
Option<bool> DSPEnableJIT("ishiiruka_dsp_jit", "DSP Enable JIT", true);
Option<int> cpu_core("ishiiruka_cpu_core", "CPU Core",
                                  {
#ifdef _M_X86
                                      {"JIT64", PowerPC::CPUCore::CORE_JIT64},
#elif _M_ARM_64
                                      {"JITARM64", PowerPC::CPUCore::CORE_JITARM64},
#endif
                                      {"Interpreter", PowerPC::CPUCore::CORE_INTERPRETER},
                                      {"Cached Interpreter", PowerPC::CPUCore::CORE_CACHEDINTERPRETER}});
Option<DiscIO::Language> Language("ishiiruka_language", "Language",
                                  {{"English", DiscIO::Language::English},
                                   {"Japanese", DiscIO::Language::Japanese},
                                   {"German", DiscIO::Language::German},
                                   {"French", DiscIO::Language::French},
                                   {"Spanish", DiscIO::Language::Spanish},
                                   {"Italian", DiscIO::Language::Italian},
                                   {"Dutch", DiscIO::Language::Dutch},
                                   {"Simplified Chinese", DiscIO::Language::SimplifiedChinese},
                                   {"Traditional Chinese", DiscIO::Language::TraditionalChinese},
                                   {"Korean", DiscIO::Language::Korean}});
Option<bool> Widescreen("ishiiruka_widescreen", "Widescreen", true);
Option<bool> WidescreenHack("ishiiruka_widescreen_hack", "WideScreen Hack", false);
Option<bool> progressiveScan("ishiiruka_progressive_scan", "Progressive Scan", true);
Option<bool> pal60("ishiiruka_pal60", "PAL60", true);
Option<u32> sensorBarPosition("ishiiruka_sensor_bar_position", "Sensor Bar Position",
                              {"Bottom", "Top"});
Option<unsigned int> audioMixerRate("ishiiruka_mixer_rate", "Audio Mixer Rate",
                                    {{"32000", 32000u}, {"48000", 48000u}});
#if 0
Option<ShaderCompilationMode> shaderCompilationMode(
    "ishiiruka_shader_compilation_mode", "Shader Compilation Mode",
    {{"sync UberShaders", ShaderCompilationMode::SynchronousUberShaders},
     {"sync", ShaderCompilationMode::Synchronous},
     {"a-sync UberShaders", ShaderCompilationMode::AsynchronousUberShaders},
     {"a-sync Skip Rendering", ShaderCompilationMode::AsynchronousSkipRendering}});
#endif
}  // namespace Options
}  // namespace Libretro
