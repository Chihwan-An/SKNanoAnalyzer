// OtJsonLutBank.h
#pragma once
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>
#include <zlib.h>

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

  // schema_version=1 (legacy hf/bvc) + schema_version=2/3 (value_type aware)
  // For integrated bundles, bundle_key == "" loads "central".
  void load_json(const std::filesystem::path &json_path,
                 const std::string &bundle_key = "") {
    nlohmann::json j = read_json_file(json_path);

    if (j.is_object() &&
        j.value("bundle_type", std::string()) == "integrated_lut_bundle") {
      const nlohmann::json *selected = nullptr;
      if (bundle_key.empty()) {
        if (!j.contains("central")) {
          throw std::runtime_error("Integrated LUT bundle missing 'central'");
        }
        selected = &j.at("central");
      } else {
        if (!j.contains("systematics")) {
          throw std::runtime_error(
              "Integrated LUT bundle missing 'systematics'");
        }
        const auto &systematics = j.at("systematics");
        const auto it = systematics.find(bundle_key);
        if (it == systematics.end()) {
          throw std::runtime_error("Missing LUT key in integrated bundle: " +
                                   bundle_key);
        }
        selected = &(*it);
      }
      parse_lut_object(*selected);
      return;
    }

    if (!bundle_key.empty()) {
      throw std::runtime_error(
          "bundle_key is only valid for integrated_lut_bundle JSON");
    }
    parse_lut_object(j);
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

  void parse_lut_object(const nlohmann::json &j) {
    const int schema = j.value("schema_version", 0);
    if (schema != 1 && schema != 2 && schema != 3) {
      throw std::runtime_error("Unsupported schema_version");
    }

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

    const auto &luts = j.at("luts");
    int max_ptbin = -1;
    for (auto it = luts.begin(); it != luts.end(); ++it) {
      max_ptbin = std::max(max_ptbin, std::stoi(it.key()));
    }
    if (max_ptbin < 0)
      throw std::runtime_error("No luts in JSON");
    bins_.assign(size_t(max_ptbin) + 1,
                 {std::nullopt, std::nullopt, std::nullopt});

    for (auto it = luts.begin(); it != luts.end(); ++it) {
      const int ptbin = std::stoi(it.key());
      const auto &by_flav = it.value();

      for (auto fit = by_flav.begin(); fit != by_flav.end(); ++fit) {
        const int fidx = flavour_index(fit.key());
        if (fidx < 0)
          continue;

        const auto &entry = fit.value();
        Lut2DJson lut;
        lut.grid = grid;

        if (value_type_ == LutValueType::kHfBvc) {
          lut.out0 = read_flat_f32(entry.at("hf"));
          lut.out1 = read_flat_f32(entry.at("bvc"));
          lut.clamp01_out = true;
        } else if (value_type_ == LutValueType::kProblProbc) {
          lut.out0 = read_flat_f32(entry.at("probl"));
          lut.out1 = read_flat_f32(entry.at("probc"));
          lut.clamp01_out = true;
        } else if (value_type_ == LutValueType::kIlrZ) {
          lut.out0 = read_flat_f32(entry.at("z1"));
          lut.out1 = read_flat_f32(entry.at("z2"));
          lut.clamp01_out = false;
        }

        lut.finalize_grid();
        bins_[size_t(ptbin)][size_t(fidx)] = std::move(lut);
      }
    }
  }

  static std::vector<uint8_t> decode_base64(std::string_view in) {
    auto b64_value = [](unsigned char c) -> int {
      if (c >= 'A' && c <= 'Z')
        return c - 'A';
      if (c >= 'a' && c <= 'z')
        return c - 'a' + 26;
      if (c >= '0' && c <= '9')
        return c - '0' + 52;
      if (c == '+')
        return 62;
      if (c == '/')
        return 63;
      return -1;
    };

    std::vector<uint8_t> out;
    out.reserve((in.size() * 3) / 4);

    int val = 0;
    int valb = -8;
    for (unsigned char c : in) {
      if (std::isspace(c))
        continue;
      if (c == '=')
        break;
      const int d = b64_value(c);
      if (d < 0) {
        throw std::runtime_error("Invalid base64 character in LUT payload");
      }
      val = (val << 6) | d;
      valb += 6;
      if (valb >= 0) {
        out.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
        valb -= 8;
      }
    }
    return out;
  }

  static float half_to_float(uint16_t h) {
    const uint32_t sign = (uint32_t(h & 0x8000u)) << 16;
    const uint16_t exp = h & 0x7C00u;
    const uint16_t mant = h & 0x03FFu;

    uint32_t fexp = 0;
    uint32_t fmant = 0;

    if (exp == 0x7C00u) {
      fexp = 0xFFu << 23;
      fmant = uint32_t(mant) << 13;
    } else if (exp != 0) {
      const int e = int(exp >> 10) - 15 + 127;
      fexp = uint32_t(e) << 23;
      fmant = uint32_t(mant) << 13;
    } else if (mant != 0) {
      int e = -14;
      uint16_t m = mant;
      while ((m & 0x0400u) == 0) {
        m <<= 1;
        --e;
      }
      m &= 0x03FFu;
      fexp = uint32_t(e + 127) << 23;
      fmant = uint32_t(m) << 13;
    }

    const uint32_t bits = sign | fexp | fmant;
    float out = 0.f;
    std::memcpy(&out, &bits, sizeof(float));
    return out;
  }

  static std::vector<float> decode_f16_base64_blob(const nlohmann::json &obj) {
    const std::string encoding = obj.at("_encoding").get<std::string>();
    if (encoding != "float16_base64") {
      throw std::runtime_error("Unsupported encoded LUT array format: " +
                               encoding);
    }

    const std::string codec = obj.value("codec", "raw");
    const std::string b64 = obj.at("data").get<std::string>();
    const auto shape = obj.at("shape").get<std::vector<int>>();
    if (shape.empty())
      throw std::runtime_error("Encoded LUT array has empty shape");

    size_t nelem = 1;
    for (const int dim : shape) {
      if (dim <= 0)
        throw std::runtime_error("Encoded LUT array has non-positive shape");
      if (nelem > std::numeric_limits<size_t>::max() / size_t(dim)) {
        throw std::runtime_error("Encoded LUT array shape overflow");
      }
      nelem *= size_t(dim);
    }

    const size_t expected_bytes = nelem * sizeof(uint16_t);
    std::vector<uint8_t> payload = decode_base64(b64);

    if (codec == "zlib") {
      std::vector<uint8_t> inflated(expected_bytes);
      uLongf inflated_len = static_cast<uLongf>(inflated.size());
      const int rc = ::uncompress(inflated.data(), &inflated_len, payload.data(),
                                  static_cast<uLong>(payload.size()));
      if (rc != Z_OK) {
        throw std::runtime_error("Failed to zlib-decompress LUT array");
      }
      if (static_cast<size_t>(inflated_len) != expected_bytes) {
        throw std::runtime_error(
            "Decoded LUT array has unexpected uncompressed size");
      }
      payload.swap(inflated);
    } else if (codec != "raw") {
      throw std::runtime_error("Unsupported LUT array codec: " + codec);
    }

    if (payload.size() != expected_bytes) {
      throw std::runtime_error("Decoded LUT array has unexpected byte size");
    }

    std::vector<float> out(nelem, 0.f);
    for (size_t i = 0; i < nelem; ++i) {
      const size_t off = i * 2;
      const uint16_t h =
          uint16_t(payload[off]) | (uint16_t(payload[off + 1]) << 8);
      out[i] = half_to_float(h);
    }
    return out;
  }

  static std::vector<float> read_flat_f32(const nlohmann::json &a) {
    std::vector<float> out;

    if (a.is_array()) {
      if (a.empty())
        return out;
      if (a.at(0).is_array()) { // 2D list -> flatten row-major
        out.reserve(a.size() * a.at(0).size());
        for (const auto &row : a) {
          if (!row.is_array())
            throw std::runtime_error("Inconsistent LUT array rank");
          for (const auto &v : row)
            out.push_back(v.get<float>());
        }
      } else { // 1D list
        out.reserve(a.size());
        for (const auto &v : a)
          out.push_back(v.get<float>());
      }
      return out;
    }

    if (a.is_object())
      return decode_f16_base64_blob(a);

    throw std::runtime_error("Expected LUT array or encoded LUT object");
  }

  static std::string read_gzip_to_string(const std::filesystem::path &p) {
    gzFile gz = ::gzopen(p.string().c_str(), "rb");
    if (!gz)
      throw std::runtime_error("Failed to open gzip JSON: " + p.string());

    std::string out;
    std::array<char, 1 << 15> buf{};
    int nread = 0;
    while ((nread = ::gzread(gz, buf.data(), int(buf.size()))) > 0) {
      out.append(buf.data(), nread);
    }

    if (nread < 0) {
      int errnum = Z_OK;
      const char *msg = ::gzerror(gz, &errnum);
      ::gzclose(gz);
      throw std::runtime_error("Failed to read gzip JSON: " +
                               std::string(msg ? msg : "unknown error"));
    }

    if (::gzclose(gz) != Z_OK) {
      throw std::runtime_error("Failed to close gzip JSON: " + p.string());
    }
    return out;
  }

  static nlohmann::json read_json_file(const std::filesystem::path &p) {
    if (p.extension() == ".gz") {
      const std::string text = read_gzip_to_string(p);
      return nlohmann::json::parse(text);
    }

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
