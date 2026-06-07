#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <system_error>

#include "../include/json_v2.hpp"

namespace fs = std::filesystem;

static std::string read_all(const fs::path& p){
  std::ifstream ifs(p, std::ios::binary);
  if(!ifs) throw std::runtime_error("failed to open: " + p.string());
  std::ostringstream oss;
  oss << ifs.rdbuf();
  return oss.str();
}

static void write_all(const fs::path& p, const std::string& s){
  std::error_code ec;
  fs::create_directories(p.parent_path(), ec);
  std::ofstream ofs(p, std::ios::binary);
  if(!ofs) throw std::runtime_error("failed to write: " + p.string());
  ofs.write(s.data(), static_cast<std::streamsize>(s.size()));
  ofs.put('\n');
}

static std::string rstrip_newlines(std::string s){
  while(!s.empty() && (s.back()=='\n' || s.back()=='\r')) s.pop_back();
  return s;
}

int main(int argc, char** argv){
  bool update_expected = false;
  for(int i=1;i<argc;++i){
    std::string arg(argv[i]);
    if(arg == "--update" || arg == "-u") update_expected = true;
  }

  const fs::path cases_dir = fs::path("test") / "cases";
  const fs::path expected_dir = fs::path("test") / "expected";
  const fs::path out_dir = fs::path("test") / "out";

  if(!fs::exists(cases_dir)){
    std::cerr << "cases dir not found: " << cases_dir << "\n";
    return EXIT_FAILURE;
  }

  std::vector<fs::path> cases;
  for(const auto& de : fs::directory_iterator(cases_dir)){
    if(!de.is_regular_file()) continue;
    if(de.path().extension() == ".json") cases.push_back(de.path());
  }
  std::sort(cases.begin(), cases.end());

  size_t num_total = 0, num_failed = 0, num_updated = 0;

  for(const auto& case_path : cases){
    ++num_total;
    const auto name = case_path.stem().string();
    const fs::path out_path = out_dir / (name + ".out");
    const fs::path exp_path = expected_dir / (name + ".out");

    try{
      const std::string in = read_all(case_path);
      auto v_opt = json::value::load(in);
      if(!v_opt) throw v_opt.error();
      const json::value v = std::move(*v_opt);
      const std::string actual = json::to_string(v);

      write_all(out_path, actual);

      if(fs::exists(exp_path)){
        const std::string expected = read_all(exp_path);
        if(rstrip_newlines(expected) != rstrip_newlines(actual)){
          if(update_expected){
            write_all(exp_path, actual);
            ++num_updated;
            std::cout << "UPDATED: " << name << "\n";
          }else{
            ++num_failed;
            std::cerr << "FAIL: " << name << "\n";
          }
        }else{
          std::cout << "OK: " << name << "\n";
        }
      }else{
        if(update_expected){
          write_all(exp_path, actual);
          ++num_updated;
          std::cout << "NEW: " << name << " (expected written)\n";
        }else{
          ++num_failed;
          std::cerr << "MISSING EXPECTED: " << name << "\n";
        }
      }
    }catch(const std::exception& e){
      ++num_failed;
      std::cerr << "ERROR: " << name << ": " << e.what() << "\n";
    }
  }

  std::cout << "Total: " << num_total << ", Failed: " << num_failed << ", Updated: " << num_updated << "\n";
  return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


