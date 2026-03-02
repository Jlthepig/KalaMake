//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <string>
#include <vector>
#include <filesystem>

#include "KalaHeaders/core_utils.hpp"
#include "KalaHeaders/log_utils.hpp"

#include "language/kma_language_c_cpp.hpp"
#include "core/kma_core.hpp"

using KalaHeaders::KalaCore::EnumToString;
using KalaHeaders::KalaCore::RemoveDuplicates;

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::LogType;

using KalaMake::Core::KalaMakeCore;
using KalaMake::Language::GlobalData;
using KalaMake::Core::BinaryType;
using KalaMake::Core::CompilerType;
using KalaMake::Core::StandardType;
using KalaMake::Core::BuildType;
using KalaMake::Core::WarningLevel;
using KalaMake::Core::CustomFlag;

using std::string;
using std::string_view;
using std::vector;
using std::filesystem::path;
using std::filesystem::exists;
using std::filesystem::is_regular_file;
using std::filesystem::is_directory;

static void PreCheck(GlobalData& globalData);

static void Compile_General(const GlobalData& globalData);
static void Compile_Static(const GlobalData& globalData);

namespace KalaMake::Language
{
	constexpr string_view cl_ide_bat_2026 = "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat";
	constexpr string_view cl_build_bat_2026 = "C:\\Program Files (x86)\\Microsoft Visual Studio\\18\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat";
	constexpr string_view cl_ide_bat_2022 = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat";
	constexpr string_view cl_build_bat_2022 = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat";

	static path foundCLPath{};

	void Language_C_CPP::Compile(GlobalData& globalData)
	{
		PreCheck(globalData);

		BinaryType binaryType = globalData.targetProfile.binaryType;

		//continue to static lib compilation function
		//since its very different from exe and shared and dynamic lib
		if (binaryType == BinaryType::B_STATIC)
		{	
			Compile_Static(globalData);
			return;
		}

		//regular compilation for executable, shared and dynamic binaries
		Compile_General(globalData);
	}

	void Language_C_CPP::Generate(GlobalData& globalData)
	{
		PreCheck(globalData);
	}
}

void PreCheck(GlobalData& globalData)
{		
	StandardType standard = globalData.targetProfile.standard;
	bool isCLanguage =
		standard == StandardType::C_89
		|| standard == StandardType::C_99
		|| standard == StandardType::C_11
		|| standard == StandardType::C_17
		|| standard == StandardType::C_23
		|| standard == StandardType::C_LATEST;

	bool isCPPLanguage =
		standard == StandardType::CPP_03
		|| standard == StandardType::CPP_11
		|| standard == StandardType::CPP_14
		|| standard == StandardType::CPP_17
		|| standard == StandardType::CPP_20
		|| standard == StandardType::CPP_23
		|| standard == StandardType::CPP_LATEST;

	auto should_remove = [isCLanguage, isCPPLanguage](
		const path& target, 
		bool isSource) -> bool
		{
			if (!isSource
				&& (!exists(target)
				|| !is_directory(target)))
			{
				return true;
			}

			if (isSource
				&& (!exists(target)
				|| is_directory(target)
				|| !is_regular_file(target)
				|| !target.has_extension()))
			{
				return true;
			}

			if (!isCLanguage
				&& !isCPPLanguage)
			{
				return true;
			}

			string extension = target.extension();

			if (isCLanguage)
			{
				if (isSource
					&& target.extension() != ".c")
				{
					return true;
				}
			}
			if (isCPPLanguage)
			{
				if (isSource
					&& target.extension() != ".c"
					&& target.extension() != ".cpp")
				{
					return true;
				}
			}

			return false;
		};

	vector<path>& sources = globalData.targetProfile.sources;
	for (auto it = sources.begin(); it != sources.end();)
	{
		path target = *it;
		if (should_remove(target, true))
		{
			Log::Print(
				"Removed invalid source script path '" + target.string() + "'",
				"LANGUAGE_C_CPP",
				LogType::LOG_INFO);

			sources.erase(it);
		}
		else ++it;
	}

	vector<path>& headers = globalData.targetProfile.headers;
	for (auto it = headers.begin(); it != headers.end();)
	{
		path target = *it;
		if (should_remove(target, false))
		{
			Log::Print(
				"Removed invalid header script path '" + target.string() + "'",
				"LANGUAGE_C_CPP",
				LogType::LOG_INFO);

			headers.erase(it);
		}
		else ++it;
	}

	if (sources.empty())
	{
		KalaMakeCore::CloseOnError(
			"LANGUAGE_C_CPP",
			"No sources were remaining after cleaning source scripts list!");
	}
}

