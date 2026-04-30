#pragma once
///@file

#include <nix/expr/get-drvs.hh>
#include <nix/expr/eval.hh>
#include <nix/store/path.hh>
#include <nix/util/json-impls.hh>
#include <nix/util/types.hh>
#include <nlohmann/json_fwd.hpp>
// we need this include or otherwise we cannot instantiate std::optional
#include <nlohmann/json.hpp> //NOLINT(misc-include-cleaner)
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "eval-args.hh"

namespace nix {
class EvalState;
struct PackageInfo;
} // namespace nix

struct Constituents {
    std::vector<std::string> constituents;
    std::vector<std::string> namedConstituents;
    bool globConstituents = false;

    bool operator==(const Constituents &) const = default;
};

/**
 * The fields of a derivation that are printed in json form
 */
struct Drv {
    static Drv fromPackageInfo(std::string &attrPath, nix::EvalState &state,
                               nix::PackageInfo &packageInfo, MyArgs &args,
                               Constituents constituents = {});

    std::string name;
    std::string storeDir;
    std::string system;
    nix::StorePath drvPath;

    std::map<std::string, std::optional<nix::StorePath>> outputs;

    std::optional<std::map<nix::StorePath, std::set<std::string>>> inputDrvs =
        std::nullopt;

    std::optional<nix::StringSet> requiredSystemFeatures = std::nullopt;

    // TODO: can we lazily allocate these?
    nix::StorePaths neededBuilds;
    std::vector<nix::StorePath> neededSubstitutes;
    std::vector<nix::StorePath> unknownPaths;

    /**
     * @TODO we might not need to store this as it can be computed from
     * the above.
     */
    enum class CacheStatus : uint8_t {
        Local,
        Cached,
        NotBuilt,
        Unknown
    } cacheStatus = CacheStatus::Unknown;

    std::optional<nlohmann::json> meta = std::nullopt;

    /**
     * Aggregate job constituents.
     *
     * Empty when the `--constituents` flag is not passed or when the
     * derivation is not an aggregate.
     */
    Constituents constituents;

    bool operator==(const Drv &) const = default;
};

JSON_IMPL(Constituents)
JSON_IMPL(Drv)
