#pragma once
///@file

#include <nix/util/json-impls.hh>
#include <nlohmann/json_fwd.hpp>
// Required for std::optional<nlohmann::json>
#include <nlohmann/json.hpp> //NOLINT(misc-include-cleaner)
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "drv.hh"

/* A typed representation of the per-job JSON messages exchanged between
   the worker (evaluator) and the collector (parent) process.

   Every message carries the attribute path that was evaluated.  The
   payload is one of three alternatives:

     Job   - a successfully evaluated derivation
     Attrs - child attribute names that should be recursed into
     Error - an evaluation error                                     */
struct Response {
    std::string attr;                  // dot-joined attribute path
    std::vector<std::string> attrPath; // attribute path components

    struct Job {
        Drv drv;
        std::optional<nlohmann::json> extraValue;
        bool operator==(const Job &) const = default;
    };

    struct Attrs {
        std::vector<std::string> attrs;
        bool operator==(const Attrs &) const = default;
    };

    struct Error {
        std::string error;
        bool operator==(const Error &) const = default;
    };

    using Payload = std::variant<Job, Attrs, Error>;
    Payload payload;

    bool operator==(const Response &) const = default;
};

JSON_IMPL(Response)