void Compile_General(const GlobalData& globalData)
{
	Log::Print("\n==========================================================================================\n");

	bool isCL = globalData.targetProfile.compiler == CompilerType::C_CL;

	string frontArg = isCL
		? "/"
		: "-";
	
	string command{};

	//set compiler

	string_view compiler{};
	EnumToString(globalData.targetProfile.compiler, KalaMakeCore::GetCompilerTypes(), compiler);
	command += string(compiler);

	//set standard

	string_view standard{};
	EnumToString(globalData.targetProfile.standard, KalaMakeCore::GetStandardTypes(), standard);

	string standardArg = isCL
		? frontArg + "std:"
		: frontArg + "std=";

	command += " " + standardArg + string(standard);

	//set output

	string outputArg = isCL
		? frontArg + "Fe:"
		: frontArg + "o ";

	path buildPath = globalData.targetProfile.buildPath;
	string binaryName = globalData.targetProfile.binaryName;

	string extension{};
	if (globalData.targetProfile.binaryType == BinaryType::B_EXECUTABLE)
	{
#ifdef _WIN32
		if (!binaryName.ends_with(".exe")) extension = ".exe";
#endif
	}
	else if (globalData.targetProfile.binaryType == BinaryType::B_SHARED
			 || globalData.targetProfile.binaryType == BinaryType::B_DYNAMIC)
	{
#ifdef _WIN32
		if (!binaryName.ends_with(".dll")) extension = ".dll";
#else
		if (!binaryName.ends_with(".so")) extension = ".so";
#endif
	}

	command += " " + outputArg + "\"" + path(buildPath / string(binaryName + extension)).string() + "\"";

	//set defines

	string defineArg = frontArg + "D";

	for (const auto& d : globalData.targetProfile.defines)
	{
		command += " " + defineArg + d;
	}

	//set flags and warning level

	vector<string> finalFlags = globalData.targetProfile.flags;

	switch (globalData.targetProfile.buildType)
	{
	case BuildType::B_DEBUG:
	{
		if (isCL)
		{
			finalFlags.push_back("Zi");
			finalFlags.push_back("Od");
		}
		else
		{
			finalFlags.push_back("g");
			finalFlags.push_back("O0");
		}
		break;
	}
	case BuildType::B_RELEASE:
	{
		finalFlags.push_back("O2");
		break;
	}
	case BuildType::B_RELDEBUG:
	{
		if (isCL)
		{
			finalFlags.push_back("Zi");
			finalFlags.push_back("O2");
		}
		else
		{
			finalFlags.push_back("g");
			finalFlags.push_back("O2");
		}
		break;
	}
	case BuildType::B_MINSIZEREL:
	{
		if (isCL) finalFlags.push_back("O1");
		else      finalFlags.push_back("Os");
	}

	default: break;
	}

	bool isMSVC = 
		isCL
		|| globalData.targetProfile.compiler == CompilerType::C_CLANG_CL;

	switch (globalData.targetProfile.warningLevel)
	{
	case WarningLevel::W_BASIC:
	{
		if (isMSVC) finalFlags.push_back("W1");
		else        finalFlags.push_back("Wall");
		break;
	}
	case WarningLevel::W_NORMAL:
	{
		if (isMSVC) finalFlags.push_back("W3");
		else
		{
			finalFlags.push_back("Wall");
			finalFlags.push_back("Wextra");
		}
		break;
	}
	case WarningLevel::W_STRONG:
	{
		if (isMSVC) finalFlags.push_back("W4");
		else
		{
			finalFlags.push_back("Wall");
			finalFlags.push_back("Wextra");
			finalFlags.push_back("Wpedantic");
		}
		break;
	}
	case WarningLevel::W_STRICT:
	{
		if (isMSVC)
		{
			finalFlags.push_back("W4");
			finalFlags.push_back("permissive-");
		}
		else
		{
			finalFlags.push_back("Wall");
			finalFlags.push_back("Wextra");
			finalFlags.push_back("Wpedantic");
			finalFlags.push_back("Wconversion");
			finalFlags.push_back("Wsign-conversion");
		}
		break;
	}
	case WarningLevel::W_ALL:
	{
		if (isMSVC) finalFlags.push_back("Wall");
		else if (globalData.targetProfile.compiler == CompilerType::C_CLANG
				 || globalData.targetProfile.compiler == CompilerType::C_CLANGPP)
		{
			finalFlags.push_back("Weverything");
		}
		else
		{
			finalFlags.push_back("Wall");
			finalFlags.push_back("Wextra");
			finalFlags.push_back("Wpedantic");
			finalFlags.push_back("Wconversion");
			finalFlags.push_back("Wsign-conversion");
		}
		break;
	}

	default: break;
	}

	RemoveDuplicates(finalFlags);
	for (const auto& f : finalFlags)
	{
		command += " " + frontArg + f;
	}

	//set sources

	for (const auto& s : globalData.targetProfile.sources)
	{
		command += " " + string(s);
	}

	//set headers

	if (!globalData.targetProfile.headers.empty())
	{
		for (const auto& h : globalData.targetProfile.headers)
		{
			command += " " + frontArg + "I\"" + h.string() + "\"";
		}
	}

	//set links

	if (!globalData.targetProfile.links.empty())
	{
		for (const auto& l : globalData.targetProfile.links)
		{
			string linkArg = l.string().find('.') == string::npos
				? frontArg + 'l'
				: string{};

			command += " " + linkArg + "\"" + l.string() + "\"";
		}
	}

	//compile

	Log::Print(
		"Output:\n" + command + "\n",
		"LANGUAGE_C_CPP",
		LogType::LOG_INFO);

	system(command.c_str());

	Log::Print(
		"Finished compilation!",
		"LANGUAGE_C_CPP",
		LogType::LOG_SUCCESS);
}
void Compile_Static(const GlobalData& globalData)
{
	Log::Print("\n==========================================================================================\n");

	Log::Print(
		"Output:\n@@@@@ reached compile static\n",
		"LANGUAGE_C_CPP",
		LogType::LOG_INFO);

	Log::Print(
		"Finished generation!",
		"LANGUAGE_C_CPP",
		LogType::LOG_SUCCESS);
}