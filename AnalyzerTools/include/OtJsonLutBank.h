// OtJsonLutBank.h
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

enum class LutValueType : uint8_t {
  kHfBvc = 0,
  kProblProbc = 1,
  kIlrZ = 2,
};

inline LutValueType parse_value_type(std::string_view s) {
  if (s == "hf_bvc")
    return LutValueType::kHfBvc;
  if (s == "probl_probc")
    return LutValueType::kProblProbc;
  if (s == "ilr_z")
    return LutValueType::kIlrZ;
  throw std::runtime_error("Unsupported value_type: " + std::string(s));
}

struct Lut2DJson {
  uint32_t N = 0;

  // grid axis centers (same axis used for x and y)
  std::vector<float> grid; // size N

  // two output fields, both size N*N (row-major iy*N+ix)
  std::vector<float> out0;
  std::vector<float> out1;

  // for sanity (optional)
  bool clamp01_out = true;

  // cached grid properties
  bool uniform = false;
  float gmin = 0.f, gmax = 1.f, inv_step = 0.f;

  void finalize_grid() {
    if (grid.size() < 2)
      throw std::runtime_error("LUT grid too small");
    N = static_cast<uint32_t>(grid.size());
    gmin = grid.front();
    gmax = grid.back();

    // uniform check
    double step0 = double(grid[1]) - double(grid[0]);
    uniform = true;
    for (uint32_t i = 1; i + 1 < N; ++i) {
      double di = double(grid[i + 1]) - double(grid[i]);
      if (std::abs(di - step0) > 1e-6 * std::max(1.0, std::abs(step0))) {
        uniform = false;
        break;
      }
    }
    if (uniform && step0 != 0.0)
      inv_step = float(1.0 / step0);

    if (out0.size() != size_t(N) * size_t(N) ||
        out1.size() != size_t(N) * size_t(N))
      throw std::runtime_error("LUT out0/out1 size mismatch with grid");
  }

  inline float clamp_to_grid(float v) const {
    if (v < gmin)
      return gmin;
    if (v > gmax)
      return gmax;
    return v;
  }

  inline std::pair<uint32_t, float> locate(float v) const {
    v = clamp_to_grid(v);

    if (uniform) {
      float x = (v - gmin) * inv_step;
      int ix = int(std::floor(x));
      if (ix < 0)
        ix = 0;
      if (ix > int(N) - 2)
        ix = int(N) - 2;
      float g0 = grid[size_t(ix)];
      float g1 = grid[size_t(ix + 1)];
      float t = (g1 > g0) ? (v - g0) / (g1 - g0) : 0.f;
      t = std::clamp(t, 0.f, 1.f);
      return {uint32_t(ix), t};
    }

    auto it = std::lower_bound(grid.begin(), grid.end(), v);
    if (it == grid.begin())
      return {0u, 0.f};
    if (it == grid.end())
      return {N - 2u, 1.f};
    uint32_t i1 = uint32_t(std::distance(grid.begin(), it));
    uint32_t i0 = i1 - 1;
    float g0 = grid[i0], g1 = grid[i1];
    float t = (g1 > g0) ? (v - g0) / (g1 - g0) : 0.f;
    t = std::clamp(t, 0.f, 1.f);
    return {i0, t};
  }

  inline float at(const std::vector<float> &arr, uint32_t ix,
                  uint32_t iy) const {
    return arr[size_t(iy) * size_t(N) + size_t(ix)];
  }

  inline float bilinear(const std::vector<float> &arr, float x, float y) const {
    auto [ix, tx] = locate(x);
    auto [iy, ty] = locate(y);
    uint32_t ix1 = ix + 1, iy1 = iy + 1;

    float v00 = at(arr, ix, iy);
    float v10 = at(arr, ix1, iy);
    float v01 = at(arr, ix, iy1);
    float v11 = at(arr, ix1, iy1);

    float vx0 = v00 + tx * (v10 - v00);
    float vx1 = v01 + tx * (v11 - v01);
    return vx0 + ty * (vx1 - vx0);
  }

  // Generic mapping: (x,y) -> (out0,out1)
  inline std::pair<float, float> map_xy(float x, float y) const {
    float o0 = bilinear(out0, x, y);
    float o1 = bilinear(out1, x, y);
    if (clamp01_out) {
      o0 = std::clamp(o0, 0.f, 1.f);
      o1 = std::clamp(o1, 0.f, 1.f);
    }
    return {o0, o1};
  }
};

class OtJsonLutBank {
public:
  explicit OtJsonLutBank(std::vector<float> pt_edges)
      : pt_edges_(std::move(pt_edges)) {
    if (pt_edges_.size() < 2)
      throw std::runtime_error("pt_edges too small");
    if (!std::is_sorted(pt_edges_.begin(), pt_edges_.end()))
      throw std::runtime_error("pt_edges must be sorted increasing");
  }

  // schema_version=1 (legacy hf/bvc) + schema_version=2 (value_type aware)
  void load_json(const std::filesystem::path &json_path) {
    nlohmann::json j = read_json_file(json_path);

    auto read_flat_f32 = [](const nlohmann::json& a) -> std::vector<float> {
      std::vector<float> out;
      if (!a.is_array()) throw std::runtime_error("expected array");
      if (a.empty()) return out;

      if (a.at(0).is_array()) { // 2D
        out.reserve(a.size() * a.at(0).size());
        for (const auto& row : a) {
          for (const auto& v : row) out.push_back(v.get<float>());
        }
      } else { // 1D
        out.reserve(a.size());
        for (const auto& v : a) out.push_back(v.get<float>());
      }
      return out;
    };

    const int schema = j.value("schema_version", 0);
    if (schema != 1 && schema != 2) {
      throw std::runtime_error("Unsupported schema_version");
    }

    // value_type
    if (schema == 1) {
      value_type_ = LutValueType::kHfBvc;
    } else {
      const std::string vt = j.at("value_type").get<std::string>();
      value_type_ = parse_value_type(vt);
    }

    const int grid_size = j.at("grid_size").get<int>();
    const auto grid = j.at("grid").get<std::vector<float>>();
    if ((int)grid.size() != grid_size)
      throw std::runtime_error("grid/grid_size mismatch");

    // Prepare bins container up to max ptbin found
    const auto &luts = j.at("luts");
    int max_ptbin = -1;
    for (auto it = luts.begin(); it != luts.end(); ++it) {
      max_ptbin = std::max(max_ptbin, std::stoi(it.key()));
    }
    if (max_ptbin < 0)
      throw std::runtime_error("No luts in JSON");
    bins_.assign(size_t(max_ptbin) + 1,
                 {std::nullopt, std::nullopt, std::nullopt});

    // Fill
    for (auto it = luts.begin(); it != luts.end(); ++it) {
      int ptbin = std::stoi(it.key());
      const auto &by_flav = it.value();

      for (auto fit = by_flav.begin(); fit != by_flav.end(); ++fit) {
        int fidx = flavour_index(fit.key());
        if (fidx < 0)
          continue;

        const auto &entry = fit.value();

        Lut2DJson lut;
        lut.grid = grid;

        // read mapped fields based on value_type_
        if (value_type_ == LutValueType::kHfBvc) {
          lut.out0 = entry.at("hf").get<std::vector<float>>();
          lut.out1 = entry.at("bvc").get<std::vector<float>>();
          lut.clamp01_out = true;
        } else if (value_type_ == LutValueType::kProblProbc) {
          lut.out0 = entry.at("probl").get<std::vector<float>>();
          lut.out1 = entry.at("probc").get<std::vector<float>>();
          lut.clamp01_out = true;
        } else if (value_type_ == LutValueType::kIlrZ) {
          lut.out0 = read_flat_f32(entry.at("z1"));
          lut.out1 = read_flat_f32(entry.at("z2"));
          lut.clamp01_out = false; // ★ 중요: ILR는 [0,1] clamp 하면 망함
        }

        lut.finalize_grid();
        bins_[size_t(ptbin)][size_t(fidx)] = std::move(lut);
      }
    }
  }

  LutValueType value_type() const { return value_type_; }

  // --- NEW: generic map (x,y)->(out0,out1)
  // partonFlavor: 5->b, 4->c, else->light (including 0)
  std::pair<float, float> map_xy(float pt, int partonFlavor, float x_in,
                                 float y_in) const {
    int ptbin = pt_bin_index(pt);
    int fidx = flavour_index_from_parton(partonFlavor);
    if (ptbin < 0)
      return {x_in, y_in};
    if (size_t(ptbin) >= bins_.size())
      return {x_in, y_in};
    const auto &opt = bins_[size_t(ptbin)][size_t(fidx)];
    if (!opt.has_value())
      return {x_in, y_in};
    return opt->map_xy(x_in, y_in);
  }

  // --- Backward-friendly wrappers
  std::pair<float, float> map_hf_bvc(float pt, int partonFlavor, float hf_in,
                                     float bvc_in) const {
    if (value_type_ != LutValueType::kHfBvc) {
      // wrong LUT type loaded -> be safe: identity
      return {hf_in, bvc_in};
    }
    return map_xy(pt, partonFlavor, hf_in, bvc_in);
  }

  std::pair<float, float> map_probl_probc(float pt, int partonFlavor,
                                          float probl_in,
                                          float probc_in) const {
    if (value_type_ != LutValueType::kProblProbc) {
      return {probl_in, probc_in};
    }
    return map_xy(pt, partonFlavor, probl_in, probc_in);
  }

  std::pair<float, float> map_ilr_z(float pt, int partonFlavor, float z1_in,
                                    float z2_in) const {
    if (value_type_ != LutValueType::kIlrZ)
      return {z1_in, z2_in};
    return map_xy(pt, partonFlavor, z1_in, z2_in);
  }

private:
  std::vector<std::array<std::optional<Lut2DJson>, 3>>
      bins_; // [ptbin][b,c,light]
  std::vector<float> pt_edges_;
  LutValueType value_type_ = LutValueType::kHfBvc;

  static nlohmann::json read_json_file(const std::filesystem::path &p) {
    std::ifstream in(p);
    if (!in)
      throw std::runtime_error("Failed to open JSON: " + p.string());
    nlohmann::json j;
    in >> j;
    return j;
  }

  int pt_bin_index(float pt) const {
    if (pt <= pt_edges_.front())
      return 0;
    if (pt >= pt_edges_.back())
      return int(pt_edges_.size()) - 2;
    auto it = std::upper_bound(pt_edges_.begin(), pt_edges_.end(), pt);
    int idx = int(std::distance(pt_edges_.begin(), it)) - 1;
    return std::clamp(idx, 0, int(pt_edges_.size()) - 2);
  }

  // b=0, c=1, light=2
  static int flavour_index(std::string_view f) {
    if (f == "b")
      return 0;
    if (f == "c")
      return 1;
    if (f == "light")
      return 2;
    return -1;
  }

  static int flavour_index_from_parton(int partonFlavor) {
    int apf = std::abs(partonFlavor);
    if (apf == 5)
      return 0;
    if (apf == 4)
      return 1;
    return 2;
  }
};